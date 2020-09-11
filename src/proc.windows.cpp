//
// proc.cpp
//
// Copyright (C) Peter Niekamp
//
// This code remains the property of the copyright holder.
// The code contained herein is licensed for use without limitation
//

#include <cstdint>
#include <windows.h>

//|///////////////////// proc_exit //////////////////////////////////////////
extern "C" void proc_exit(uint32_t rval)
{
  ExitProcess(rval);
}
