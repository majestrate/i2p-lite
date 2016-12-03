#ifndef I2PD_DATATYPES_H_
#define I2PD_DATATYPES_H_
#include <stdint.h>
#include <unistd.h>

/** identity hash, sha256 */
#define IDENT_HASH_SIZE 32
typedef uint8_t ident_hash[IDENT_HASH_SIZE];

/** an i2np tunnel data message */
typedef uint8_t tunnel_data_message[1028];

#endif

