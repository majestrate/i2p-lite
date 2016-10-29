#ifndef I2PD_UTIL_H_
#define I2PD_UTIL_H_

#include <unistd.h>
#include <stdint.h>

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

/** @brief read i2p string from buffer, allocates result and puts it into str, returns where we left off in buffer or NULL on error */
uint8_t * read_i2pstring(uint8_t * buff, size_t buflen, char ** str);

/** @brief callback for reading entries in an i2p dict */
typedef void(*i2pdict_reader)(char *, char *, void *);

/** @brief read an i2p dict, call callback for each entry, return where we left off in buffer or NULL on error */
uint8_t * read_i2pdict(uint8_t * buf, size_t buflen, i2pdict_reader r, void * u);

#endif
