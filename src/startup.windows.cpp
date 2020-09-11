//
// startup.cpp
//
// Copyright (C) Peter Niekamp
//
// This code remains the property of the copyright holder.
// The code contained herein is licensed for use without limitation
//

extern "C" int main();

extern "C" int mainCRTStartup()
{
  return main();
}

extern "C" int WinMainCRTStartup()
{
  return main();
}

#ifdef __MINGW64__

extern "C" void __main()
{
}

#endif
