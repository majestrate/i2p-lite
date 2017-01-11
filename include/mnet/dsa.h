#ifndef I2P_DSA_H_
#define I2P_DSA_H_
#include <stdint.h>
#include <unistd.h>

/** @brief dsa signing context */
struct dsa_Sign;
/** @brief dsa verify context */
struct dsa_Verify;

#define DSA_PUBKEY_LENGTH 128
#define DSA_SIG_LENGTH 40
#define DSA_PRIVKEY_LENGTH (DSA_SIG_LENGTH/2)

typedef uint8_t dsa_pubkey[DSA_PUBKEY_LENGTH];
typedef uint8_t dsa_privkey[DSA_PRIVKEY_LENGTH];
typedef uint8_t dsa_signature[DSA_SIG_LENGTH];

void dsa_keygen(dsa_privkey * priv, dsa_pubkey * pub);

void dsa_Verify_new(struct dsa_Verify ** d, dsa_pubkey * pubkey);
void dsa_Verify_free(struct dsa_Verify ** d);

/** @brief get pointer to internal public key */
void dsa_Verify_get_key(struct dsa_Verify * d, uint8_t ** k);

/** @brief verify dsa signature */
int dsa_verify_signature(struct dsa_Verify * d, const uint8_t * data, const size_t len, dsa_signature * sig);

void dsa_Sign_new(struct dsa_Sign **d, dsa_privkey * priv);
void dsa_Sign_free(struct dsa_Sign **d);

/** @brief sign data with dsa signer */
void dsa_sign_data(struct dsa_Sign * d, const uint8_t * data, const size_t len, dsa_signature * sig);

/** @brief copy private key data */
void dsa_Sign_copy_key_data(struct dsa_Sign * d, dsa_privkey * priv);

#endif
