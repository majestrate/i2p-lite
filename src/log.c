#include <i2pd/log.h>
#include <i2pd/memory.h>

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

struct log_ctx
{
  int level;
  int scope;
  FILE * f;
};

static struct log_ctx * log;

void i2p_log_init()
{
  log = mallocx(sizeof(struct log_ctx), MALLOCX_ZERO);
  log->f = stderr;
}

void i2p_log_set_level(int level)
{
  log->level = level;
}

void i2p_log_set_scope(int scope)
{
  log->scope = scope;
}

void i2p_log_end()
{
  // close file if log file
  if(log->f != stderr &&  log->f != stdout)
    fclose(log->f);
  free(log);
  log = NULL;
}

void __i2p_log(int level, int lineno, const char * f, int scope, const char * fmt, ...)
{
  int color;
  va_list args;
  const char * lc;
  if(!log) return;
  if(!(log->scope & scope)) return;
  if(log->level > level) return;
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
  fprintf(log->f, "\033[1;%dm[%s %lu %s:%d] ", color, lc, time(NULL), f, lineno);
  va_start(args, fmt);
  vfprintf(log->f, fmt, args);
  va_end(args);
  fprintf(log->f, "\033[0m\n");
}

// dump memory
void __i2p_debug_memory(int line, const char * f, int scope, const uint8_t * begin, const uint8_t * end)
{
  uint8_t * p;
  size_t c = 1;
  if(!log) return;
  if(log->scope & scope && log->level == L_DEBUG)
  {
    p = (uint8_t*) begin;
    fprintf(log->f, "[MEM %s:%d] %p -> %p\n%p ", f, line, begin, end, p);
    while(p != end) {
      if( c % 16 == 0)
        fprintf(log->f, " %.2x\n%p ", *p, p);
      else
        fprintf(log->f, " %.2x", *p);
      p++;
      c++;
    }
    fprintf(log->f, "\n");
  }
}
