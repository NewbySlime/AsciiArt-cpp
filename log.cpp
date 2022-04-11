#include "log.h"

#ifdef DO_LOG

#include "stdlib.h"
#include "stdarg.h"

const char filepath_log[] = "LOG.txt";
FILE *log_FILE = NULL;

void PRINTLOG(const char *format, ...){
  va_list args;
  va_start(args, format);
  vfprintf(log_FILE, format, args);
  va_end(args);
}

void __OnExit(){
  if(log_FILE != stderr)
    fclose(log_FILE);
}

int __Init(){
  log_FILE = fopen(filepath_log, "w");
  if(log_FILE == NULL){
    printf("Cannot write output log file.\nUsing stderr...\n\n");
    log_FILE = stderr;
  }

  atexit(__OnExit);
}

int __tmp = __Init();

#endif