#include <mnet/eddsa.h>
#include <mnet/memory.h>
#include <mnet/rand.h>
#include <mnet/hash.h>
#include <assert.h>

struct eddsa_Verify {
  eddsa_pubkey k;
};

struct eddsa_Sign {
  eddsa_privkey sk;
  eddsa_pubkey pk;
};


void eddsa_Verify_new(struct eddsa_Verify ** v, eddsa_pubkey * pub)
{
  *v = mallocx(sizeof(struct eddsa_Verify), MALLOCX_ZERO);
  memcpy((*v)->k, *pub, sizeof(eddsa_pubkey));
}

void eddsa_Verify_free(struct eddsa_Verify ** v)
{
  free(*v);
  *v = NULL;
}

void eddsa_Sign_new(struct eddsa_Sign ** s, eddsa_privkey * priv)
{
  *s = mallocx(sizeof(struct eddsa_Sign), MALLOCX_ZERO);
  memcpy((*s)->sk, *priv, sizeof(eddsa_privkey));
  //ed25519_ref10_pubkey((*s)->pk, (*s)->sk);
}

void eddsa_Sign_free(struct eddsa_Sign ** s)
{
  free(*s);
  *s = NULL;
}

void eddsa_keygen(eddsa_privkey * priv, eddsa_pubkey * pub)
{
  ident_hash h;
  mnet_rand(h, 32);
  mnet_hash(&h, *priv, 32);
  //ed25519_ref10_pubkey(*pub, *priv);
}

void eddsa_sign_data(struct eddsa_Sign * s, const uint8_t * data, const size_t len, eddsa_sig * sig)
{
  //ed25519_ref10_sign(*sig, data, len, s->sk, s->pk);
}

void eddsa_Sign_copy_key_data(struct eddsa_Sign * s, eddsa_privkey * k)
{
  memcpy(*k, s->sk, sizeof(eddsa_privkey));
}

int eddsa_verify_signature(struct eddsa_Verify * v, const uint8_t * data, const size_t len, eddsa_sig * sig)
{
  return 0;
  //return ed25519_ref10_open(*sig, data, len, v->k) == 0;
}

void eddsa_Verify_get_key(struct eddsa_Verify * v, uint8_t ** k)
{
  *k = v->k;
}

