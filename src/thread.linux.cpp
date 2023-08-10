//
// thread.cpp
//
// Copyright (C) Peter Niekamp
//
// This code remains the property of the copyright holder.
// The code contained herein is licensed for use without limitation
//

#include "thread.h"
#include <sys/types.h>
#include <sys/syscall.h>

#define	EINVAL 22

#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define PROT_EXEC 0x4
#define PROT_NONE 0x0

#define MAP_SHARED 0x01
#define MAP_PRIVATE 0x02
#define MAP_ANONYMOUS 0x20

#define CLONE_VM 0x00000100
#define CLONE_FS 0x00000200
#define CLONE_FILES 0x00000400
#define CLONE_SIGHAND 0x00000800
#define CLONE_THREAD 0x00010000
#define CLONE_SYSVSEM 0x00040000
#define CLONE_SETTLS 0x00080000
#define CLONE_PARENT_SETTID 0x00100000
#define CLONE_CHILD_CLEARTID 0x00200000

#define FUTEX_WAIT 0

#define THREAD_DEAD  0x01
#define THREAD_DETACHED	0x02

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

  void munmap_and_exit(void *addr, size_t length)
  {
    long n = SYS_munmap;

    asm volatile ("syscall" :: "a"(n), "D"(addr), "S"(length) : "rcx", "r11", "memory");
    asm volatile ("syscall" :: "a"(SYS_exit), "D"(0) : "rcx", "r11", "memory");
  }

  int clone(void *stack, unsigned long flags, int *parent_tid, int *child_tid, void *tls)
  {
    long n = SYS_clone;

    int ret;
    register int *r10 asm("r10") = child_tid;
    register unsigned long r8 asm("r8") = (unsigned long)tls;
    asm volatile (
          ".intel_syntax noprefix\n"
          "    syscall\n"
          "    test eax, eax\n"
          "    jnz 1f\n"
          "    xor ebp, ebp\n"
          "    mov rdi, [rsp]\n"
          "    and rsp, -16\n"
          "    call __thread_start\n"
          "    hlt\n"
          "1:\n"
          ".att_syntax\n"
      : "=a"(ret) : "0"(n), "D"(flags), "S"(stack), "d"(parent_tid), "r"(r10), "r"(r8) : "rcx", "r11", "memory");
    return ret;
  }

  int futex(int *uaddr, int op, int val, timespec *timeout)
  {
    long n = SYS_futex;

    unsigned long ret;
    register timespec *r10 asm("r10") = timeout;
    asm volatile ("syscall" : "=a"(ret) : "0"(n), "D"(uaddr), "S"(op), "d"(val), "r"(r10) : "rcx", "r11", "memory");
    return ret;
  }

  void exit(int rval)
  {
    for (;;)
    {
      long n = SYS_exit;

      unsigned long ret;
      asm volatile ("syscall" : "=a"(ret) : "0"(n), "D"(rval) : "rcx", "r11", "memory");
    }
  }
}

//|///////////////////// __thread_start /////////////////////////////////////
extern "C" void __thread_start(thread_data *td)
{
  auto rval = td->start_routine(td->start_argument);

  if (__atomic_fetch_or(&td->flags, THREAD_DEAD, __ATOMIC_SEQ_CST) & THREAD_DETACHED)
    munmap_and_exit(td->memory, td->size);

  exit(rval);
}

//|///////////////////// pthread_create /////////////////////////////////////
extern "C" int pthread_create(pthread_t *thread, pthread_attr_t const *attr, int (*start_routine)(void*), void *start_argument)
{
  long size = 8388608;

  void *memory = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if ((long)memory < 0)
    return (long)memory;

  auto self = thread_self();
  auto tls = self->tls;

#if TLS_ABOVE_TP
  auto tlsbase = ((uintptr_t)memory + size - tls->size) & (tls.align - 1);
  auto dtv = (uintptr_t*)(tlsbase - 2*sizeof(void*) - GAP_ABOVE_TP);
  auto td = (thread_data*)(tlsbase - sizeof(thread_data) - 2*sizeof(void*) - GAP_ABOVE_TP);
  auto stack = (void*)((uintptr_t)td & -16);
#else
  auto tlsbase = ((uintptr_t)memory + size - 2*sizeof(void*) - tls->size - sizeof(thread_data)) & -tls->align;
  auto dtv = (uintptr_t*)(tlsbase + tls->size + sizeof(thread_data));
  auto td = (thread_data*)(tlsbase + tls->size);
  auto stack = tlsbase & -16;
#endif

  dtv[0] = 1;
  dtv[1] = tlsbase + DTP_OFFSET;
  __builtin_memcpy((void*)tlsbase, (void*)(tls->base + tls->vaddr), tls->len);

  td->self = td;
  td->canary = 0xdeadbeef;
  td->tls = tls;
  td->dtv = dtv;
  td->pid = self->pid;
  td->start_routine = start_routine;
  td->start_argument = start_argument;
  td->memory = memory;
  td->size = size;
  td->flags = 0;

  stack -= sizeof(thread_data*);
  *(void**)stack = td;

  *thread = (pthread_t)td;

#if TLS_ABOVE_TP
  td = (thread_data*)((uintptr_t)td + sizeof(thread_data) + TP_OFFSET);
#endif

  long flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | CLONE_SYSVSEM | CLONE_SETTLS | CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID;

  int ret = clone((void*)stack, flags, &td->tid, &td->tid, td);

  if (ret < 0)
  {
    munmap(memory, size);
  }

  return ret;
}

//|///////////////////// pthread_join ///////////////////////////////////////
extern "C" int pthread_join(pthread_t thread, int *rval)
{
  auto td = (thread_data*)(thread);

  for(;;)
  {
    auto tid = td->tid;

    if (tid == 0)
      break;

    futex(&td->tid, FUTEX_WAIT, tid, NULL);
  }

  munmap(td->memory, td->size);

  return 0;
}

//|///////////////////// pthread_detach /////////////////////////////////////
extern "C" int pthread_detach(pthread_t thread)
{
  auto td = (thread_data*)(thread);

  if (__atomic_fetch_or(&td->flags, THREAD_DETACHED, __ATOMIC_SEQ_CST) & THREAD_DEAD)
    munmap(td->memory, td->size);

  return 0;
}
