#
# zrt
#

set(CMAKE_CXX_STANDARD 17)

if(UNIX OR MINGW)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wfloat-conversion -Wno-unused-parameter -Wno-missing-field-initializers")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-builtin -fno-stack-protector -fno-exceptions")
endif(UNIX OR MINGW)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-missing-braces -Wno-char-subscripts")
endif()

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244 /wd4800 /wd4267 /wd4146 /wd4814")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Oi- /GS- /EHa-")
  STRING (REGEX REPLACE "/RTC[^ ]*" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
endif(MSVC)

if(WIN32)
  add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS -D_WINSOCK_DEPRECATED_NO_WARNINGS)
endif(WIN32)

set(SRCS ${SRCS} fd.h)
set(SRCS ${SRCS} mem.h)
set(SRCS ${SRCS} clock.h)
set(SRCS ${SRCS} proc.h)
set(SRCS ${SRCS} thread.h)
set(SRCS ${SRCS} crash.h)

if(UNIX)
  set(SRCS ${SRCS} fd.linux.cpp)
  set(SRCS ${SRCS} mem.linux.cpp)
  set(SRCS ${SRCS} clock.linux.cpp)
  set(SRCS ${SRCS} thread.linux.cpp)
  set(SRCS ${SRCS} startup.linux.cpp)
endif(UNIX)

if(WIN32)
  set(SRCS ${SRCS} fd.windows.cpp)
  set(SRCS ${SRCS} mem.windows.cpp)
  set(SRCS ${SRCS} clock.windows.cpp)
  set(SRCS ${SRCS} thread.windows.cpp)
  set(SRCS ${SRCS} startup.windows.cpp)
endif(WIN32)

set(SRCS ${SRCS} exit.cpp)
set(SRCS ${SRCS} memset.cpp)
set(SRCS ${SRCS} memcpy.cpp)
set(SRCS ${SRCS} memmove.cpp)
set(SRCS ${SRCS} memfind.cpp)
set(SRCS ${SRCS} stringcmp.cpp)
set(SRCS ${SRCS} stack.x64.cpp)
set(SRCS ${SRCS} check.cpp)
set(SRCS ${SRCS} fmod.cpp)
set(SRCS ${SRCS} floor.cpp)
set(SRCS ${SRCS} ceil.cpp)
set(SRCS ${SRCS} round.cpp)
set(SRCS ${SRCS} trunc.cpp)
set(SRCS ${SRCS} frexp.cpp)
set(SRCS ${SRCS} ldexp.cpp)
set(SRCS ${SRCS} sqrt.cpp)

if(MSVC)
  enable_language(ASM_MASM)
  set(SRCS ${SRCS} stack.msvc.asm)
endif(MSVC)

add_library(zrt ${SRCS})

set_property(TARGET zrt PROPERTY POSITION_INDEPENDENT_CODE ON)

#
# install
#

INSTALL(TARGETS zrt DESTINATION lib)
