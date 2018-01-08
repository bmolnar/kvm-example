#ifndef _LIBKVM_H
#define _LIBKVM_H

#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/kvm.h>

#include <memory>
#include <vector>

#include "file.h"

class Kvm;
class Vm;
class Vcpu;


class Kvm {
public:
  Kvm();
  ~Kvm();
  int GetApiVersion();
  int GetVcpuMmapSize();
  Vm* CreateVm();
protected:
  std::shared_ptr<File> fp_;
  std::vector<Vm*> vms_;
};

class Vm {
public:
  Vm(Kvm* kvm, std::shared_ptr<File> fp);
  ~Vm();
  Kvm* GetKvm();
  void SetTssAddr(unsigned long addr);
  Vcpu* CreateVcpu(unsigned long slot);
  void* GetPhysMem(unsigned long start, unsigned long len);
private:
  Kvm* kvm_;
  std::shared_ptr<File> fp_;
  std::vector<Vcpu*> vcpus_;
};


class Vcpu {
public:
  Vcpu(Vm* vm, unsigned long slot, std::shared_ptr<File> fp);
  ~Vcpu();
  Vm* GetVm();
  void GetRegs(struct kvm_regs* regs);
  void SetRegs(const struct kvm_regs& regs);
  void GetSregs(struct kvm_sregs* sregs);
  void SetSregs(const struct kvm_sregs& sregs);
  void Run();
private:
  Vm* vm_;
  unsigned long slot_;
  std::shared_ptr<File> fp_;
  struct kvm_run* kvm_run_;
};

#endif // _LIBKVM_H
