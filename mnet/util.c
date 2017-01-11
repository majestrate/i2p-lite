#include <i2pd/util.h>
#include <i2pd/types.h>
#include <i2pd/memory.h>
#include <i2pd/log.h>

#include <assert.h>
#include <dirent.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

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

int del_file(char * path)
{
  return unlink(path);
}

int _is_file(char * path, void * u)
{
  (void) u;
  struct stat st;
  if(stat(path, &st) == -1) {
    i2p_error(LOG_UTIL, "stat(%s): %s", path, strerror(errno));
    return 0;
  }
  return S_ISREG(st.st_mode);
}

int _is_dir(char * path, void * u)
{
  (void) u;
  struct stat st; 
  if(stat(path, &st) == -1) {
    i2p_error(LOG_UTIL, "stat(%s): %s", path, strerror(errno));
    return 0;
  }
  return S_ISDIR(st.st_mode);
}

int iterate_all_files(char * path, dir_iterator i, void * u)
{
  return iterate_all_with_filter(path, i, _is_file, u);
}

int iterate_all_dirs(char * path, dir_iterator i, void * u)
{
  return iterate_all_with_filter(path, i, _is_dir, u);
}

int iterate_all_with_filter(char * path, dir_iterator i, dir_filter f, void * u)
{
  DIR * d = opendir(path);
  if (!d) {
    i2p_error(LOG_UTIL, "failed to open %s", path);
    return 0;
  }

  struct dirent * ent ;

  int r = 0;
  while((ent = readdir(d))) {
    if(ent == NULL) break;
    if(strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..")) {
      char * fpath = path_join(path, ent->d_name, 0);
      if(f(fpath, u)) i(fpath, u);
      free(fpath);
    }
  }
  
  closedir(d);
  return errno == 0;
}

uint8_t * read_bytestring(uint8_t * buf, size_t len, uint8_t ** bs, size_t * sz)
{
  
}

uint8_t * read_i2pstring(uint8_t * buf, size_t len, char ** str)
{
  uint8_t sz = *buf;
  buf ++;
  if(sz >= len) return NULL; // overflow
  *str = mallocx(sz+1, MALLOCX_ZERO);
  memcpy(*str, buf, sz);
  i2p_debug(LOG_UTIL, "read string %s size %d", *str, sz);
  return buf + sz;
}

uint8_t * read_i2pdict(uint8_t * buf, size_t len, i2pdict_reader r, void * u)
{
  char k[256] = {0};
  char v[256] = {0};
  uint16_t sz = bufbe16toh(buf);
  
  if(sz > len) {
    i2p_error(LOG_UTIL, "read_i2pdict() overflow: %d > %d", sz, len);
    return NULL; // overflow
  }
  i2p_debug(LOG_UTIL, "read dict of size %d", sz);
  
  uint8_t s = 0;
  buf += sizeof(uint16_t);
  while(sz) {
    // key
    s = *buf ++;
    memcpy(k, buf, s);
    k[s] = 0;
    sz -= s + 2;
    buf += s + 1;
    // value
    s = *buf ++;
    memcpy(v, buf, s);
    v[s] = 0;
    buf += s + 1;
    sz -= s + 2;
    // process
    r(k, v, u);
  }
  return buf;
}
