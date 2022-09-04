#ifndef OSLIB_PREPROC_HPP_INCLUDED
#define OSLIB_PREPROC_HPP_INCLUDED

#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ < 12
#define OSLIB_GCC_UNKNOWN_REGION_PRAGMA 1
#else
#define OSLIB_GCC_UNKNOWN_REGION_PRAGMA 0
#endif

#endif