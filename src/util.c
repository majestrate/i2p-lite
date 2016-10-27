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
  i2p_debug(LOG_UTIL, "open dir %s", path);
  DIR * d = opendir(path);
  if (!d) {
    i2p_error(LOG_UTIL, "failed to open %s", path);
    return 0;
  }

  struct dirent * dent = readdir(d);
  if(dent) {
    struct dirent * ent = NULL;
    
    while(!readdir_r(d, dent, &ent)) {
      if(ent == NULL) break;
      if(strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..")) {
        char * fpath = path_join(path, ent->d_name, 0);
        i2p_debug(LOG_UTIL, "visit %s", fpath);
        if(f(fpath, u)) i(fpath, u);
        free(fpath);
      }
    }
  } else {
    i2p_error(LOG_UTIL, "failed to read dir %s", path);
  }
  if(errno)
    i2p_error(LOG_UTIL, "error: %s", strerror(errno));
  
  closedir(d);
  i2p_debug(LOG_UTIL, "closed %s", path);
  return errno == 0;
}
