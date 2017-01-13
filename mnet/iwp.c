#include <mnet/iwp.h>
#include <mnet/memory.h>
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


void iwp_config_new(struct iwp_config ** cfg)
{
  *cfg = xmalloc(sizeof(struct iwp_config));
}

void iwp_config_free(struct iwp_config ** cfg)
{
  free((*cfg)->pubkey);
  free((*cfg)->addr);
  free(*cfg);
  *cfg =  NULL;
}

static void iwp_config_iter_dict(bencode_obj_t d, const char * key, bencode_obj_t val, void * user)
{
  struct iwp_config * cfg = (struct iwp_config *) user;
  if(!strcmp(key, "pubkey")) {
    uint8_t * v =  NULL;
    ssize_t sz = bencode_obj_getstr(val, &v);
    if (v) {
      v[sz] = 0;
      cfg->pubkey = strdup(v);
    }
  } else if(!strcmp(key, "addr")) {
    uint8_t * a =  NULL;
    ssize_t sz = bencode_obj_getstr(val, &a);
    if (a) {
      a[sz] = 0;
      cfg->addr = strdup(a);
    }
  }
}

int iwp_config_load_dict(struct iwp_config * cfg, bencode_obj_t d)
{
  bencode_obj_iter_dict(d, iwp_config_iter_dict, cfg);
  return cfg->addr && cfg->pubkey;
}
