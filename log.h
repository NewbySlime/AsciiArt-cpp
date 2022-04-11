#ifndef LOGGING_HEADER
#define LOGGING_HEADER

#include "stdio.h"

#ifdef DO_LOG
  void PRINTLOG(const char *format, ...);
#else
  #define PRINTLOG(format, ...)
#endif

#endif