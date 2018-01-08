#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string>

class File {
 private:
  int fd_;
  bool owner_;
 public:
  File();
  File(int fd, bool owner = false);
  File(std::string path, int flags, mode_t mode = (S_IRUSR | S_IWUSR));
  ~File();
  int Fd() const;
  int Ioctl(int req, unsigned long arg);
  int Ioctl(int req, void* arg);
  ssize_t Write(const void* buf, size_t count);
  ssize_t Read(void* buf, size_t count);
};

#endif // FILE_H
