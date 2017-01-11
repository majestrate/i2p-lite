#ifndef I2PD_DATATYPES_H_
#define I2PD_DATATYPES_H_
#include <stdint.h>
#include <unistd.h>
#include <sodium/crypto_aead_chacha20poly1305.h>
#include <sodium/crypto_box_curve25519xsalsa20poly1305.h>
#include <sodium/crypto_generichash_blake2b.h>

/** version 2 identity hash, blake2b */
#define IDENT_V2_HASH_SIZE crypto_generichash_blake2b_BYTES
typedef uint8_t ident_hash[IDENT_V2_HASH_SIZE];

/**
   @brief tunnel id, 4 byte big endian integer
*/
typedef uint32_t tunnel_id_t;

#define TUNID_SIZE 4

#define TUNKEY_V2_SIZE crypto_aead_chacha20poly1305_ietf_KEYBYTES

/**
   @brief a symmetric key for encryption/decryption
 */
typedef uint8_t sym_key_t[TUNKEY_V2_SIZE];

/**
   @brief a symmettric key for tunnel encryption/decryption
 */
typedef sym_key_t tunnel_key_t;

#define TUNMAC_V2_SIZE 16
/**
   @brief a mac block for version 2 tunnel data messages
 */
typedef uint8_t tunnel_mac_t[TUNMAC_V2_SIZE];

#define TUNNONCE_V2_SIZE 12

typedef uint8_t tunnel_nonce_t[TUNNONCE_V2_SIZE];


// this size was chosen because it will make the entire message exactly 2048 bytes large
// also because that's THE CURRENT YEAR, get with the times DOOOOOOD.
#define TUNDATA_V2_SIZE 2016

#define TDM_V2_SIZE (TUNMAC_V2_SIZE + TUNNONCE_V2_SIZE + TUNID_SIZE + TUNDATA_V2_SIZE)

#define TUNDATA_V2_OFFSET (TDM_V2_SIZE - TUNDATA_V2_SIZE)
#define TUNMAC_V2_OFFSET (0)
#define TUNID_V2_OFFSET (TUNNONCE_V2_SIZE + TUNMAC_V2_SIZE)
#define TUNNONCE_V2_OFFSET (TUNMAC_V2_SIZE)

/**
   @brief a version 2 tunnel data message

   16 byte mac + 12 byte nonce + 4 byte tunnel id + 2016 bytes encrypted body

 */
typedef uint8_t tunnel_data_message_v2[TDM_V2_SIZE];

/**
   @brief plaintext block from a version 2 tunnel data message
*/
typedef uint8_t tunnel_data_block_v2[TUNDATA_V2_SIZE];

/**
   @brief a public encryption key
 */
typedef uint8_t pub_enc_key_t[crypto_box_curve25519xsalsa20poly1305_PUBLICKEYBYTES];

/**
   @brief a secret key for public key encryption
 */
typedef uint8_t sec_enc_key_t[crypto_box_curve25519xsalsa20poly1305_SEEDBYTES];

#endif

