#ifndef I2PD_ELG_H_
#define I2PD_ELG_H_

#include <stdint.h>

/** elgamal key */
#define ELG_KEY_SIZE 256
typedef uint8_t elg_key[ELG_KEY_SIZE];

/** derived shared secret */
#define DH_KEY_SIZE 32
typedef uint8_t dh_shared_key[DH_KEY_SIZE];

/** block for elg encryption */
#define ELG_BLOCK_SIZE 514
typedef uint8_t elg_block[ELG_BLOCK_SIZE];
#define ELG_DATA_SIZE 222

/** @brief clear a elg block */
void elg_block_wipe(elg_block * block);

/** for dh key exchange */
struct elg_DH;

/** @brief free previously allocated dh */
void elg_DH_free(struct elg_DH ** k);
/** @brief allocate dhkeys */
int elg_DH_alloc(struct elg_DH ** k);

/** @brief generate dh keys */
void elg_DH_generate(struct elg_DH * k, elg_key * priv, elg_key * pub);

/** @brief agree on a shared key */
void elg_DH_agree(struct elg_DH * k, elg_key pub, dh_shared_key * shared);

/** @brief generate new elgamal keypair */
void elg_keygen(elg_key * priv, elg_key * pub);

/** @brief elg encryption context */
struct elg_Encryption;

/** @brief free previously allocated elg encryption context */
void elg_Encryption_free(struct elg_Encryption ** e);
/** @brief allocate new elg encryption context, copies key data */
void elg_Encryption_new(struct elg_Encryption ** e, elg_key pub);

/** @brief set a pointer to internal public key data */
void elg_Encryption_get_key(struct elg_Encryption * e, uint8_t ** data);

/** @brief do elg encryption on a block */ 
void elg_Encrypt(struct elg_Encryption * e, elg_block * block, int zeroPad);

/** @brief do elg decryption on a block */
int elg_Decrypt(elg_key priv, elg_block * block, int zeroPad);

#endif

