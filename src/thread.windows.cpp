//
// thread.cpp
//
// Copyright (C) Peter Niekamp
//
// This code remains the property of the copyright holder.
// The code contained herein is licensed for use without limitation
//

#include "thread.h"
#include <windows.h>
#include <stdint.h>

using pthread_t = void*;
using pthread_attr_t = void*;

//|///////////////////// pthread_create /////////////////////////////////////
extern "C" int pthread_create(pthread_t *thread, pthread_attr_t const *attr, int (*start_routine)(void*), void *start_argument)
{
  DWORD tid;

  HANDLE handle = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)(uintptr_t)start_routine, start_argument, 0, &tid);

  if (!handle)
    return GetLastError();

  *thread = (pthread_t)handle;

  return 0;
}

//|///////////////////// pthread_join ///////////////////////////////////////
extern "C" int pthread_join(pthread_t thread, int *rval)
{
  auto handle = (HANDLE)(thread);

  WaitForSingleObject(handle, INFINITE);

  CloseHandle(handle);

  return 0;
}

//|///////////////////// pthread_detach /////////////////////////////////////
extern "C" int pthread_detach(pthread_t thread)
{
  auto handle = (HANDLE*)(thread);

  CloseHandle(handle);

  return 0;
}
