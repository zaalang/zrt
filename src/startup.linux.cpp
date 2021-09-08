//
// startup.cpp
//
// Copyright (C) Peter Niekamp
//
// This code remains the property of the copyright holder.
// The code contained herein is licensed for use without limitation
//

#include "proc.h"
#include "thread.h"
#include <alloca.h>

struct Elf64_Phdr
{
  uint32_t p_type;
  uint32_t p_flags;
  uint64_t p_offset;
  uint64_t p_vaddr;
  uint64_t p_paddr;
  uint64_t p_filesz;
  uint64_t p_memsz;
  uint64_t p_align;
};

namespace
{
  tls_module tls;

  void init_tcb(void *memory)
  {
#if TLS_ABOVE_TP
    auto tlsbase = ((uintptr_t)memory + sizeof(thread_data) + 2*sizeof(void*) + GAP_ABOVE_TP + tls.align - 1) & (tls.align - 1);
    auto dtv = (uintptr_t*)(tlsbase - 2*sizeof(void*) - GAP_ABOVE_TP);
    auto td = (thread_data*)(tlsbase - sizeof(thread_data) - 2*sizeof(void*) - GAP_ABOVE_TP);
#else
    auto tlsbase = ((uintptr_t)memory + tls.align - 1) & -tls.align;
    auto dtv = (uintptr_t*)(tlsbase + tls.size + sizeof(thread_data));
    auto td = (thread_data*)(tlsbase + tls.size);
#endif

    dtv[0] = 1;
    dtv[1] = tlsbase + DTP_OFFSET;
    __builtin_memcpy((void*)tlsbase, (void*)(tls.base + tls.vaddr), tls.len);

    td->self = td;
    td->canary = 0xdeadbeef;

    td->dtv = dtv;

#if TLS_ABOVE_TP
    thread_set_data_area((thread_data*)((uintptr_t)td + sizeof(thread_data) + TP_OFFSET);
#else
    thread_set_data_area(td);
#endif
  }

  int tls_area(char **envp)
  {
    int envc = 0;
    while (envp[envc])
      ++envc;

    size_t *auxv = (size_t*)(envp + envc + 1);

    uintptr_t phdr = 0;
    size_t phnum = 0;
    size_t phent = 0;

    for(int i = 0; auxv[i]; i += 2)
    {
      switch (auxv[i])
      {
        case 3: // AT_PHDR
          phdr = auxv[i+1];
          break;

        case 4: // AT_PHENT
          phent = auxv[i+1];
          break;

        case 5: // AT_PHNUM
          phnum = auxv[i+1];
          break;
      }
    }

    for(size_t i = 0; i < phnum; ++i)
    {
      auto entry = (Elf64_Phdr*)(phdr + i*phent);

      switch (entry->p_type)
      {
        case 6: // PT_PHDR
          tls.base = phdr - entry->p_vaddr;
          break;

        case 7: // PT_TLS
          tls.vaddr = entry->p_vaddr;
          tls.len = entry->p_filesz;
          tls.size = entry->p_memsz;
          tls.align = entry->p_align;
          break;
      }
    }

    if (tls.align < 1)
      tls.align = 1;

    tls.size = (tls.size + tls.align - 1) & -tls.align;

    return 2*sizeof(void*) + sizeof(thread_data) + GAP_ABOVE_TP + tls.size + tls.align;
  }
}

extern "C" {

  void *__tls_get_addr(size_t *v)
  {
    auto self = thread_self();
    return (void *)(self->dtv[v[0]] + v[1]);
  }
}

extern "C" {

  int main(int, char**, char**);

  void __start(int argc, char** argv, char** envp)
  {
    init_tcb(alloca(tls_area(envp)));

    exit(main(argc, argv, envp));
  }

  __attribute__((naked)) void _start()
  {
    asm(
      ".intel_syntax noprefix\n"
      "    xor rbp, rbp\n"
      "    mov rdi, [rsp]\n"          // argc
      "    lea rsi, [rsp+8]\n"        // argv
      "    lea rdx, [rsp+rdi*8+16]\n" // envp
      "    and rsp, -16\n"
      "    call __start\n"
      "    hlt\n"
    );
  }
}


