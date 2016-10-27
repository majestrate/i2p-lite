#include <i2pd/eddsa.h>
#include <i2pd/memory.h>
#include <assert.h>
#include <sodium.h>

struct eddsa_Verify {
  eddsa_pubkey k;
};

struct eddsa_Sign {
  uint8_t k[64];
};


void eddsa_Verify_new(struct eddsa_Verify ** v, eddsa_pubkey * pub)
{
  (*v) = mallocx(sizeof(struct eddsa_Verify), MALLOCX_ZERO);
  memcpy((*v)->k, *pub, sizeof(eddsa_pubkey));
}

void eddsa_Verify_free(struct eddsa_Verify ** v)
{
  free(*v);
  *v = NULL;
}

void eddsa_Sign_new(struct eddsa_Sign ** s, eddsa_privkey * priv)
{
  eddsa_pubkey k;
  (*s) = mallocx(sizeof(struct eddsa_Sign), MALLOCX_ZERO);
  crypto_sign_seed_keypair(k, (*s)->k, *priv);
}

void eddsa_Sign_free(struct eddsa_Sign ** s)
{
  free(*s);
  *s = NULL;
}

void eddsa_keygen(eddsa_privkey * priv, eddsa_pubkey * pub)
{
  uint8_t secret[64] = {0};
  crypto_sign_keypair(*pub, secret);
  crypto_sign_ed25519_sk_to_seed(*priv, secret);
}

void eddsa_sign_data(struct eddsa_Sign * s, const uint8_t * data, const size_t len, eddsa_sig * sig)
{
  assert(crypto_sign_detached(*sig, NULL, data, len, s->k) != -1);
}

int eddsa_verify_signature(struct eddsa_Verify * v, const uint8_t * data, const size_t len, eddsa_sig * sig)
{
  return crypto_sign_verify_detached(*sig, data, len, v->k) == 0;
}
