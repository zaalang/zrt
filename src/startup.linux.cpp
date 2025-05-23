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

struct Elf64_Ehdr
{
  unsigned char e_ident[16];
  uint16_t e_type;
  uint16_t e_machine;
  uint32_t e_version;
  uint64_t e_entry;
  uint64_t e_phoff;
  uint64_t e_shoff;
  uint32_t e_flags;
  uint16_t e_ehsize;
  uint16_t e_phentsize;
  uint16_t e_phnum;
  uint16_t e_shentsize;
  uint16_t e_shnum;
  uint16_t e_shstrndx;
};

struct Elf64_Dyn
{
  uint64_t d_tag;
  uint64_t d_val;
};

struct Elf64_Sym
{
  uint32_t st_name;
  uint8_t st_info;
  uint8_t st_other;
  uint16_t st_shndx;
  uint64_t st_value;
  uint64_t st_size;
};

extern "C" uintptr_t vdso_clock_getres;
extern "C" uintptr_t vdso_clock_gettime;

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
    __builtin_memset((void*)tlsbase, 0, tls.size);
    __builtin_memcpy((void*)tlsbase, (void*)(tls.base + tls.vaddr), tls.len);

    td->self = td;
    td->canary = 0xdeadbeef;
    td->tls = &tls;
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

  bool strcmp(const char *lhs, const char *rhs)
  {
    for(; *lhs == *rhs && *rhs != 0; ++lhs, ++rhs)
      ;

    return *lhs == *rhs;
  }

  void init_vdso(char **envp)
  {
    int envc = 0;
    while (envp[envc])
      ++envc;

    size_t *auxv = (size_t*)(envp + envc + 1);

    uintptr_t ehdr = 0;

    for(int i = 0; auxv[i]; i += 2)
    {
      if (auxv[i] == 33) // AT_SYSINFO_EHDR
      {
        ehdr = auxv[i+1];
        break;
      }
    }

    if (!ehdr)
      return;

    uintptr_t base = 0;
    uintptr_t dynv = 0;

    for(size_t i = 0; i < ((Elf64_Ehdr*)ehdr)->e_phnum; ++i)
    {
      auto entry = (Elf64_Phdr*)(ehdr + ((Elf64_Ehdr*)ehdr)->e_phoff + i*((Elf64_Ehdr*)ehdr)->e_phentsize);

      switch (entry->p_type)
      {
        case 1: // PT_LOAD
          base = ehdr + entry->p_offset - entry->p_vaddr;
          break;

        case 2: // PT_DYNAMIC
          dynv = ehdr + entry->p_offset;
          break;
      }
    }

    if (!base || !dynv)
      return;

    uintptr_t strtab = 0;
    uintptr_t hashtb = 0;
    uintptr_t symtab = 0;

    for(int i = 0; ((uint64_t*)dynv)[i]; i += 2)
    {
      auto entry = (Elf64_Dyn*)((uintptr_t*)dynv + i);

      switch (entry->d_tag)
      {
        case 5: // DT_STRTAB
          strtab = base + entry->d_val;
          break;

        case 6: // DT_SYMTAB
          symtab = base + entry->d_val;
          break;

        case 4: // DT_HASH
          hashtb = base + entry->d_val;
          break;
      }
    }

    if (!strtab || !hashtb || !symtab)
      return;

    for(int i = 0; i < ((int32_t*)hashtb)[1]; ++i)
    {
       auto entry = (Elf64_Sym*)(symtab) + i;

       if (strcmp((const char*)(strtab + entry->st_name), "__vdso_clock_getres"))
         vdso_clock_getres = base + entry->st_value;

       if (strcmp((const char*)(strtab + entry->st_name), "__vdso_clock_gettime"))
         vdso_clock_gettime = base + entry->st_value;
    }
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

    init_vdso(envp);

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
      ".att_syntax\n"
    );
  }
}
