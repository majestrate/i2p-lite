#include <i2pd/datatypes.h>
#include <i2pd/cert.h>
#include <i2pd/identity.h>
#include <i2pd/eddsa.h>
#include <i2pd/hash.h>
#include <i2pd/log.h>
#include <i2pd/memory.h>
#include <i2pd/bencode.h>
#include <string.h>
#include <errno.h>


struct i2p_identity
{
  struct i2p_cert * cert;
  struct eddsa_Verify * eddsa;
  ident_hash ident;
};

void i2p_identity_new(struct i2p_identity ** i)
{
  (*i) = xmalloc(sizeof(struct i2p_identity));
}

int i2p_identity_read(struct i2p_identity * i, FILE * f)
{
  eddsa_pubkey ed_pub = {0};
  if(fread(ed_pub, sizeof(eddsa_pubkey), 1, f) != sizeof(eddsa_pubkey)) {
    // bad read
    i2p_error(LOG_DATA, "i2p identity read eddsa pub key failed");
    return 0;
  }

  eddsa_Verify_new(&i->eddsa, &ed_pub);

  if(i->cert) i2p_cert_free(&i->cert); // free existing if it's there
  i2p_cert_new(&i->cert);
  return i2p_cert_read(i->cert, f);
}

uint8_t * i2p_identity_read_buffer(struct i2p_identity * i, uint8_t * in, size_t len)
{
  uint8_t * begin = in;
  eddsa_pubkey ed_pub = {0};
  if(len < (3 + sizeof(eddsa_pubkey))) {
    // too small
    i2p_error(LOG_DATA, "i2p identity buffer too small: %d", len);
    return NULL;
  }
  memcpy(ed_pub, in, sizeof(eddsa_pubkey));
  in += sizeof(eddsa_pubkey);

  if(i->cert) i2p_cert_free(&i->cert); // free if already there

  i2p_cert_new(&i->cert);
  return i2p_cert_read_buffer(i->cert, in, len - sizeof(eddsa_pubkey));
}

int i2p_identity_write(struct i2p_identity * i, FILE * f)
{
  uint8_t * ed_pub = NULL;
  eddsa_Verify_get_key(i->eddsa, &ed_pub);
  if(fwrite(ed_pub, sizeof(eddsa_pubkey), 1, f) == -1) return 0;

  return i2p_cert_write(i->cert, f);
}

void i2p_identity_free(struct i2p_identity ** i)
{
  if(*i)
  {
    if((*i)->cert) i2p_cert_free(&(*i)->cert);
    if((*i)->eddsa) eddsa_Verify_free(&(*i)->eddsa);
    free(*i);
  }
  *i = NULL;
}

size_t i2p_identity_size(struct i2p_identity * i)
{
  return sizeof(eddsa_pubkey) + i2p_cert_buffer_length(i->cert);
}

size_t i2p_identity_siglen(struct i2p_identity * i)
{
  return sizeof(eddsa_sig);
}

int i2p_identity_verify_data(struct i2p_identity * i, uint8_t * data, size_t inlen, uint8_t * sig)
{
  eddsa_sig ed_sig = {0};
  ident_hash h = {0};
  memcpy(ed_sig, sig, sizeof(eddsa_sig));
  i2p_hash(&h, data, inlen);
  return eddsa_verify_signature(i->eddsa, h, sizeof(ident_hash), &ed_sig) != 0;
}

uint16_t i2p_identity_sigtype(struct i2p_identity * i)
{
  if(i2p_cert_type(i->cert) == I2P_CERT_TYPE_KEY)
    return bufbe16toh(i2p_cert_data(i->cert));
  else
    return SIGNING_KEY_TYPE_NACL;
}

void i2p_identity_hash(struct i2p_identity * i, ident_hash * h)
{
  struct i2p_hasher * hash = NULL;
  i2p_hasher_new(&hash);

  uint8_t * data = NULL;
  size_t sz = sizeof(eddsa_pubkey);
  eddsa_Verify_get_key(i->eddsa, &data);
  i2p_hasher_update(hash, data, sz);

  data = i2p_cert_buffer(i->cert);
  sz = i2p_cert_buffer_length(i->cert);

  i2p_hasher_update(hash, data, sz);

  i2p_hasher_final(hash, h);
  i2p_hasher_free(&hash);
}

struct i2p_identity_keys
{
  struct i2p_identity * ident;
  struct eddsa_Sign * eddsa;
};

