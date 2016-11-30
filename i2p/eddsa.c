#include <i2pd/eddsa.h>
#include <ed25519_ref10.h>
#include <i2pd/memory.h>
#include <assert.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

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
  ed25519_ref10_pubkey((*s)->pk, *priv);
}

void eddsa_Sign_free(struct eddsa_Sign ** s)
{
  free(*s);
  *s = NULL;
}

void eddsa_keygen(eddsa_privkey * priv, eddsa_pubkey * pub)
{
  uint8_t k[32];
  RAND_bytes(k, 32);
  SHA256(*priv, 32, k);
  ed25519_ref10_pubkey(*pub, *priv);
}

void eddsa_sign_data(struct eddsa_Sign * s, const uint8_t * data, const size_t len, eddsa_sig * sig)
{
  ed25519_ref10_sign(*sig, data, len, s->sk, s->pk);
}

int eddsa_verify_signature(struct eddsa_Verify * v, const uint8_t * data, const size_t len, eddsa_sig * sig)
{
  return ed25519_ref10_open(*sig, data, len, v->k) == 0;
}
