#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string>
#include <stdexcept>

#include "log.h"
#include "file.h"

File::File()
  : fd_(-1), owner_(false) {
  LOG("File::File(fd=%d, owner=%d)\n", fd_, owner_);
}
File::File(int fd, bool owner)
  : fd_(fd), owner_(owner) {
  LOG("File::File(fd=%d, owner=%d)\n", fd_, owner_);
}
File::File(std::string path, int flags, mode_t mode)
  : fd_(-1), owner_(true) {
  LOG("File::File(path=\"%s\", flags=%04x, mode=%04x)\n", path.c_str(), flags, mode);

  fd_ = open(path.c_str(), flags, mode);
  if (fd_ < 0) {
    fprintf(stderr, "open(\"%s\", %04x) failed: %s\n", path.c_str(), (unsigned int) flags, strerror(errno));
    throw std::runtime_error("open() failed:");
  }
}
File::~File() {
  LOG("File::~File(fd=%d)\n", fd_);

  if (owner_ && fd_ >= 0) {
    close(fd_);
    fprintf(stderr, "close(fd=%d): SUCCESS\n", fd_);
    fd_ = -1;
  }
}
int File::Fd() const {
  return fd_;
}
int File::Ioctl(int req, unsigned long arg) {
  int rv = ioctl(fd_, req, arg);
  if (rv < 0) {
    fprintf(stderr, "ioctl(fd=%d, req=%d) failed: %s\n", fd_, req, strerror(errno));
  }
  return rv;
}
int File::Ioctl(int req, void* arg) {
  int rv = ioctl(fd_, req, arg);
  if (rv < 0) {
    fprintf(stderr, "ioctl(fd=%d, req=%d) failed: %s\n", fd_, req, strerror(errno));
  }
  return rv;
}
ssize_t File::Write(const void* buf, size_t count) {
  ssize_t rv = write(fd_, buf, count);
  if (rv < 0) {
    fprintf(stderr, "write() failed: %s\n", strerror(errno));
  }
  return rv;
}
ssize_t File::Read(void* buf, size_t count) {
  ssize_t rv = read(fd_, buf, count);
  if (rv < 0) {
    fprintf(stderr, "read() failed: %s\n", strerror(errno));
  }
  return rv;
}
