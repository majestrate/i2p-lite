#include <mnet/util.h>
#include <mnet/types.h>
#include <mnet/memory.h>
#include <mnet/log.h>

#include <assert.h>
#include <dirent.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

char * path_join(const char * base, ...) {
  mnet_filename path = {0};
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
  return access(path, F_OK) != -1;
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
    mnet_error(LOG_UTIL, "stat(%s): %s", path, strerror(errno));
    errno = 0;
    return 0;
  }
  return S_ISREG(st.st_mode);
}

int _is_dir(char * path, void * u)
{
  (void) u;
  struct stat st; 
  if(stat(path, &st) == -1) {
    mnet_error(LOG_UTIL, "stat(%s): %s", path, strerror(errno));
    errno = 0;
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
    mnet_error(LOG_UTIL, "failed to open %s", path);
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
