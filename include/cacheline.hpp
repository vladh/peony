// Author: Nick Strupat
// Date: October 29, 2010
// Returns the cache line size (in bytes) of the processor, or 0 on failure

#ifndef CACHELINE_H
#define CACHELINE_H

#include <stddef.h>
size_t cacheline_get_size();

#if defined(__APPLE__)

#include <sys/sysctl.h>
size_t cacheline_get_size() {
  size_t line_size = 0;
  size_t sizeof_line_size = sizeof(line_size);
  sysctlbyname("hw.cachelinesize", &line_size, &sizeof_line_size, 0, 0);
  return line_size;
}

#elif defined(_WIN32)

#include <stdlib.h>
#include <windows.h>
size_t cacheline_get_size() {
  size_t line_size = 0;
  DWORD buffer_size = 0;
  DWORD i = 0;
  SYSTEM_LOGICAL_PROCESSOR_INFORMATION * buffer = 0;

  GetLogicalProcessorInformation(0, &buffer_size);
  buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION *)malloc(buffer_size);
  GetLogicalProcessorInformation(&buffer[0], &buffer_size);

  for (i = 0; i != buffer_size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i) {
    if (buffer[i].Relationship == RelationCache && buffer[i].Cache.Level == 1) {
      line_size = buffer[i].Cache.LineSize;
      break;
    }
  }

  free(buffer);
  return line_size;
}

#elif defined(linux)

#include <stdio.h>
size_t cacheline_get_size() {
  FILE * p = 0;
  p = fopen("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size", "r");
  unsigned int i = 0;
  if (p) {
    fscanf(p, "%d", &i);
    fclose(p);
  }
  return i;
}

#else
#error Unrecognized platform
#endif

#endif
