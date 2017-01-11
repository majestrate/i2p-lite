#ifndef MNET_BENCODE_H
#define MNET_BENCODE_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

struct bencode_obj;

/**
   @brief a bencoded object stored in memory to be written out
 */
typedef struct bencode_obj* bencode_obj_t;

/**
   @brief allocate empty dict
 */
void bencode_obj_dict(bencode_obj_t * o);

/**
   @brief allocate new string, copies
 */
void bencode_obj_str(bencode_obj_t * o, const uint8_t * data, size_t sz);

/**
   @brief allocate new int
 */
void bencode_obj_int(bencode_obj_t * o, int64_t i);

/**
   @brief allocate empty list
 */
void bencode_obj_list(bencode_obj_t * o);

/**
   @brief recursively free o
 */
void bencode_obj_free(bencode_obj_t * o);

/**
   @brief clones o into c
 */
void bencode_obj_clone(bencode_obj_t o, bencode_obj_t * c);

/**
   @brief determine if o is a dict
   @return 1 if dictionary otherwise return 0
 */
int bencode_obj_is_dict(bencode_obj_t o);

/**
   @brief get pointer to string if it's a string
   @param str pointer to contain copy of string, caller must NOT free after if not NULL
   @return -1 if this is not a string otherwise retuns size of string
 */
ssize_t bencode_obj_getstr(bencode_obj_t o, uint8_t ** str);

/**
   @return 1 if o is a list otherwise return 0
 */
int bencode_obj_is_list(bencode_obj_t o);

/**
   @brief get as integer
   @param i pointer to result
   @return 0 if this is not an int otherwise return 1
 */
int bencode_obj_getint(bencode_obj_t o, int64_t * i);

/**
   @brief iterator for iterating over all dict items in a bencoded dict
 */
typedef void (*bencode_obj_dict_iter)(bencode_obj_t, const char *, bencode_obj_t, void*);

/**
   @brief iterate over all items in a bencoded dict
 */
void bencode_obj_iter_dict(bencode_obj_t d, bencode_obj_dict_iter i, void *u);

/**
   @brief d[k] = v
   @param d dict
   @param k key
   @param v value to be set, copied
 */
void bencode_obj_dict_set(bencode_obj_t d, const char * k, bencode_obj_t v);

/**
   @brief iterator for bencoded lists
 */
typedef void (*bencode_obj_list_iter)(bencode_obj_t, bencode_obj_t, void *);

/**
   @brief iterate over a bencoded list
 */
void bencode_obj_iter_list(bencode_obj_t o, bencode_obj_list_iter i, void * u);

/**
   @brief append to a bencoded list
   @param list a bencoded list
   @param item item to append, is copied
 */
void bencode_obj_list_append(bencode_obj_t list, bencode_obj_t item);

/**
 * @brief write a decoded bencode_t to a file descriptor
 * @param fd the file descriptor to use
 * @return -1 on fail otherwise the number of bytes written
 */
ssize_t bencode_write_fd(bencode_obj_t o, int fd);

/**
   @brief write a bencoded object to an open file
   @param o bencoded object
   @param f open file
   @return -1 on fail otherwise the number of bytes written
 */
ssize_t bencode_write_file(bencode_obj_t o, FILE * f);

/**
 * @param buf pointer to buffer that will contain the serialized entity
 * @return size of buf or -1 on fail
 */
//ssize_t bencode_write_buffer(bencode_obj_t o, uint8_t ** buf);

/**
   @brief read bencoded object from file descriptor
   @param o pointer to result
   @param fd file descriptor
   @param number of bytes read or -1 on error
 */
ssize_t bencode_read_fd(bencode_obj_t * o, int fd);

/**
   @brief read bencoded object from file
   @param o pointer to result
   @param f open file
   @return number of bytes read or -1 on error
 */
ssize_t bencode_read_file(bencode_obj_t * o, FILE * f);

/**
   @brief read bencoded object from buffer
   @param o pointer to result
   @param buf start of buffer
   @param sz size of buffer
   @return pointer in buffer were we stopped reading or NULL on underflow 
 */
//uint8_t * bencode_read_buffer(bencode_obj_t * o, uint8_t * buf, size_t sz);

#endif /* MNET_BENCODE_H */
