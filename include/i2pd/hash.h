#ifndef I2PD_HASH_H_
#define I2PD_HASH_H_
#include <i2pd/datatypes.h>

void i2p_hash(ident_hash * h, const uint8_t * data, size_t sz);

struct i2p_hasher;

void i2p_hasher_new(struct i2p_hasher ** h);
void i2p_hasher_free(struct i2p_hasher ** h);

void i2p_hasher_update(struct i2p_hasher * h, const uint8_t * data, size_t len);
void i2p_hasher_final(struct i2p_hasher * h, ident_hash * digest);

#endif
