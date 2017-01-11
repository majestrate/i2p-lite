#ifndef BENCODE_H
#define BENCODE_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

enum {
    /* init state */
    BENCODE_TOK_NONE,
    /* a list */
    BENCODE_TOK_LIST,
    /* the length of a dictionary key */
    BENCODE_TOK_DICT_KEYLEN,
    /* a dictionary key */
    BENCODE_TOK_DICT_KEY,
    /* a dictionary value */
    BENCODE_TOK_DICT_VAL,
    /* an integer */
    BENCODE_TOK_INT,
    /* the length of the string */
    BENCODE_TOK_STR_LEN,
    /* string */
    BENCODE_TOK_STR,
    BENCODE_TOK_DICT
}; 

typedef struct bencode_s bencode_t;

typedef struct {

    /**
     * @param dict_key The dictionary key for this item.
     *        This is set to null for list entries
     * @param val The integer value
     * @return 0 on error; otherwise 1
     */
    int (*hit_int)(bencode_t *s,
            const char *dict_key,
            const long int val);

    /**
     * Call when there is some string for us to read.
     * This callback could fire multiple times for large strings
     *
     * @param dict_key The dictionary key for this item.
     *        This is set to null for list entries
     * @param val The string value
     * @param v_total_len The total length of the string
     * @param v_len The length of the string we're currently emitting
     * @return 0 on error; otherwise 1
     */
    int (*hit_str)(bencode_t *s,
        const char *dict_key,
        unsigned int v_total_len,
        const unsigned char* val,
        unsigned int v_len);

    /**
     * @param dict_key The dictionary key for this item.
     *        This is set to null for list entries
     * @param val The integer value
     * @return 0 on error; otherwise 1
     */
    int (*dict_enter)(bencode_t *s,
            const char *dict_key);
    /**
     * Called when we have finished processing a dictionary
     * @param dict_key The dictionary key for this item.
     *        This is set to null for list entries
     * @param val The integer value
     * @return 0 on error; otherwise 1
     */
    int (*dict_leave)(bencode_t *s,
            const char *dict_key);
    /**
     * @param dict_key The dictionary key for this item.
     *        This is set to null for list entries
     * @param val The integer value
     * @return 0 on error; otherwise 1
     */
    int (*list_enter)(bencode_t *s,
            const char *dict_key);
    /**
     * @param dict_key The dictionary key for this item.
     *        This is set to null for list entries
     * @param val The integer value
     * @return 0 on error; otherwise 1
     */
    int (*list_leave)(bencode_t *s,
            const char *dict_key);
    /**
     * Called when we have just finished processing a list item
     * @return 0 on error; otherwise 1
     */
    int (*list_next)(bencode_t *s);

    /**
     * Called when we have just finished processing a dict item
     * @return 0 on error; otherwise 1
     */
    int (*dict_next)(bencode_t *s);

} bencode_callbacks_t;

typedef struct {

    /* dict key */
    char* key;
    int k_size;

    char* strval;
    int sv_size;
    /* length of key buffer */
//    int keylen;

    long int intval;

    int len;

    int pos;

    /* token type */
    int type;

    /* user data for context specific to frame */
    void* udata;

} bencode_frame_t;

struct bencode_s {
    /* stack */
    bencode_frame_t* stk;

    /* number of frames we can push down, ie. maximum depth */
    unsigned int nframes;

    /* current depth within stack */
    unsigned int d;

    /* user data for context */
    void* udata;

    bencode_callbacks_t cb;
};


/**
 * @param expected_depth The expected depth of the bencode
 * @param cb The callbacks we need to parse the bencode
 * @return new memory for a bencode sax parser
 */
bencode_t* bencode_new(
        int expected_depth,
        bencode_callbacks_t* cb,
        void* udata);

/**
 * Initialise reader
 */
void bencode_init(bencode_t*);

/**
 * @param buf The buffer to read new input from
 * @param len The size of the buffer
 * @return 0 on error; otherwise 1
 */
int bencode_dispatch_from_buffer(
        bencode_t*,
        const char* buf,
        unsigned int len);
/**
 * @param cb The callbacks we need to parse the bencode
 */
void bencode_set_callbacks(
        bencode_t*,
        bencode_callbacks_t* cb);


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
ssize_t bencode_write_buffer(bencode_obj_t o, uint8_t ** buf);

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
uint8_t * bencode_read_buffer(bencode_obj_t * o, uint8_t * buf, size_t sz);

#endif /* BENCODE_H */
