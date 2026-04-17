/* cpuid.h stub for clang-cl on Windows
   Translates GCC 5-arg __cpuid macro to MSVC 2-arg __cpuid intrinsic */
#pragma once

#ifdef __cpuid
#undef __cpuid
#endif

#define __cpuid(level, a, b, c, d)  \
  do {                              \
    int _r[4];                      \
    __cpuid(_r, (int)(level));      \
    (a) = (unsigned int)_r[0];      \
    (b) = (unsigned int)_r[1];      \
    (c) = (unsigned int)_r[2];      \
    (d) = (unsigned int)_r[3];      \
  } while (0)
