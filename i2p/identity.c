#include <i2pd/datatypes.h>
#include <i2pd/cert.h>
#include <i2pd/identity.h>
#include <i2pd/dsa.h>
#include <i2pd/eddsa.h>
#include <i2pd/elg.h>
#include <i2pd/log.h>
#include <i2pd/memory.h>
#include <string.h>
#include <openssl/sha.h>

struct i2p_identity
{
  struct i2p_cert * cert;
  struct dsa_Verify * dsa;
  struct eddsa_Verify * eddsa;
  struct elg_Encryption * elg;
  ident_hash ident;
};

void i2p_identity_new(struct i2p_identity ** i)
{
  (*i) = xmalloc(sizeof(struct i2p_identity));
}

int i2p_identity_read(struct i2p_identity * i, int fd)
{
  elg_key elg_pub = {0};
  dsa_pubkey dsa_pub = {0};
  eddsa_pubkey ed_pub = {0};
  if(read(fd, elg_pub, sizeof(elg_key)) != sizeof(elg_key)) {
    // bad read
    i2p_error(LOG_DATA, "i2p identity read elg key failed");
    return 0;
  }
  if(read(fd, dsa_pub, sizeof(dsa_pubkey)) != sizeof(dsa_pubkey)) {
    // bad read
    i2p_error(LOG_DATA, "i2p identity read dsa key failed");
    return 0;
  }
  if(i->cert) i2p_cert_free(&i->cert); // free existing if it's there
  i2p_cert_new(&i->cert);
  int ret = i2p_cert_read(i->cert, fd);
  if(ret) {
    elg_Encryption_new(&i->elg, &elg_pub);
    uint16_t sigtype = i2p_identity_sigtype(i);
    if(sigtype == SIGNING_KEY_TYPE_EDDSA_SHA512_ED25519) {
      memcpy(ed_pub, dsa_pub+96, sizeof(eddsa_pubkey));
      eddsa_Verify_new(&i->eddsa, &ed_pub);
    } else if (sigtype == SIGNING_KEY_TYPE_DSA_SHA1) {
      dsa_Verify_new(&i->dsa, &dsa_pub);
    } else {
      i2p_error(LOG_DATA, "i2p identity has unsupported signing key type: %d", sigtype);
    }
  }
  return ret;
}

uint8_t * i2p_identity_read_buffer(struct i2p_identity * i, uint8_t * in, size_t len)
{
  uint8_t * begin = in;
  elg_key elg_pub = {0};
  dsa_pubkey dsa_pub = {0};
  eddsa_pubkey ed_pub = {0};
  if(len < (3 + sizeof(elg_key) + sizeof(dsa_pubkey))) {
    // too small
    i2p_error(LOG_DATA, "i2p identity buffer too small: %d", len);
    return NULL;
  }
  memcpy(elg_pub, in, sizeof(elg_key));
  in += sizeof(elg_key);
  memcpy(dsa_pub, in, sizeof(dsa_pubkey));
  in += sizeof(dsa_pubkey);
  i2p_cert_new(&i->cert);
  in = i2p_cert_read_buffer(i->cert, in, len - (sizeof(elg_key) + sizeof(dsa_pubkey)));
  uint16_t sigtype = i2p_identity_sigtype(i);
  if(sigtype == SIGNING_KEY_TYPE_EDDSA_SHA512_ED25519) {
    memcpy(ed_pub, dsa_pub+96, sizeof(eddsa_pubkey));
    eddsa_Verify_new(&i->eddsa, &ed_pub);
  } else if (sigtype == SIGNING_KEY_TYPE_DSA_SHA1) {
    dsa_Verify_new(&i->dsa, &dsa_pub);
  }
  elg_Encryption_new(&i->elg, &elg_pub);
  // calculate ident hash
  SHA256(begin, in - begin, i->ident);
  return in;
}

