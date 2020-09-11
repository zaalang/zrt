//
// fd.h
//
// Copyright (C) Peter Niekamp
//
// This code remains the property of the copyright holder.
// The code contained herein is licensed for use without limitation
//

#pragma once

#include <cstdint>

constexpr uintptr_t STDIN = 0;
constexpr uintptr_t STDOUT = 1;
constexpr uintptr_t STDERR = 2;

struct iovec
{
  uint8_t *data;
  uint64_t len;
};

struct ciovec
{
  uint8_t const *data;
  uint64_t len;
};

struct string
{
  uint8_t const *data;
  uint64_t len;
};

namespace fd
{
  enum oflags
  {
    open = 0x0,
    create = 0x01,
    exclusive = 0x02,
    trunc = 0x04,
  };

  enum rights
  {
    read = 0x01,
    write = 0x02,
  };

  enum fdflags
  {
    append = 0x01,
    dsync = 0x02,
    nonblock = 0x04,
    rsync = 0x08,
    sync = 0x10,
  };
}

struct fd_result
{
  uint32_t erno;
  uint64_t length;
};

extern "C" uint32_t fd_open(uintptr_t &fd, string path, uint32_t oflags, uint64_t rights, uint32_t fdflags);

extern "C" fd_result fd_readv(uintptr_t fd, iovec *iovs, uint64_t n);
extern "C" fd_result fd_preadv(uintptr_t fd, iovec *iovs, uint64_t n, uint64_t offset);

extern "C" fd_result fd_writev(uintptr_t fd, ciovec const *iovs, uint64_t n);
extern "C" fd_result fd_pwritev(uintptr_t fd, ciovec const *iovs, uint64_t n, uint64_t offset);

extern "C" uint32_t fd_close(uintptr_t fd);
