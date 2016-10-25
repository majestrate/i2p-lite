#include <i2pd/log.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

struct log_ctx
{
  int level;
  int scope;
};

static struct log_ctx l;

void i2p_log_init(int level, int scope)
{
  l.level = level;
  l.scope = scope;
}

void __i2p_log(int level, int lineno, const char * f, int scope, const char * fmt, ...)
{
  int color;
  va_list args;
  const char * lc;
  if(!(l.scope & scope)) return;
  if(l.level > level) return;
  switch(level) {
  default:
    color = 31;
    lc = "???";
    break;
  case L_INFO:
    color = 36;
    lc = "NFO";
    break;
  case L_DEBUG:
    color = 37;
    lc = "DBG";
    break;
  case L_WARN:
    color = 33;
    lc = "WRN";
    break;
  case L_ERROR:
    color = 31;
    lc = "ERR";
    break;
  }
  printf("\033[1;%dm[%s %lu %s:%d]\t", color, lc, time(NULL), f, lineno);
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
  printf("\033[0m\n");
}

// dump memory to stdout
void __i2p_debug_memory(int line, const char * f, int scope, const uint8_t * begin, const uint8_t * end)
{
  uint8_t * p;
  size_t c = 1;
  if(l.scope & scope && l.level == L_DEBUG)
  {
    p = (uint8_t*) begin;
    printf("[MEM %s:%d] %p -> %p\n%p ", f, line, begin, end, p);
    while(p != end) {
      if( c % 16 == 0)
        printf(" %.2x\n%p ", *p, p);
      else
        printf(" %.2x", *p);
      p++;
      c++;
    }
    printf("\n");
  }
}