int i2p_identity_write(struct i2p_identity * i, int fd)
{
  uint8_t * elg = NULL;
  elg_Encryption_get_key(i->elg, &elg);
  if(write(fd, elg, sizeof(elg_key)) == -1) return 0;
  
  uint8_t * dsa = NULL;
  dsa_Verify_get_key(i->dsa, &dsa);
  if(write(fd, dsa, sizeof(dsa_pubkey)) == -1) return 0;

  return i2p_cert_write(i->cert, fd);
}

void i2p_identity_free(struct i2p_identity ** i)
{
  i2p_cert_free(&(*i)->cert);
  if((*i)->elg) elg_Encryption_free(&(*i)->elg);
  if((*i)->dsa) dsa_Verify_free(&(*i)->dsa);
  if((*i)->eddsa) eddsa_Verify_free(&(*i)->eddsa);
  free(*i);
  *i = NULL;
}

size_t i2p_identity_size(struct i2p_identity * i)
{
  return sizeof(elg_key) + sizeof(dsa_pubkey) + i2p_cert_buffer_length(i->cert);
}

size_t i2p_identity_siglen(struct i2p_identity * i)
{
  switch(i2p_identity_sigtype(i)) {
  case SIGNING_KEY_TYPE_EDDSA_SHA512_ED25519:
    return sizeof(eddsa_sig);
  case SIGNING_KEY_TYPE_DSA_SHA1:
    return sizeof(dsa_signature);
  default:
    return 0;
  }
}

int i2p_identity_verify_data(struct i2p_identity * i, uint8_t * data, size_t inlen, uint8_t * sig)
{
  dsa_signature d_sig = {0};
  eddsa_sig ed_sig = {0};
  int valid = 0;
  uint16_t t = i2p_identity_sigtype(i);
  switch(t) {
  case SIGNING_KEY_TYPE_DSA_SHA1:
    memcpy(d_sig, sig, sizeof(d_sig));
    i2p_debug(LOG_DATA, "check DSA signature for identity, size %lu", inlen);
    valid = dsa_verify_signature(i->dsa, data, inlen, &d_sig);
    break;
  case SIGNING_KEY_TYPE_EDDSA_SHA512_ED25519:
    i2p_debug(LOG_DATA, "check EdDSA signature for identity, size %lu", inlen);
    memcpy(ed_sig, sig, sizeof(eddsa_sig));
    valid = eddsa_verify_signature(i->eddsa, data, inlen, &ed_sig);
    break;
  default:
    i2p_error(LOG_DATA, "invalid identity signature type: %du", t);
  }
  return valid;
}

uint16_t i2p_identity_sigtype(struct i2p_identity * i)
{
  if(i2p_cert_type(i->cert) == I2P_CERT_TYPE_KEY)
    return bufbe16toh(i2p_cert_data(i->cert));
  else
    return SIGNING_KEY_TYPE_DSA_SHA1;
}

void i2p_identity_hash(struct i2p_identity * i, ident_hash * h)
{
  memcpy(*h, i->ident, sizeof(ident_hash));
}

struct i2p_identity_keys
{
  struct i2p_identity * ident;
  elg_key elg_priv;
  struct dsa_Sign * dsa;
  struct eddsa_Sign * eddsa;
};

void i2p_identity_keys_new(struct i2p_identity_keys ** k)
{
  *k = xmalloc(sizeof(struct i2p_identity_keys)); 
}

void i2p_identity_keys_free(struct i2p_identity_keys ** k)
{
  if((*k)->dsa) dsa_Sign_free(&(*k)->dsa);
  if((*k)->eddsa) eddsa_Sign_free(&(*k)->eddsa);
  if((*k)->ident) i2p_identity_free(&(*k)->ident);
  free(*k);
  *k = NULL;
}

