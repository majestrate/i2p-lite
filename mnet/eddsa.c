#include <mnet/eddsa.h>
#include <mnet/memory.h>
#include <mnet/rand.h>
#include <mnet/hash.h>
#include <mnet/log.h>
#include <assert.h>

typedef uint8_t eddsa_crypto_sign_privkey[crypto_sign_SECRETKEYBYTES];

struct eddsa_Verify {
  eddsa_pubkey k;
};

struct eddsa_Sign {
  eddsa_crypto_sign_privkey k;
};


void eddsa_Verify_new(struct eddsa_Verify ** v, eddsa_pubkey * pub)
{
  *v = xmalloc(sizeof(struct eddsa_Verify));
  memcpy((*v)->k, *pub, sizeof(eddsa_pubkey));
}

void eddsa_Verify_free(struct eddsa_Verify ** v)
{
  free(*v);
  *v = NULL;
}

void eddsa_Sign_new(struct eddsa_Sign ** s, eddsa_privkey * priv)
{
  *s = xmalloc(sizeof(struct eddsa_Sign));
  eddsa_pubkey pk;
  crypto_sign_seed_keypair(pk, (*s)->k, *priv);
}

void eddsa_Sign_free(struct eddsa_Sign ** s)
{
  free(*s);
  *s = NULL;
}

void eddsa_keygen(eddsa_privkey * priv, eddsa_pubkey * pub)
{
  ident_hash h;
  ident_hash k;
  mnet_rand(k, sizeof(ident_hash));
  mnet_hash(&h, k, sizeof(eddsa_privkey));
  eddsa_crypto_sign_privkey sk = {0};
  assert(crypto_sign_seed_keypair(*pub, sk, h) != -1);
  memcpy(*priv, h, sizeof(eddsa_privkey));
}

void eddsa_sign_data(struct eddsa_Sign * s, const uint8_t * data, const size_t len, eddsa_sig * sig)
{
  unsigned long long l;
  assert(crypto_sign_detached(*sig, &l, data, len, s->k) != -1);
}

void eddsa_Sign_copy_key_data(struct eddsa_Sign * s, eddsa_privkey * k)
{
  uint8_t * d = *k;
  // TODO: this assumes that the secret key is stored first
  memcpy(d, s->k, sizeof(eddsa_privkey));
}

int eddsa_verify_signature(struct eddsa_Verify * v, const uint8_t * data, const size_t len, eddsa_sig * sig)
{
  return crypto_sign_verify_detached(*sig, data, len, v->k) != -1;
}

void eddsa_Verify_get_key(struct eddsa_Verify * v, uint8_t ** k)
{
  *k = v->k;
}

