//
// thread.h
//
// Copyright (C) Peter Niekamp
//
// This code remains the property of the copyright holder.
// The code contained herein is licensed for use without limitation
//

struct thread_data
{
  thread_data *self;  
  uintptr_t pad[4];
  uintptr_t canary;

  char bytes[128];
};

#if defined __unix__

static inline struct thread_data *thread_self()
{
  thread_data *self;
  asm ("mov %%fs:0, %0" : "=r"(self) );
  return self;
}

static inline int thread_set_data_area(void *area)
{
  unsigned long ret;
  asm volatile ("syscall" : "=a"(ret) : "0"(158), "D"(0x1002), "S"(area) : "rcx", "r11", "memory");
  return ret;
}

#endif
