#ifndef I2PD_DATATYPES_H_
#define I2PD_DATATYPES_H_
#include <stdint.h>
#include <unistd.h>

/** identity hash, sha256 */
#define IDENT_HASH_SIZE 32
typedef uint8_t ident_hash[IDENT_HASH_SIZE];

struct i2p_identity;

uint8_t * i2p_identity_read(struct i2p_identity ** i, uint8_t * in, size_t len);

size_t i2p_identity_size(struct i2p_identity * i);
uint8_t * i2p_identity_data(struct i2p_identity * i);
size_t i2p_identity_siglen(struct i2p_identity * i);
int i2p_identity_verify_data(struct i2p_identity * i, uint8_t * in, size_t inlen, uint8_t *sig);

#endif

