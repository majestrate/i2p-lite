#ifndef I2PD_UTIL_H_
#define I2PD_UTIL_H_

char * path_join(const char * base, ...);

int check_file(char * path);

int _is_dir(char * path, void * u);

int _is_file(char * path, void * u);

#define is_file(path) _is_file(path, NULL)
#define is_dir(path) _is_dir(path, NULL)

typedef int(*dir_filter)(char *, void *);

typedef void(*dir_iterator)(char *, void *);

int iterate_all_files(char * path, dir_iterator i, void * u);

int iterate_all_dirs(char * path, dir_iterator i, void * u);

int iterate_all_with_filter(char * path, dir_iterator i, dir_filter f, void * u);

#endif
