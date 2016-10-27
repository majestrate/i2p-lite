#include <i2pd/datatypes.h>
#include <i2pd/cert.h>
#include <i2pd/identity.h>
#include <i2pd/dsa.h>
#include <i2pd/elg.h>
#include <i2pd/log.h>
#include <i2pd/memory.h>
#include <string.h>
#include <openssl/sha.h>

struct i2p_identity
{
  elg_key enckey;
  dsa_pubkey sigkey;
  struct i2p_cert * cert;

  struct dsa_Verify * dsa;
  
};

struct i2p_cert
{
  uint8_t * data;
  size_t len;
  uint8_t type;
};


uint8_t * i2p_identity_read(struct i2p_identity ** i, uint8_t * in, size_t len)
{
  if(len < (3 + sizeof(elg_key) + sizeof(dsa_pubkey))) {
    // too small
    *i = NULL;
    return NULL;
  }
  (*i) = mallocx(sizeof(struct i2p_identity), MALLOCX_ZERO);
  memcpy((*i)->enckey, in, sizeof(elg_key));
  in += sizeof(elg_key);
  memcpy((*i)->sigkey, in, sizeof(dsa_pubkey));

  dsa_Verify_new(&(*i)->dsa, &(*i)->sigkey);

  
  in += sizeof(dsa_pubkey);
  return i2p_cert_read(&(*i)->cert, in, len - (sizeof(elg_key) + sizeof(dsa_pubkey)));
}

void i2p_identity_free(struct i2p_identity ** i)
{
  dsa_Verify_free(&(*i)->dsa);
  i2p_cert_free(&(*i)->cert);
  free(*i);
}

size_t i2p_identity_size(struct i2p_identity * i)
{
  return sizeof(elg_key) + sizeof(dsa_pubkey) + i->cert->len + 3;
}

size_t i2p_identity_siglen(struct i2p_identity * i)
{
  if (i->cert->type == I2P_CERT_TYPE_KEY)
    return 0;
  else
    return DSA_SIG_LENGTH;
}


int i2p_identity_verify_data(struct i2p_identity * i, uint8_t * data, size_t inlen, uint8_t * sig)
{
  dsa_signature s = {0};
  int valid = 0;
  uint16_t t = i2p_identity_sigtype(i);
  switch(t) {
  case SIGNING_KEY_TYPE_DSA_SHA1:
    memcpy(s, sig, sizeof(s));
    i2p_debug(LOG_DATA, "check DSA signature for identity, size %lu", inlen);
    valid = dsa_verify_signature(i->dsa, data, inlen, &s);
    break;
  default:
    i2p_error(LOG_DATA, "invalid identity signature type: %du", t);
  }
  return valid;
}

uint16_t i2p_identity_sigtype(struct i2p_identity * i)
{
  if(i->cert->type == I2P_CERT_TYPE_KEY)
    return bufbe16toh(i->cert->data + 3);
  else
    return SIGNING_KEY_TYPE_DSA_SHA1;
}

uint8_t * i2p_cert_read(struct i2p_cert ** c, uint8_t * d, size_t len)
{
  if(len < 3) {
    // underflow
    *c = NULL;
    return NULL;
  }
  (*c) = mallocx(sizeof(struct i2p_cert), MALLOCX_ZERO);
  (*c)->type = *d;
  (*c)->len = bufbe16toh(d+1);
  
  if(!(*c)->len) // length is zero
    return d + 3;
  if((*c)->len > len) { // overflow
    free(*c);
    *c = NULL;
    i2p_error(LOG_DATA, "i2p certificate overflow: %lu > %lu", (*c)->len, len);
    return NULL;
  }
  // fits just fine
  (*c)->data = mallocx((*c)->len, MALLOCX_ZERO);
  memcpy((*c)->data, d, 3);
  memcpy((*c)->data + 3, d + 3, (*c)->len);
  return d + (*c)->len + 3;
}

void i2p_cert_free(struct i2p_cert ** c)
{
  free((*c)->data);
  free(*c);
  *c = NULL;
}

size_t i2p_cert_data(struct i2p_cert * c, uint8_t ** ptr)
{
  *ptr = c->data + 3;
  return c->len;
}

int i2p_cert_type(struct i2p_cert * c)
{
  return c->type;
}

void i2p_identity_hash(struct i2p_identity * i, ident_hash * h)
{
  SHA256_CTX c;
  SHA256_Init(&c);
  SHA256_Update(&c, i->enckey, sizeof(elg_key));
  SHA256_Update(&c, i->sigkey, sizeof(dsa_pubkey));
  SHA256_Update(&c, i->cert->data, i->cert->len);
  SHA256_Final(*h, &c);
}
