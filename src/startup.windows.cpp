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
#include "windows.h"
#include <stdint.h>

namespace
{
  void parse_args(HANDLE heap, wchar_t *cmdline, int &argc, char **&argv)
  {
    argc = 0;
    argv = (char**)HeapAlloc(heap, 0, sizeof(char*) * 256);

    for (auto ch = cmdline; *ch; )
    {
      while (*ch == ' ')
        ++ch;

      auto beg = ch;
      auto end = ch;

      if (ch[0] == '"')
      {
        ++beg;
        ++ch;

        while (*ch && ch[0] != '"')
        {
          if (ch[0] == '\\' && ch[1] != 0)
            ++ch;

          ++ch;
        }

        end = ch;

        if (*ch)
          ++ch;
      }
      else
      {
        while (*ch && ch[0] != ' ')
          ++ch;

        end = ch;
      }

      if (beg != end)
      {
        if (argc % 256 == 255)
          argv = (char**)HeapReAlloc(heap, 0, argv, sizeof(char*) * (argc + 257));

        auto n = end - beg;
        auto arg = (char*)HeapAlloc(heap, 0, n * sizeof(uint32_t) + 1);
        auto nchars = WideCharToMultiByte(CP_UTF8, 0, beg, n, arg, n * sizeof(uint32_t), nullptr, nullptr);

        arg[nchars] = 0;
        argv[argc] = arg;
        argc++;
      }
    }

    argv[argc] = nullptr;
  }

  void parse_envp(HANDLE heap, wchar_t *env, int &envc, char **&envp)
  {
    envc = 0;
    envp = (char**)HeapAlloc(heap, 0, sizeof(char*) * 256);

    for (auto ch = env; *ch; )
    {
      auto beg = ch;

      while (*ch)
        ++ch;

      auto end = ch;

      if (beg != end)
      {
        if (envc % 256 == 255)
          envp = (char**)HeapReAlloc(heap, 0, envp, sizeof(char*) * (envc + 257));

        auto n = end - beg;
        auto arg = (char*)HeapAlloc(heap, 0, n * sizeof(uint32_t) + 1);
        auto nchars = WideCharToMultiByte(CP_UTF8, 0, beg, n, arg, n * sizeof(uint32_t), nullptr, nullptr);

        arg[nchars] = 0;
        envp[envc] = arg;
        envc++;
      }

      ++ch;
    }

    envp[envc] = nullptr;
  }
}

extern "C" {

  int _fltused = 0;

  int main(int, char**, char**);

  int mainCRTStartup()
  {
    int argc;
    char** argv;
    int envc;
    char** envp;

    auto heap = HeapCreate(0, 0, 0);

    parse_args(heap, GetCommandLineW(), argc, argv);
    parse_envp(heap, GetEnvironmentStringsW(), envc, envp);

    return main(argc, argv, envp);
  }

  int WinMainCRTStartup()
  {
    int argc;
    char** argv;
    int envc;
    char** envp;

    auto heap = HeapCreate(0, 0, 0);

    parse_args(heap, GetCommandLineW(), argc, argv);
    parse_envp(heap, GetEnvironmentStringsW(), envc, envp);

    return main(argc, argv, envp);
  }

#if defined __MINGW64__

  void __main()
  {
  }

#endif

}
