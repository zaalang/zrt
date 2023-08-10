//
// fd.cpp
//
// Copyright (C) Peter Niekamp
//
// This code remains the property of the copyright holder.
// The code contained herein is licensed for use without limitation
//

#include "fd.h"
#include <cstddef>
#include <limits.h>
#include <sys/types.h>
#include <sys/syscall.h>

#define	ENAMETOOLONG 36

#define O_RDONLY 00000000
#define O_WRONLY 00000001
#define O_RDWR 00000002

#define O_CREAT 00000100
#define O_EXCL 00000200
#define O_TRUNC 00001000
#define O_APPEND 00002000
#define O_NONBLOCK 00004000
#define O_DSYNC 00010000
#define O_SYNC (04000000|O_DSYNC)

#define S_IFMT 0170000
#define S_IFDIR 0040000
#define S_IFCHR 0020000
#define S_IFBLK 0060000
#define S_IFREG 0100000
#define S_IFIFO 0010000
#define S_IFLNK 0120000
#define S_IFSOCK 0140000

struct stat
{
  dev_t st_dev;
  ino_t st_ino;
  nlink_t st_nlink;

  mode_t st_mode;
  uid_t st_uid;
  gid_t st_gid;
  unsigned int __pad0;
  dev_t st_rdev;
  off_t st_size;
  blksize_t st_blksize;
  blkcnt_t st_blocks;

  struct timespec st_atim;
  struct timespec st_mtim;
  struct timespec st_ctim;
  long __unused[3];
};

namespace
{
  int open(const char *path, int flags, int mode)
  {
    long n = SYS_open;

    int ret;
    asm volatile ("syscall" : "=a"(ret) : "0"(n), "D"(path), "S"(flags), "d"(mode) : "rcx", "r11", "memory");
    return ret;
  }

  int fstat(int fd, struct stat *fs)
  {
    long n = SYS_fstat;

    long ret;
    asm volatile ("syscall" : "=a"(ret) : "0"(n), "D"(fd), "S"(fs) : "rcx", "r11", "memory");
    return ret;
  }

  ssize_t readv(int fd, iovec *iov, int iovcnt)
  {
    long n = SYS_readv;

    long ret;
    asm volatile ("syscall" : "=a"(ret) : "0"(n), "D"(fd), "S"(iov), "d"(iovcnt) : "rcx", "r11", "memory");
    return ret;
  }

  ssize_t preadv(int fd, iovec *iov, int iovcnt, off_t offset)
  {
    long n = SYS_preadv;

    long ret;
    register long r10 asm("r10") = offset;
    register long r8 asm("r8") = 0;
    asm volatile ("syscall" : "=a"(ret) : "0"(n), "D"(fd), "S"(iov), "d"(iovcnt), "r"(r10), "r"(r8) : "rcx", "r11", "memory");
    return ret;
  }

  ssize_t writev(int fd, ciovec const *iov, int iovcnt)
  {
    long n = SYS_writev;

    long ret;
    asm volatile ("syscall" : "=a"(ret) : "0"(n), "D"(fd), "S"(iov), "d"(iovcnt) : "rcx", "r11", "memory");
    return ret;
  }

  ssize_t pwritev(int fd, ciovec const *iov, int iovcnt, off_t offset)
  {
    long n = SYS_pwritev;

    long ret;
    register long r10 asm("r10") = offset;
    register long r8 asm("r8") = 0;
    asm volatile ("syscall" : "=a"(ret) : "0"(n), "D"(fd), "S"(iov), "d"(iovcnt), "r"(r10), "r"(r8) : "rcx", "r11", "memory");
    return ret;
  }

  int close(int fd)
  {
    long n = SYS_close;

    int ret;
    asm volatile ("syscall" : "=a"(ret) : "0"(n), "D"(fd) : "rcx", "r11", "memory");
    return ret;
  }
}

extern "C" void *memcpy(void * __restrict__ dst, const void * __restrict__ src, size_t n);

//|///////////////////// fd_open ////////////////////////////////////////////
extern "C" uint32_t fd_open(uintptr_t *fd, string path, uint32_t oflags, uint64_t rights, uint32_t fdflags)
{
  if (path.len >= PATH_MAX)
    return ENAMETOOLONG;

  char pathstr[PATH_MAX];
  memcpy(pathstr, path.data, path.len);
  pathstr[path.len] = 0;

  int flags = 0;

  if (oflags & fd::oflags::create)
    flags |= O_CREAT;

  if (oflags & fd::oflags::exclusive)
    flags |= O_EXCL;

  if (oflags & fd::oflags::trunc)
    flags |= O_TRUNC;

  if (fdflags & fd::fdflags::append)
    flags |= O_APPEND;

  if (fdflags & fd::fdflags::dsync)
    flags |= O_DSYNC;

  if (fdflags & fd::fdflags::nonblock)
    flags |= O_NONBLOCK;

  if (fdflags & fd::fdflags::rsync)
    flags |= O_SYNC;

  if (fdflags & fd::fdflags::sync)
    flags |= O_SYNC;

  if ((rights & fd::rights::read) && !(rights & fd::rights::write))
    flags |= O_RDONLY;

  if (!(rights & fd::rights::read) && (rights & fd::rights::write))
    flags |= O_WRONLY;

  if ((rights & fd::rights::read) && (rights & fd::rights::write))
    flags |= O_RDWR;

  int rc = open(pathstr, flags, 0666);

  if (rc < 0)
    return -rc;

  *fd = rc;

  return 0;
}

