#ifndef I2P_EDDSA_H_
#define I2P_EDDSA_H_
#include <stdint.h>
#include <unistd.h>

#define EDDSA_PRIVKEY_LENGTH 32
#define EDDSA_PUBKEY_LENGTH 32
#define EDDSA_SIG_LENGTH 64

typedef uint8_t eddsa_privkey[EDDSA_PRIVKEY_LENGTH];
typedef uint8_t eddsa_pubkey[EDDSA_PUBKEY_LENGTH];
typedef uint8_t eddsa_sig[EDDSA_SIG_LENGTH];


void eddsa_keygen(eddsa_privkey * priv, eddsa_pubkey * pub);

struct eddsa_Verify;

void eddsa_Verify_new(struct eddsa_Verify ** v, eddsa_pubkey * pub);
void eddsa_Verify_free(struct eddsa_Verify ** v);

/** @brief get pointer to internal public key data */
void eddsa_Verify_get_key(struct eddsa_Verify * v, uint8_t ** k);

int eddsa_verify_signature(struct eddsa_Verify * v, const uint8_t * data, const size_t len, eddsa_sig * sig);

struct eddsa_Sign;

void eddsa_Sign_new(struct eddsa_Sign ** s, eddsa_privkey * priv);
void eddsa_Sign_free(struct eddsa_Sign ** s);

void eddsa_sign_data(struct eddsa_Sign *s, const uint8_t * data, const size_t len, eddsa_sig * sig);

/** @brief copy private key inside this signer into buffer */
void eddsa_copy_key_data(struct eddsa_Sign * s, eddsa_privkey * k);

#endif
