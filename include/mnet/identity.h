#ifndef MNET_IDENTITY_H_
#define MNET_IDENTITY_H_
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#define SIGNING_KEY_TYPE_NACL 0

#define DEFAULT_IDENTITY_SIG_TYPE SIGNING_KEY_TYPE_NACL


struct mnet_identity;

void mnet_identity_new(struct mnet_identity ** i);

/** @brief read i2p identity from buffer, returns NULL on under/overflow, otherwise returns pointer to where we stopped reading */
uint8_t * mnet_identity_read_buffer(struct mnet_identity * i, uint8_t * in, size_t len);
/** @brief read i2p identity from file descriptor, returns 1 on success, otherwise returns 0 */
int mnet_identity_read(struct mnet_identity * i, FILE * f);

int mnet_identity_write(struct mnet_identity * i, FILE * f);

void mnet_identity_free(struct mnet_identity ** i);

size_t mnet_identity_size(struct mnet_identity * i);
uint8_t * mnet_identity_data(struct mnet_identity * i);
size_t mnet_identity_siglen(struct mnet_identity * i);
int mnet_identity_verify_data(struct mnet_identity * i, uint8_t * in, size_t inlen, uint8_t *sig);
void mnet_identity_hash(struct mnet_identity * i, ident_hash * ident);
uint16_t mnet_identity_sigtype(struct mnet_identity * i);
char * mnet_identity_to_base64(struct mnet_identity * i);
int mnet_identity_from_base64(struct mnet_identity * i, char * str);

void mnet_identity_clone(struct mnet_identity * i, struct mnet_identity ** clone);

struct mnet_identity_keys;

void mnet_identity_keys_new(struct mnet_identity_keys ** k);
void mnet_identity_keys_free(struct mnet_identity_keys ** k);

int mnet_identity_keys_generate(struct mnet_identity_keys * k, uint16_t sigtype);

int mnet_identity_keys_read(struct mnet_identity_keys * k, FILE * f);
int mnet_identity_keys_write(struct mnet_identity_keys * k, FILE * f);

size_t mnet_identity_keys_keylen(struct mnet_identity_keys * k);

void mnet_identity_sign_data(struct mnet_identity_keys * k, uint8_t * in, size_t inlen, uint8_t *sig);

void mnet_identity_keys_to_public(struct mnet_identity_keys * k, struct mnet_identity ** pub);


#endif
