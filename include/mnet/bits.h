#ifndef MNET_BITS_H_
#define MNET_BITS_H_
#include <stdint.h>
#if UINTPTR_MAX == 0xffffffff
#define __32_BIT
#elif UINTPTR_MAX == 0xffffffffffffffff
#define __64_BIT
#else
#error "platform not supported"
#endif
#endif