int i2p_identity_keys_generate(struct i2p_identity_keys * k, uint16_t sigtype)
{
  if(sigtype != SIGNING_KEY_TYPE_EDDSA_SHA512_ED25519) return 0; // not supported
  elg_key elg_pub = {0};
  eddsa_pubkey ed_pub = {0};
  eddsa_privkey ed_priv = {0};
  elg_keygen(&k->elg_priv, &elg_pub);
  eddsa_keygen(&ed_priv, &ed_pub);
  // free existing public ident
  if(k->ident) i2p_identity_free(&k->ident);
  i2p_identity_new(&k->ident);
  // create inner members
  elg_Encryption_new(&k->ident->elg, &elg_pub);
  eddsa_Verify_new(&k->ident->eddsa, &ed_pub);
  // init certificate data
  uint8_t cert[4] = {0};
  htobe16buf(cert, sigtype);
  i2p_cert_new(&k->ident->cert);
  i2p_cert_init(k->ident->cert, I2P_CERT_TYPE_KEY, cert, 4);
  return 1;
}

int i2p_identity_keys_read(struct i2p_identity_keys * k, int fd)
{
  eddsa_privkey ed_priv = {0};
  dsa_privkey dsa_priv = {0};
  
  // read public identity
  i2p_identity_new(&k->ident);
  if(!i2p_identity_read(k->ident, fd)) {
    i2p_identity_free(&k->ident);
    // bad read
    return 0;
  }
  // read private encryption key
  if(read(fd, k->elg_priv, sizeof(elg_key)) != sizeof(elg_key)) {
    i2p_identity_free(&k->ident);
    // bad read
    return 0;
  }
  uint16_t sigtype = i2p_identity_sigtype(k->ident);
  if(sigtype == SIGNING_KEY_TYPE_EDDSA_SHA512_ED25519) {
    if(read(fd, ed_priv, sizeof(eddsa_privkey)) != sizeof(eddsa_privkey)) {
      i2p_identity_free(&k->ident);
      // bad read
      return 0;
    }
    eddsa_Sign_new(&k->eddsa, &ed_priv);
  } else if(sigtype == SIGNING_KEY_TYPE_DSA_SHA1) {
    if(read(fd, dsa_priv, sizeof(dsa_privkey)) != sizeof(dsa_privkey)) {
      i2p_identity_free(&k->ident);
      // bad read
      return 0;
    }
    dsa_Sign_new(&k->dsa, &dsa_priv);
  } else {
    // unknown signing key type
    i2p_error(LOG_DATA, "i2p router keys unsuportted signing key type: %d", sigtype);
    i2p_identity_free(&k->ident);
    return 0;
  }
  return 1;
}

int i2p_identity_keys_write(struct i2p_identity_keys * k, int fd)
{

  uint16_t sigtype = i2p_identity_sigtype(k->ident);
  
  if(sigtype == SIGNING_KEY_TYPE_DSA_SHA1) {
    
    if(!i2p_identity_write(k->ident, fd)) return 0;
    
    if(write(fd, k->elg_priv, sizeof(elg_key)) == -1) return 0;
    
    dsa_privkey priv;
    dsa_Sign_copy_key_data(k->dsa, &priv);
    if(write(fd, priv, sizeof(dsa_privkey)) == -1) return 0;
    
  } else if (sigtype == SIGNING_KEY_TYPE_EDDSA_SHA512_ED25519) {
    
    if(!i2p_identity_write(k->ident, fd)) return 0;
    
    if(write(fd, k->elg_priv, sizeof(elg_key)) == -1) return 0;
    
    eddsa_privkey edpriv;
    eddsa_Sign_copy_key_data(k->eddsa, &edpriv);
    if(write(fd, edpriv, sizeof(eddsa_privkey)) == -1) return 0;
  } else {
    i2p_error(LOG_DATA, "invalid sigtype for identity keys %d", sigtype);
    return 0;
  }
  return 1;
}

void i2p_identity_keys_to_public(struct i2p_identity_keys * k, struct i2p_identity ** pub)
{
  *pub = k->ident;
}

void i2p_identity_get_elg_key(struct i2p_identity * i, elg_key * k)
{
  uint8_t * ptr = NULL;
  elg_Encryption_get_key(i->elg, &ptr);
  if(ptr) {
    memcpy(*k, ptr, sizeof(elg_key));
  }
}
