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

namespace
{
  thread_data module;

  void init_tls(thread_data *td)
  {
    td->self = td;
    td->canary = 0xdeadbeef;

    thread_set_data_area(td);
  }
}

extern "C" {

  int main(int, char**, char**);

  void __start(int argc, char** argv, char** envp)
  {
    init_tls(&module);

    proc_exit(main(argc, argv, envp));
  }

  void __attribute__((naked)) _start()
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
