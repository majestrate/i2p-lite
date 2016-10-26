
#include <i2pd/types.h>
#include <i2pd/util.h>

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

char * path_join(const char * base, ...) {
  i2p_filename path = {0};
  char * part = 0;
  int idx = 0;
  int i = 0;
  va_list args;

  idx += snprintf(path, sizeof(path), "%s", base);
  assert(idx > 0);

  va_start(args, base);
  while((part = va_arg(args, char *))) {
    i = snprintf(path+idx, sizeof(path) - idx, "%s%s", __PATH_SEP__, part);
    assert(i > 0);
    idx += i;
  }
  
  va_end(args);
  return strdup(path);
}

// check if a file exists 
int check_file(char * path)
{
  FILE * f = fopen(path, "r");
  if(!f) return 0;
  fclose(f);
  return 1;
}

