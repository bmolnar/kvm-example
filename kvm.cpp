#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/kvm.h>

#include "log.h"
#include "kvm.h"


//
// Kvm
//
Kvm::Kvm() {
  fp_ = std::make_shared<File>("/dev/kvm", O_RDWR);
}
Kvm::~Kvm() {
  LOG("Kvm::~Kvm()");
}
int Kvm::GetApiVersion() {
  return fp_->Ioctl(KVM_GET_API_VERSION, (unsigned long) 0);
}
int Kvm::GetVcpuMmapSize() {
  return fp_->Ioctl(KVM_GET_VCPU_MMAP_SIZE, (unsigned long) 0);
}
Vm* Kvm::CreateVm() {
  int fd = fp_->Ioctl(KVM_CREATE_VM, (unsigned long) 0);
  //std::shared_ptr<File> fp = std::make_shared<File>(fd);
  return new Vm(this, std::make_shared<File>(fd));
}



//
// Vm
//
Vm::Vm(Kvm* kvm, std::shared_ptr<File> fp)
  : kvm_(kvm), fp_(fp) {
}
Vm::~Vm() {
  //for (Vcpu* vcpu : vcpus_) {  
  //}
}
Kvm* Vm::GetKvm() {
  return kvm_;
}
void Vm::SetTssAddr(unsigned long addr) {
  fp_->Ioctl(KVM_SET_TSS_ADDR, addr);
}
Vcpu* Vm::CreateVcpu(unsigned long slot) {
  int vcpu_fd = fp_->Ioctl(KVM_CREATE_VCPU, slot);
  //std::shared_ptr<File> fp = std::make_shared<File>(vcpu_fd);
  Vcpu* result = new Vcpu(this, slot, std::make_shared<File>(vcpu_fd));
  vcpus_.push_back(result);
  return result;
}
void* Vm::GetPhysMem(unsigned long start, unsigned long len) {
  size_t page_size = sysconf(_SC_PAGESIZE);

  void *user_addr;
  if (posix_memalign(&user_addr, page_size, len) != 0) {
    return NULL;
  }

  struct kvm_userspace_memory_region region;
  region.slot = 0;
  region.flags = 0;
  region.guest_phys_addr = start;
  region.memory_size = len;
  region.userspace_addr = (__u64) user_addr;

  LOG("region: .slot=%lu, .flags=%08lx, .guest_phys_addr=0x%016llx, .memory_size=%llu, .userspace_addr=0x%016llx\n",
      (unsigned long) region.slot,
      (unsigned long) region.flags,
      (unsigned long long) region.guest_phys_addr,
      (unsigned long long) region.memory_size,
      (unsigned long long) region.userspace_addr);

  if (fp_->Ioctl(KVM_SET_USER_MEMORY_REGION, (void *) &region) < 0) {
    free(user_addr);
    return NULL;
  }

  return user_addr;
}


//
// Vcpu
//
Vcpu::Vcpu(Vm* vm, unsigned long slot, std::shared_ptr<File> fp)
  : vm_(vm), slot_(slot), fp_(fp) {
  int mmap_size = vm->GetKvm()->GetVcpuMmapSize();

  kvm_run_ = (struct kvm_run *) mmap(NULL, mmap_size, PROT_READ|PROT_WRITE, MAP_SHARED, fp_->Fd(), 0);
  if (kvm_run_ == NULL) {
    LOG("mmap() failed\n");
  }
}
Vcpu::~Vcpu() {
}
Vm* Vcpu::GetVm() {
  return vm_;
}
void Vcpu::GetRegs(struct kvm_regs* regs) {
  fp_->Ioctl(KVM_GET_REGS, (void *) regs);
}
void Vcpu::SetRegs(const struct kvm_regs& regs) {
  fp_->Ioctl(KVM_SET_REGS, (void *) &regs);
}
void Vcpu::GetSregs(struct kvm_sregs* sregs) {
  fp_->Ioctl(KVM_GET_SREGS, (void *) sregs);
}
void Vcpu::SetSregs(const struct kvm_sregs& sregs) {
  fp_->Ioctl(KVM_SET_SREGS, (void *) &sregs);
}
void Vcpu::Run() {
  int rv;
  unsigned int value;

  for (;;) {
    rv = fp_->Ioctl(KVM_RUN, (unsigned long) 0);
    if (rv < 0 && errno != EINTR && errno != EAGAIN) {
      return;
    }

    //LOG("kvm_exit: %d\n", kvm_run_->exit_reason);

    switch (kvm_run_->exit_reason) {
      case KVM_EXIT_UNKNOWN:
        break;
      case KVM_EXIT_EXCEPTION:
        break;
      case KVM_EXIT_IO:
        LOG("KVM_EXIT_IO: .direction=%u, .size=%u, .port=%u, .count=%u, .data_offset=%llu\n",
            kvm_run_->io.direction, kvm_run_->io.size, kvm_run_->io.port, kvm_run_->io.count,
            (unsigned long long) kvm_run_->io.data_offset);
        switch (kvm_run_->io.direction) {
          case KVM_EXIT_IO_IN:
            fprintf(stdout, "> Send value: ");
            scanf("%u", &value);
            *(((uint8_t *) kvm_run_) + kvm_run_->io.data_offset) = value;
            break;
          case KVM_EXIT_IO_OUT:
            value = *(((uint8_t *) kvm_run_) + kvm_run_->io.data_offset);
            LOG("> Got value: %u\n", value);
            break;
        };
        break;
      case KVM_EXIT_HYPERCALL:
        exit(0);
      case KVM_EXIT_DEBUG:
        LOG("Stepping...\n");
        break;
      case KVM_EXIT_HLT:
        return;
      case KVM_EXIT_MMIO:
        break;
      case KVM_EXIT_IRQ_WINDOW_OPEN:
        break;
      case KVM_EXIT_SHUTDOWN:
        break;
      case KVM_EXIT_FAIL_ENTRY:
        break;
      case KVM_EXIT_INTR:
        break;
      case KVM_EXIT_SET_TPR:
        break;
      case KVM_EXIT_TPR_ACCESS:
        break;
      case KVM_EXIT_S390_SIEIC:
        break;
      case KVM_EXIT_S390_RESET:
        break;
      case KVM_EXIT_DCR:
        break;
      case KVM_EXIT_NMI:
        break;
      case KVM_EXIT_INTERNAL_ERROR:
        break;
    }
  }
}



