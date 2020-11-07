//
// mem.cpp
//
// Copyright (C) Peter Niekamp
//
// This code remains the property of the copyright holder.
// The code contained herein is licensed for use without limitation
//

#include "mem.h"
#include <cstddef>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/user.h>

#define PROT_READ	0x1		/* Page can be read.  */
#define PROT_WRITE	0x2		/* Page can be written.  */
#define PROT_EXEC	0x4		/* Page can be executed.  */
#define PROT_NONE	0x0		/* Page can not be accessed.  */

#define MAP_SHARED    0x01	/* Share changes.  */
#define MAP_PRIVATE	  0x02	/* Changes are private.  */
#define MAP_ANONYMOUS 0x20	/* Don't use a file.  */

namespace
{
  void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
  {
    long n = SYS_mmap;

    void *ret;
    register long r10 asm("r10") = flags;
    register long r8 asm("r8") = fd;
    register long r9 asm("r9") = offset;
    asm volatile ("syscall" : "=a"(ret) : "0"(n), "D"(addr), "S"(length), "d"(prot), "r"(r10), "r"(r8), "r"(r9) : "rcx", "r11", "memory");
    return ret;
  }

  int munmap(void *addr, size_t length)
  {
    long n = SYS_munmap;

    int ret;
    asm volatile ("syscall" : "=a"(ret) : "0"(n), "D"(addr), "S"(length) : "rcx", "r11", "memory");
    return ret;
  }
}

//|///////////////////// mem_alloc //////////////////////////////////////////
extern "C" mem_result mem_alloc(uint64_t size)
{
  mem_result result = {};

  size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

  result.size = size;
  result.addr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if ((long)result.addr < 0)
  {
    result.erno = -(long)result.addr;
    result.addr = nullptr;
    result.size = 0;
  }

  return result;
}

//|///////////////////// mem_free ///////////////////////////////////////////
extern "C" void mem_free(void const *addr)
{
  munmap(const_cast<void*>(addr), PTRDIFF_MAX);
}
