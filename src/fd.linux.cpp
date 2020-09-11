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
#include <sys/types.h>
#include <sys/syscall.h>
#include <linux/limits.h>
#include <asm-generic/errno.h>
#include <asm-generic/fcntl.h>

namespace
{
  int open(const char *path, int flags, int mode)
  {
    long n = SYS_open;

    int ret;
    asm volatile ("syscall" : "=a"(ret) : "0"(n), "D"(path), "S"(flags), "d"(mode) : "rcx", "r11", "memory");
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
    asm volatile ("syscall" : "=a"(ret) : "0"(n), "D"(fd), "S"(iov), "d"(iovcnt), "r"(r10) : "rcx", "r11", "memory");
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
    asm volatile ("syscall" : "=a"(ret) : "0"(n), "D"(fd), "S"(iov), "d"(iovcnt), "r"(r10) : "rcx", "r11", "memory");
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
extern "C" uint32_t fd_open(uintptr_t &fd, string path, uint32_t oflags, uint64_t rights, uint32_t fdflags)
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

  int result = open(pathstr, flags, 0666);

  if (result < 0)
    return -result;

  fd = result;

  return 0;
}

//|///////////////////// fd_readv ///////////////////////////////////////////
extern "C" fd_result fd_readv(uintptr_t fd, iovec *iovs, uint64_t n)
{
  fd_result result = {};

  auto cnt = readv(fd, iovs, n);

  if (cnt > 0)
    result.length = cnt;

  if (cnt < 0)
    result.erno = -cnt;

  return result;
}

//|///////////////////// fd_preadv //////////////////////////////////////////
extern "C" fd_result fd_preadv(uintptr_t fd, iovec *iovs, uint64_t n, uint64_t offset)
{
  fd_result result = {};

  auto cnt = preadv(fd, iovs, n, offset);

  if (cnt > 0)
    result.length = cnt;

  if (cnt < 0)
    result.erno = -cnt;

  return result;
}

//|///////////////////// fd_writev //////////////////////////////////////////
extern "C" fd_result fd_writev(uintptr_t fd, ciovec const *iovs, uint64_t n)
{
  fd_result result = {};

  while (n != 0)
  {
    auto cnt = writev(fd, iovs, n);

    if (cnt >= 0)
    {
      result.length += cnt;

      for(; n != 0 && iovs[0].len <= uint64_t(cnt); ++iovs, --n)
      {
        cnt -= iovs[0].len;
      }

      if (cnt > 0)
      {
        ciovec rest = { iovs[0].data + iovs[0].len - cnt, iovs[0].len - cnt };

        while (rest.len > 0 && (cnt = writev(fd, &rest, 1)) >= 0)
        {
          rest.data += cnt;
          rest.len -= cnt;
        }

        result.length += iovs[0].len - rest.len;

        ++iovs;
        --n;
      }
    }

    if (cnt < 0)
    {
      result.erno = -cnt;
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
    auto cnt = pwritev(fd, iovs, n, offset);

    if (cnt >= 0)
    {
      offset += cnt;
      result.length += cnt;

      for(; n != 0 && iovs[0].len <= uint64_t(cnt); ++iovs, --n)
      {
        cnt -= iovs[0].len;
      }

      if (cnt > 0)
      {
        ciovec rest = { iovs[0].data + iovs[0].len - cnt, iovs[0].len - cnt };

        while (rest.len > 0 && (cnt = pwritev(fd, &rest, 1, offset)) >= 0)
        {
          rest.data += cnt;
          rest.len -= cnt;
          offset += cnt;
        }

        result.length += iovs[0].len - rest.len;

        ++iovs;
        --n;
      }
    }

    if (cnt < 0)
    {
      result.erno = -cnt;
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
