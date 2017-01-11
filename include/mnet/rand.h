#ifndef MNET_RAND_H_
#define MNET_RAND_H_

#include <sodium/randombytes.h>

#define mnet_rand randombytes_buf
#define mnet_random randombytes_random

#endif
