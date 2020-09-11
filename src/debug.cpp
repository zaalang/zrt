//
// debug.cpp
//
// Copyright (C) Peter Niekamp
//
// This code remains the property of the copyright holder.
// The code contained herein is licensed for use without limitation
//

#include <cstdint>
#include <windows.h>

namespace
{
  size_t itoa(uint64_t value, char *buffer, size_t n, uint64_t radix)
  {
    size_t i, j;

    for(i = n - 1; i != 0; --i)
    {
      buffer[i] = "0123456789abcdef"[value % radix];

      value /= radix;

      if (value == 0)
        break;
    }

    for(j = 0; i < n; ++i, ++j)
      buffer[j] = buffer[i];

    buffer[j] = 0;

    return j;
  }
}

struct string_slice
{
  int64_t len;
  uint8_t *data;
};

//|///////////////////// __debug_print_i64 //////////////////////////////////
extern "C" void __debug_print_i64(int64_t value)
{
  DWORD count = 0;
  char buffer[128];

  if (value < 0)
  {
    buffer[count++] = '-';
    value = -value;
  }

  count += itoa(value, buffer+count, sizeof(buffer)-count, 10);

  buffer[count++] = '\n';

  DWORD written;
  WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), buffer, count, &written, nullptr);
}

//|///////////////////// __debug_print_u64 //////////////////////////////////
extern "C" void __debug_print_u64(uint64_t value)
{
  DWORD count = 0;
  char buffer[128];

  count += itoa(value, buffer+count, sizeof(buffer)-count, 10);

  buffer[count++] = '\n';

  DWORD written;
  WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), buffer, count, &written, nullptr);
}

//|///////////////////// __debug_print_string ///////////////////////////////
extern "C" void __debug_print_string(string_slice const &str)
{
  DWORD written;
  WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), str.data, str.len, &written, nullptr);
  WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), "\n", 1, &written, nullptr);
}
