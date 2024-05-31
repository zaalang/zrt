//
// fd.cpp
//
// Copyright (C) Peter Niekamp
//
// This code remains the property of the copyright holder.
// The code contained herein is licensed for use without limitation
//

#include "fd.h"
#include <windows.h>

namespace
{
  HANDLE get_osfhandle(uintptr_t fd)
  {
    return (HANDLE)fd;
  }

  uintptr_t set_osfhandle(HANDLE handle)
  {
    /* can this even happen ? */
    while (handle == (HANDLE)0 || handle == (HANDLE)1 || handle == (HANDLE)2)
    {
      DuplicateHandle(GetCurrentProcess(), handle, GetCurrentProcess(), &handle, 0, FALSE, DUPLICATE_SAME_ACCESS);
    }

    return (uintptr_t)handle;
  }
}

//|///////////////////// fd_open ////////////////////////////////////////////
extern "C" uint32_t fd_open(uintptr_t *fd, string path, uint32_t oflags, uint64_t rights, uint32_t fdflags)
{
  WCHAR filename[4096];

  auto nchars = MultiByteToWideChar(CP_UTF8, 0, (LPCCH)path.data, path.len, filename, sizeof(filename) / 2);

  if (nchars == 0)
    return GetLastError();

  auto access = 0;

  if (rights & fd::read)
    access |= GENERIC_READ;

  if (rights & fd::write)
    access |= GENERIC_WRITE;

  if (fdflags & fd::append)
    access = FILE_APPEND_DATA;

  auto disposition = OPEN_EXISTING;

  if ((oflags & (fd::create | fd::exclusive)) == (fd::create | fd::exclusive))
    disposition = CREATE_NEW;

  if ((oflags & (fd::create | fd::exclusive | fd::trunc)) == (fd::create | fd::trunc))
    disposition = CREATE_ALWAYS;

  if ((oflags & (fd::create | fd::exclusive | fd::trunc)) == fd::create)
    disposition = OPEN_ALWAYS;

  if ((oflags & (fd::create | fd::exclusive | fd::trunc)) == fd::trunc)
    disposition = TRUNCATE_EXISTING;

  auto handle = CreateFileW(filename, access, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, disposition, FILE_ATTRIBUTE_NORMAL, NULL);

  if (handle == INVALID_HANDLE_VALUE)
    return GetLastError();

  *fd = set_osfhandle(handle);

  return 0;
}

//|///////////////////// fd_stat ////////////////////////////////////////////
extern "C" uint32_t fd_stat(uintptr_t fd, filestat *fs)
{
  auto handle = get_osfhandle(fd);

  BY_HANDLE_FILE_INFORMATION info;

  if (auto rc = GetFileInformationByHandle(handle, &info); !rc)
    return GetLastError();

  fs->type = filetype::regular;
  fs->size = ((uint64_t)info.nFileSizeHigh << 32) + info.nFileSizeLow;
  fs->atime = (((uint64_t)info.ftLastAccessTime.dwHighDateTime << 32) + info.ftLastAccessTime.dwLowDateTime - 116444736000000000) * 100;
  fs->mtime = (((uint64_t)info.ftLastWriteTime.dwHighDateTime << 32) + info.ftLastWriteTime.dwLowDateTime - 116444736000000000) * 100;
  fs->ctime = (((uint64_t)info.ftCreationTime.dwHighDateTime << 32) + info.ftCreationTime.dwLowDateTime - 116444736000000000) * 100;

  return 0;
}

