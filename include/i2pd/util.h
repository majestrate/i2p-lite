#ifndef I2PD_UTIL_H_
#define I2PD_UTIL_H_

char * path_join(const char * base, ...);

int check_file(char * path);

typedef void(*dir_iterator)(char *, void *);

void iterate_all_files(char * path, dir_iterator i, void * u);

void iterate_all_dirs(char * path, dir_iterator i, void * u);

#endif