void load_program(unsigned char *ram)
{
  int rv;
  int i;

#define BINEXE
#ifdef BINEXE	
  LOG("Loading guest.bin file\n");
  int fd = open("guest.bin", O_RDONLY);
  if (fd < 0) {
    LOG("open() failed\n");
    return;
  }
#else
  LOG("Loading guest.elf32 file\n");
  int fd = open("guest.elf32", O_RDONLY);
  if (fd < 0) {
    LOG("open() failed\n");
    return;
  }
  // Skipping ELF header
  LOG("Skipped %d bytes\nDONE\n", rv);
  rv = read(fd, ram, 0x1000);
  if (rv < 0) {
    LOG("read() failed\n");
    return;
  }
#endif

  rv = read(fd, ram, 4096);
  if (rv < 0) {
    return;
  }
  LOG("Read %d bytes\nDONE\n", rv);

  fprintf(stdout, "RAM dump: [[\n");
  for (i = 0; i < rv; i++) {
    fprintf(stdout, "%x ", (unsigned char) ram[i]);
  }
  fprintf(stdout, "\n]] OVER\n");
}


int do_test()
{
  Kvm kvm;

  Vm* vm = kvm.CreateVm();
  vm->SetTssAddr(0xfffbd000);

  Vcpu* vcpu = vm->CreateVcpu(0);

  unsigned long ram_size = 16L * 1024 * 1024;
  void* ram_start = vm->GetPhysMem(0, ram_size);
  if (ram_start == NULL) {
    LOG("Failed to allocate ram\n");
    return -1;
  }

  load_program((unsigned char *) ram_start);

  struct kvm_sregs sregs;
  vcpu->GetSregs(&sregs);

  sregs.cs.selector = 0;
  sregs.ss.selector = 0;
  sregs.ds.selector = 0;
  sregs.es.selector = 0;
  sregs.fs.selector = 0;
  sregs.gs.selector = 0;

  sregs.cs.base = 16 * 0;
  sregs.ss.base = 16 * 0;
  sregs.ds.base = 16 * 0;
  sregs.es.base = 16 * 0;
  sregs.fs.base = 16 * 0;
  sregs.gs.base = 16 * 0;

  sregs.cr0 |= 0x1;

  // Create a GDT somewhere in RAM and make segments cover whole RAM
#define GDTLOC (0xF00Baa)
  *(unsigned long*)(((unsigned char *) ram_start) + GDTLOC) = 0x00000000000000000; // null descriptor
  *(unsigned long*)(((unsigned char *) ram_start) + GDTLOC + 8) = 0x00cf9a000000ffff; // code
  *(unsigned long*)(((unsigned char *) ram_start) + GDTLOC + 16) = 0x00cf92000000ffff; // data

  sregs.gdt.base = GDTLOC;
  sregs.gdt.limit = 32;
  sregs.idt.base = 0;
  sregs.idt.limit = 0;

  sregs.cs.selector =  8;
  sregs.ss.selector = 16;
  sregs.ds.selector = 16;
  sregs.es.selector = 16;

  sregs.cs.g = 1;
  sregs.ss.g = 1;
  sregs.ds.g = 1;
  sregs.es.g = 1;

  sregs.cs.db = 1;
  sregs.ss.db = 1;
  sregs.ds.db = 1;
  sregs.es.db = 1;

  sregs.cs.limit = 0xffffffff;
  sregs.ss.limit = 0xffffffff;
  sregs.ds.limit = 0xffffffff;
  sregs.es.limit = 0xffffffff;
  
  vcpu->SetSregs(sregs);


  struct kvm_regs regs;
  memset(&regs, 0, sizeof(regs));
  // Instead of starting in 16-bit real mode, start in protected mode!
  regs.rflags = 0x0000000000000002ULL;
  regs.rip = 0x0000;
  regs.rsp = ram_size;
  regs.rbp = ram_size - 0x500;
  vcpu->SetRegs(regs);

  // Run cpu
  vcpu->Run();

  return 0;
}

int main(int argc, char *argv[])
{
  do_test();
  return 0;
}