//|///////////////////// fd_readv ///////////////////////////////////////////
extern "C" fd_result fd_readv(uintptr_t fd, iovec *iovs, uint64_t n)
{
  fd_result result = {};

  if (fd == STDIN)
  {
    fd = (uintptr_t)GetStdHandle(STD_INPUT_HANDLE);
  }

  auto handle = get_osfhandle(fd);

  for(uint64_t i = 0; i < n; ++i)
  {
    DWORD bytes;

    if (auto rc = ReadFile(handle, iovs[i].data, iovs[i].len, &bytes, nullptr); !rc)
    {
      result.erno = GetLastError();

      if (result.erno == ERROR_BROKEN_PIPE)
        result.erno = 0;

      break;
    }

    result.length += bytes;

    if (bytes != iovs[i].len)
      break;
  }

  return result;
}

//|///////////////////// fd_preadv //////////////////////////////////////////
extern "C" fd_result fd_preadv(uintptr_t fd, iovec *iovs, uint64_t n, uint64_t offset)
{
  fd_result result = {};

  auto handle = get_osfhandle(fd);

  OVERLAPPED overlapped = {};

  overlapped.Offset = offset & 0xffffffff;
  overlapped.OffsetHigh = (offset >> 32) & 0xffffffff;

  for(uint64_t i = 0; i < n; ++i)
  {
    DWORD bytes;

    if (auto rc = ReadFile(handle, iovs[i].data, iovs[i].len, &bytes, &overlapped); !rc)
    {
      result.erno = GetLastError();

      if (result.erno == ERROR_BROKEN_PIPE)
        result.erno = 0;

      break;
    }

    result.length += bytes;

    if (bytes != iovs[i].len)
      break;
  }

  return result;
}

//|///////////////////// fd_writev //////////////////////////////////////////
extern "C" fd_result fd_writev(uintptr_t fd, ciovec const *iovs, uint64_t n)
{
  fd_result result = {};

  if (fd == STDOUT || fd == STDERR)
  {
    auto handle = GetStdHandle((fd == STDOUT) ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE);

    DWORD mode;
    if (GetConsoleMode(handle, &mode))
    {
      for(uint64_t i = 0; i < n; ++i)
      {
        auto beg = iovs[i].data;
        auto end = iovs[i].data + iovs[i].len;

        while (beg != end)
        {
          WCHAR buffer[4096];

          size_t len = end - beg;

          if (len > sizeof(buffer) / 2)
          {
            len = sizeof(buffer) / 2;

            while (len > sizeof(buffer)/2-4 && (beg[len] & 0xC0) == 0x80)
              --len;
          }

          auto nchars = MultiByteToWideChar(CP_UTF8, 0, (LPCCH)beg, len, buffer, sizeof(buffer) / 2);

          DWORD written;
          WriteConsoleW(handle, buffer, nchars, &written, nullptr);

          beg += len;
        }

        result.length += iovs[i].len;
      }

      return result;
    }

    fd = (uintptr_t)handle;
  }

  auto handle = get_osfhandle(fd);

  for(uint64_t i = 0; i < n; ++i)
  {
    DWORD written;

    if (auto rc = WriteFile(handle, iovs[i].data, iovs[i].len, &written, nullptr); !rc)
    {
      result.erno = GetLastError();
      break;
    }

    result.length += written;
  }

  return result;
}

//|///////////////////// fd_pwritev /////////////////////////////////////////
extern "C" fd_result fd_pwritev(uintptr_t fd, ciovec const *iovs, uint64_t n, uint64_t offset)
{
  fd_result result = {};

  auto handle = get_osfhandle(fd);

  OVERLAPPED overlapped = {};

  overlapped.Offset = offset & 0xffffffff;
  overlapped.OffsetHigh = (offset >> 32) & 0xffffffff;

  for(uint64_t i = 0; i < n; ++i)
  {
    DWORD written;

    if (auto rc = WriteFile(handle, iovs[i].data, iovs[i].len, &written, &overlapped); !rc)
    {
      result.erno = GetLastError();
      break;
    }

    result.length += written;
  }

  return result;
}

//|///////////////////// fd_close ///////////////////////////////////////////
extern "C" uint32_t fd_close(uintptr_t fd)
{
  CloseHandle(get_osfhandle(fd));

  return 0;
}