//|///////////////////// fd_stat ////////////////////////////////////////////
extern "C" uint32_t fd_stat(uintptr_t fd, filestat *fs)
{
  struct stat buf;

  if (auto rc = fstat(fd, &buf); rc < 0)
    return -rc;

  if ((buf.st_mode & S_IFMT) == S_IFDIR)
    fs->type = filetype::directory;

  if ((buf.st_mode & S_IFMT) == S_IFREG)
    fs->type = filetype::regular_file;

  if ((buf.st_mode & S_IFMT) == S_IFLNK)
    fs->type = filetype::symbolic_link;

  fs->size = buf.st_size;
  fs->atime = (uint64_t)buf.st_atim.tv_sec * 1000000000 + buf.st_atim.tv_nsec;
  fs->ctime = (uint64_t)buf.st_ctim.tv_sec * 1000000000 + buf.st_ctim.tv_nsec;
  fs->mtime = (uint64_t)buf.st_mtim.tv_sec * 1000000000 + buf.st_mtim.tv_nsec;

  return 0;
}

//|///////////////////// fd_readv ///////////////////////////////////////////
extern "C" fd_result fd_readv(uintptr_t fd, iovec *iovs, uint64_t n)
{
  fd_result result = {};

  auto bytes = readv(fd, iovs, n);

  if (bytes > 0)
    result.length = bytes;

  if (bytes < 0)
    result.erno = -bytes;

  return result;
}

//|///////////////////// fd_preadv //////////////////////////////////////////
extern "C" fd_result fd_preadv(uintptr_t fd, iovec *iovs, uint64_t n, uint64_t offset)
{
  fd_result result = {};

  auto bytes = preadv(fd, iovs, n, offset);

  if (bytes > 0)
    result.length = bytes;

  if (bytes < 0)
    result.erno = -bytes;

  return result;
}

//|///////////////////// fd_writev //////////////////////////////////////////
extern "C" fd_result fd_writev(uintptr_t fd, ciovec const *iovs, uint64_t n)
{
  fd_result result = {};

  while (n != 0)
  {
    auto bytes = writev(fd, iovs, n);

    if (bytes >= 0)
    {
      result.length += bytes;

      for(; n != 0 && iovs[0].len <= uint64_t(bytes); ++iovs, --n)
      {
        bytes -= iovs[0].len;
      }

      if (bytes > 0)
      {
        ciovec rest = { iovs[0].data + iovs[0].len - bytes, iovs[0].len - bytes };

        while (rest.len > 0 && (bytes = writev(fd, &rest, 1)) >= 0)
        {
          rest.data += bytes;
          rest.len -= bytes;
        }

        result.length += iovs[0].len - rest.len;

        ++iovs;
        --n;
      }
    }

    if (bytes < 0)
    {
      result.erno = -bytes;
      break;
    }
  }

  return result;
}

//|///////////////////// fd_pwritev /////////////////////////////////////////
extern "C" fd_result fd_pwritev(uintptr_t fd, ciovec const *iovs, uint64_t n, uint64_t offset)
{
  fd_result result = {};

  while (n != 0)
  {
    auto bytes = pwritev(fd, iovs, n, offset);

    if (bytes >= 0)
    {
      offset += bytes;
      result.length += bytes;

      for(; n != 0 && iovs[0].len <= uint64_t(bytes); ++iovs, --n)
      {
        bytes -= iovs[0].len;
      }

      if (bytes > 0)
      {
        ciovec rest = { iovs[0].data + iovs[0].len - bytes, iovs[0].len - bytes };

        while (rest.len > 0 && (bytes = pwritev(fd, &rest, 1, offset)) >= 0)
        {
          rest.data += bytes;
          rest.len -= bytes;
          offset += bytes;
        }

        result.length += iovs[0].len - rest.len;

        ++iovs;
        --n;
      }
    }

    if (bytes < 0)
    {
      result.erno = -bytes;
      break;
    }
  }

  return result;
}

//|///////////////////// fd_close ///////////////////////////////////////////
extern "C" uint32_t fd_close(uintptr_t fd)
{
  return close(fd);
}
