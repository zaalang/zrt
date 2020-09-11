//
// stack.cpp
//
// Copyright (C) Peter Niekamp
//
// This code remains the property of the copyright holder.
// The code contained herein is licensed for use without limitation
//

#include "fd.h"
#include "proc.h"
#include "thread.h"
#include "crash.h"

uintptr_t __stack_chk_guard = 0xdeadbeef;

extern "C" void __stack_chk_fail()
{
  crash("stack smashed\n", 14);
}

extern "C" void __stack_chk_fail_local()
{
  __stack_chk_fail();
}

#ifdef __WIN32

extern "C" void ___chkstk_ms()
{
  asm volatile (
    ".intel_syntax noprefix\n"
    "    push %rax\n"
    "    push %rcx\n"
    "    cmp %rax, 0x1000\n"
    "    lea %rcx, [%rsp+24]\n"
    "    jb 1f\n"
    " 2: sub %rcx, 0x1000\n"
    "    test [%rcx], %rcx\n"
    "    sub %rax, 0x1000\n"
    "    cmp %rax, 0x1000\n"
    "    ja 2b\n"
    " 1: sub %rcx, %rax\n"
    "    test [%rcx], %rcx\n"
    "    pop %rcx\n"
    "    pop %rax\n"
    "    ret\n"
  );
}

extern "C" void __alloca()
{
  asm volatile (
    ".intel_syntax noprefix\n"
    "    mov %rax, %rcx\n"
    "    jmp ___chkstk\n"
  );
}

extern "C" void ___chkstk()
{
  asm volatile (
    ".intel_syntax noprefix\n"
    "    push %rcx\n"
    "    cmp %rax, 0x1000\n"
    "    lea %rcx, [%rsp+16]\n"
    "    jb 1f\n"
    " 2: sub %rcx, 0x1000\n"
    "    test [%rcx], %rcx\n"
    "    sub %rax, 0x1000\n"
    "    cmp %rax, 0x1000\n"
    "    ja 2b\n"
    " 1: sub %rcx, %rax\n"
    "    test [%rcx], %rcx\n"
    "    lea %rax, [%rsp+8]\n"
    "    mov %rsp, %rcx\n"
    "    mov %rcx, [%rax-8]\n"
    "    push [%rax]\n"
    "    sub %rax, %rsp\n"
    "    ret\n"
  );
}

#endif
