#ifndef I2PD_IDENTITY_H_
#define I2PD_IDENTITY_H_
#include <stdint.h>
#include <unistd.h>

#define SIGNING_KEY_TYPE_DSA_SHA1 0
#define SIGNING_KEY_TYPE_ECDSA_SHA256_P256 1
#define SIGNING_KEY_TYPE_ECDSA_SHA384_P384 2
#define SIGNING_KEY_TYPE_ECDSA_SHA512_P521 3
#define SIGNING_KEY_TYPE_RSA_SHA256_2048 4
#define SIGNING_KEY_TYPE_RSA_SHA384_3072 5
#define SIGNING_KEY_TYPE_RSA_SHA512_4096 6
#define SIGNING_KEY_TYPE_EDDSA_SHA512_ED25519 7


struct i2p_identity;

/** @brief read i2p identity from buffer, returns NULL on under/overflow, otherwise returns pointer to where we stopped reading */
uint8_t * i2p_identity_read(struct i2p_identity ** i, uint8_t * in, size_t len);
void i2p_identity_free(struct i2p_identity ** i);

size_t i2p_identity_size(struct i2p_identity * i);
uint8_t * i2p_identity_data(struct i2p_identity * i);
size_t i2p_identity_siglen(struct i2p_identity * i);
int i2p_identity_verify_data(struct i2p_identity * i, uint8_t * in, size_t inlen, uint8_t *sig);
void i2p_identity_hash(struct i2p_identity * i, ident_hash * ident);
uint16_t i2p_identity_sigtype(struct i2p_identity * i);

#endif