void i2p_identity_keys_new(struct i2p_identity_keys ** k)
{
  *k = xmalloc(sizeof(struct i2p_identity_keys)); 
}

void i2p_identity_keys_free(struct i2p_identity_keys ** k)
{
  if((*k)->eddsa) eddsa_Sign_free(&(*k)->eddsa);
  free(*k);
  *k = NULL;
}

int i2p_identity_keys_generate(struct i2p_identity_keys * k, uint16_t sigtype)
{
  eddsa_pubkey ed_pub = {0};
  eddsa_privkey ed_priv = {0};
  eddsa_keygen(&ed_priv, &ed_pub);
  // free if signer already existing
  if(k->eddsa) eddsa_Sign_free(&k->eddsa);
  // init eddsa signer
  eddsa_Sign_new(&k->eddsa, &ed_priv);
  // free existing public ident
  if(k->ident) i2p_identity_free(&k->ident);
  i2p_identity_new(&k->ident);
  eddsa_Verify_new(&k->ident->eddsa, &ed_pub);

  // init certificate data
  uint8_t cert[3] = {0};
  htobe16buf(cert, sigtype);
  i2p_cert_new(&k->ident->cert);
  i2p_cert_init(k->ident->cert, 0, cert, 0);
  return 1;
}

// context for iterating over identity key dict
struct i2p_keys_iter_ctx
{
  struct i2p_identity_keys * k;
  int success;
};

static void i2p_identity_keys_iter_dict(bencode_obj_t p, const char * k, bencode_obj_t v, void * u)
{
  struct i2p_keys_iter_ctx * ctx = (struct i2p_keys_iter_ctx *) u;
  if(!ctx->k->eddsa) {
    // no key found yet
    if(!strcmp(k, "eddsa_privkey")) {
      // this is an eddsa private key, try getting it as a bytestring
      uint8_t * key = NULL;
      ctx->success = bencode_obj_getstr(v, &key) == sizeof(eddsa_privkey);
      if(ctx->success) {
        // this was a bytestring and the right size
        eddsa_privkey privkey = {0};
        memcpy(privkey, key, sizeof(eddsa_privkey));
        eddsa_Sign_new(&ctx->k->eddsa, &privkey);
      }
      free(key);
    }
  }
  // TODO: load cert
}

int i2p_identity_keys_read(struct i2p_identity_keys * k, FILE * f)
{
  eddsa_privkey ed_priv = {0};
  // free identity if already allocated
  if(k->ident) i2p_identity_free(&k->ident);
  // free signer if already allocated
  if(k->eddsa) eddsa_Sign_free(&k->eddsa);

  bencode_obj_t benc = NULL;
  if(bencode_read_file(&benc, f) == -1) {
    // bad read
    i2p_error(LOG_DATA, "bad read reading identity keys: %s", strerror(errno));
    return -1;
  }
  int res = 0;
  struct i2p_keys_iter_ctx ctx = {
    .k = k,
    .success = 0,
  };
  if(bencode_obj_is_dict(benc)) {
    // yey it's a dict
    bencode_obj_iter_dict(benc, i2p_identity_keys_iter_dict, &ctx);
    // set res to be if we succeeded or not
    res = ctx.success == 1;
  }
  bencode_obj_free(&benc);
  return res;
}

int i2p_identity_keys_write(struct i2p_identity_keys * k, FILE * f)
{
  bencode_obj_t benc = NULL;
  bencode_obj_t key = NULL;
  int res = 0;
  eddsa_privkey privkey = {0};

  if(!k->eddsa) return res; // no key data

  eddsa_Sign_copy_key_data(k->eddsa, &privkey);

  bencode_obj_dict(&benc);
  bencode_obj_str(&key, privkey, sizeof(eddsa_privkey));

  bencode_obj_dict_set(benc, "eddsa_privkey", key);
  if(k->ident->cert) {
    bencode_obj_t cert = NULL;
    bencode_obj_str(&cert, i2p_cert_buffer(k->ident->cert), i2p_cert_buffer_length(k->ident->cert));
    bencode_obj_dict_set(benc, "cert", cert);
    bencode_obj_free(&cert);
  }
  res = bencode_write_file(benc, f) != -1;

  bencode_obj_free(&benc);
  bencode_obj_free(&key);

  return res;
}

void i2p_identity_keys_to_public(struct i2p_identity_keys * k, struct i2p_identity ** pub)
{
  *pub = k->ident;
}
