//
// tests.cpp
//
// Copyright (C) 2020 Peter Niekamp. All rights reserved.
//
// This file is part of zaalang, which is BSD-2-Clause licensed.
// See http://opensource.org/licenses/BSD-2-Clause
//

#include "fd.h"
#include <iostream>

using namespace std;

//|//////////////////// main ////////////////////////////////////////////////
int main(int argc, char *args[])
{
  cout << "zrt tests\n" << endl;

  try
  {
    ciovec io;
    io.data = (uint8_t const *)"Hello World\n";
    io.len = 12;

    fd_writev(STDOUT, &io, 1);
  }
  catch(exception &e)
  {
    cout << "** " << e.what() << endl;
  }

  cout << endl;
}
