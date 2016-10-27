#include <i2pd/datatypes.h>
#include <i2pd/dsa.h>
#include <i2pd/elg.h>
#include <i2pd/memory.h>
#include <i2pd/log.h>
#include <string.h>

struct i2p_identity
{
  elg_key enckey;
  dsa_pubkey sigkey;
  struct i2p_cert * cert;
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
  in += sizeof(dsa_pubkey);
  return i2p_cert_read(&(*i)->cert, in, len - (sizeof(elg_key) + sizeof(dsa_pubkey)));
}

struct i2p_cert
{
  uint8_t * data;
  size_t len;
  uint8_t type;
};

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
  memcpy((*c)->data, d + 3, (*c)->len);
  return d + (*c)->len + 3;
}

  
