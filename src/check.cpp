//
// check.cpp
//
// Copyright (C) Peter Niekamp
//
// This code remains the property of the copyright holder.
// The code contained herein is licensed for use without limitation
//

#include "crash.h"

extern "C" void __div0_chk_fail()
{
  crash("divide by zero\n", 15);
}

extern "C" void __carry_chk_fail()
{
  crash("overflow\n", 9);
}

extern "C" void __null_chk_fail()
{
  crash("null reference\n", 15);
}
