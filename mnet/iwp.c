#include <i2pd/iwp.h>
#include <sodium/crypto_box_curve25519xsalsa20poly1305.h>
#include <sodium/crypto_scalarmult.h>

/**
   @brief session key derived from BLAKE2B(sh) where sh is a shared secret derived from scalarmult(our_long_term_pubkey, their_long_term_pubkey) and a 32 byte nonce
 */
typedef uint8_t iwp_session_key_t[IDENT_V2_HASH_SIZE];

typedef uint8_t iwp_transit_pubkey_t[crypto_scalarmult_BYTES];

typedef uint8_t iwp_transit_seckey_t[crypto_scalarmult_BYTES];

struct iwp_session
{
  iwp_transit_pubkey_t eph_pubkey;
  iwp_transit_seckey_t eph_seckey;
};
