//
// thread.h
//
// Copyright (C) Peter Niekamp
//
// This code remains the property of the copyright holder.
// The code contained herein is licensed for use without limitation
//

#include <cstdint>
#include <cstddef>

#define TLS_ABOVE_TP 0
#define GAP_ABOVE_TP 0
#define TP_OFFSET 0
#define DTP_OFFSET 0

struct tls_module
{
  uintptr_t base;
  uintptr_t vaddr;
  size_t len, size, align;
};

struct thread_data
{
  thread_data *self;
  uintptr_t *dtv;
  uintptr_t pad[3];
  uintptr_t canary;
  tls_module *tls;

  int pid;
  int tid;

  char bytes[112];
};

#if defined __unix__

static inline struct thread_data *thread_self()
{
  thread_data *self;
  asm ("mov %%fs:0, %0" : "=r"(self) );

#if TLS_ABOVE_TP
  self = (thread_data*)((uintptr_t)self - sizeof(thread_data) - TP_OFFSET);
#endif

  return self;
}

static inline int thread_set_data_area(void *area)
{
  unsigned long ret;
  asm volatile ("syscall" : "=a"(ret) : "0"(158), "D"(0x1002), "S"(area) : "rcx", "r11", "memory");
  return ret;
}

#endif
