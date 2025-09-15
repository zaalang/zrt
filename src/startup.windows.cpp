//
// startup.cpp
//
// Copyright (C) Peter Niekamp
//
// This code remains the property of the copyright holder.
// The code contained herein is licensed for use without limitation
//

#include "thread.h"
#include <windows.h>
#include <stdint.h>

#if defined(_MSC_VER)
#pragma section(".CRT$XLA",long,read)
#pragma section(".CRT$XLB",long,read)
#pragma section(".CRT$XLZ",long,read)
#pragma section(".rdata$T",long,read)
#pragma section(".tls",long,read,write)
#pragma section(".tls$AAA",long,read,write)
#pragma section(".tls$ZZZ",long,read,write)
#pragma comment(linker, "/merge:.CRT=.rdata")
#endif

#if defined(_MSC_VER)
#define _CRTALLOC(x) __declspec(allocate(x))
#elif defined(__GNUC__)
#define _CRTALLOC(x) __attribute__ ((section (x) ))
#else
#error Your compiler is not supported.
#endif

extern "C" {

  unsigned long _tls_index = 0;

  _CRTALLOC(".tls") char *_tls_start = NULL;
  _CRTALLOC(".tls$ZZZ") char *_tls_end = NULL;

  _CRTALLOC(".CRT$XLA") PIMAGE_TLS_CALLBACK __xl_a = NULL;
  _CRTALLOC(".CRT$XLZ") PIMAGE_TLS_CALLBACK __xl_z = NULL;

  IMAGE_TLS_DIRECTORY _tls_used = {
    (ULONG_PTR) &_tls_start,
    (ULONG_PTR) &_tls_end,
    (ULONG_PTR) &_tls_index,
    (ULONG_PTR) (&__xl_a + 1),
    (ULONG) 0,
    (ULONG) 0
  };

  //VOID WINAPI __tls_callback(HANDLE hDllHandle, DWORD dwReason, LPVOID lpreserved)
  //{
  //  __debugbreak();
  //}

  //_CRTALLOC(".CRT$XLB") PIMAGE_TLS_CALLBACK __xl_b = (PIMAGE_TLS_CALLBACK) __tls_callback;
}

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

int main(int, char**, char**);

extern "C" {

  int _fltused = 0;

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
