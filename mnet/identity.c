#include <mnet/datatypes.h>
#include <mnet/cert.h>
#include <mnet/identity.h>
#include <mnet/eddsa.h>
#include <mnet/hash.h>
#include <mnet/log.h>
#include <mnet/memory.h>
#include <mnet/bencode.h>
#include <string.h>
#include <errno.h>


struct mnet_identity
{
  struct mnet_cert * cert;
  struct eddsa_Verify * eddsa;
  ident_hash ident;
};

void mnet_identity_new(struct mnet_identity ** i)
{
  (*i) = xmalloc(sizeof(struct mnet_identity));
}

int mnet_identity_read(struct mnet_identity * i, FILE * f)
{
  eddsa_pubkey ed_pub = {0};
  if(fread(ed_pub, sizeof(eddsa_pubkey), 1, f) != sizeof(eddsa_pubkey)) {
    // bad read
    mnet_error(LOG_DATA, "i2p identity read eddsa pub key failed");
    return 0;
  }

  eddsa_Verify_new(&i->eddsa, &ed_pub);

  if(i->cert) mnet_cert_free(&i->cert); // free existing if it's there
  mnet_cert_new(&i->cert);
  return mnet_cert_read(i->cert, f);
}

uint8_t * mnet_identity_read_buffer(struct mnet_identity * i, uint8_t * in, size_t len)
{
  uint8_t * begin = in;
  eddsa_pubkey ed_pub = {0};
  if(len < (3 + sizeof(eddsa_pubkey))) {
    // too small
    mnet_error(LOG_DATA, "i2p identity buffer too small: %d", len);
    return NULL;
  }
  memcpy(ed_pub, in, sizeof(eddsa_pubkey));
  in += sizeof(eddsa_pubkey);

  if(i->cert) mnet_cert_free(&i->cert); // free if already there

  mnet_cert_new(&i->cert);
  return mnet_cert_read_buffer(i->cert, in, len - sizeof(eddsa_pubkey));
}

int mnet_identity_write(struct mnet_identity * i, FILE * f)
{
  uint8_t * ed_pub = NULL;
  eddsa_Verify_get_key(i->eddsa, &ed_pub);
  if(fwrite(ed_pub, sizeof(eddsa_pubkey), 1, f) == -1) return 0;

  return mnet_cert_write(i->cert, f);
}

void mnet_identity_free(struct mnet_identity ** i)
{
  if(*i)
  {
    if((*i)->cert) mnet_cert_free(&(*i)->cert);
    if((*i)->eddsa) eddsa_Verify_free(&(*i)->eddsa);
    free(*i);
  }
  *i = NULL;
}

size_t mnet_identity_size(struct mnet_identity * i)
{
  return sizeof(eddsa_pubkey) + mnet_cert_buffer_length(i->cert);
}

size_t mnet_identity_siglen(struct mnet_identity * i)
{
  return sizeof(eddsa_sig);
}

int mnet_identity_verify_data(struct mnet_identity * i, uint8_t * data, size_t inlen, uint8_t * sig)
{
  eddsa_sig ed_sig = {0};
  ident_hash h = {0};
  memcpy(ed_sig, sig, sizeof(eddsa_sig));
  mnet_hash(&h, data, inlen);
  return eddsa_verify_signature(i->eddsa, h, sizeof(ident_hash), &ed_sig) != 0;
}

uint16_t mnet_identity_sigtype(struct mnet_identity * i)
{
  return SIGNING_KEY_TYPE_NACL;
}

void mnet_identity_hash(struct mnet_identity * i, ident_hash * h)
{
  struct mnet_hasher * hash = NULL;
  mnet_hasher_new(&hash);

  uint8_t * data = NULL;
  size_t sz = sizeof(eddsa_pubkey);
  eddsa_Verify_get_key(i->eddsa, &data);
  mnet_hasher_update(hash, data, sz);

  data = mnet_cert_buffer(i->cert);
  sz = mnet_cert_buffer_length(i->cert);

  mnet_hasher_update(hash, data, sz);

  mnet_hasher_final(hash, h);
  mnet_hasher_free(&hash);
}

struct mnet_identity_keys
{
  struct mnet_identity * ident;
  struct eddsa_Sign * eddsa;
};

void mnet_identity_keys_new(struct mnet_identity_keys ** k)
{
  *k = xmalloc(sizeof(struct mnet_identity_keys)); 
}

void mnet_identity_keys_free(struct mnet_identity_keys ** k)
{
  if((*k)->eddsa) eddsa_Sign_free(&(*k)->eddsa);
  free(*k);
  *k = NULL;
}

int mnet_identity_keys_generate(struct mnet_identity_keys * k, uint16_t sigtype)
{
  eddsa_pubkey ed_pub = {0};
  eddsa_privkey ed_priv = {0};
  eddsa_keygen(&ed_priv, &ed_pub);
  // free if signer already existing
  if(k->eddsa) eddsa_Sign_free(&k->eddsa);
  // init eddsa signer
  eddsa_Sign_new(&k->eddsa, &ed_priv);
  // free existing public ident
  if(k->ident) mnet_identity_free(&k->ident);
  mnet_identity_new(&k->ident);
  eddsa_Verify_new(&k->ident->eddsa, &ed_pub);

  // init certificate data
  uint8_t cert[3] = {0};
  htobe16buf(cert, sigtype);
  mnet_cert_new(&k->ident->cert);
  mnet_cert_init(k->ident->cert, 0, cert, 0);
  return 1;
}

// context for iterating over identity key dict
struct mnet_keys_iter_ctx
{
  struct mnet_identity_keys * k;
  int success;
};

static void mnet_identity_keys_iter_dict(bencode_obj_t p, const char * k, bencode_obj_t v, void * u)
{
  struct mnet_keys_iter_ctx * ctx = (struct mnet_keys_iter_ctx *) u;
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

int mnet_identity_keys_read(struct mnet_identity_keys * k, FILE * f)
{
  eddsa_privkey ed_priv = {0};
  // free identity if already allocated
  if(k->ident) mnet_identity_free(&k->ident);
  // free signer if already allocated
  if(k->eddsa) eddsa_Sign_free(&k->eddsa);

  bencode_obj_t benc = NULL;
  if(bencode_read_file(&benc, f) == -1) {
    // bad read
    mnet_error(LOG_DATA, "bad read reading identity keys: %s", strerror(errno));
    return -1;
  }
  int res = 0;
  struct mnet_keys_iter_ctx ctx = {
    .k = k,
    .success = 0,
  };
  if(bencode_obj_is_dict(benc)) {
    // yey it's a dict
    bencode_obj_iter_dict(benc, mnet_identity_keys_iter_dict, &ctx);
    // set res to be if we succeeded or not
    res = ctx.success == 1;
  }
  bencode_obj_free(&benc);
  return res;
}

int mnet_identity_keys_write(struct mnet_identity_keys * k, FILE * f)
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
    bencode_obj_str(&cert, mnet_cert_buffer(k->ident->cert), mnet_cert_buffer_length(k->ident->cert));
    bencode_obj_dict_set(benc, "cert", cert);
    bencode_obj_free(&cert);
  }
  res = bencode_write_file(benc, f) != -1;

  bencode_obj_free(&benc);
  bencode_obj_free(&key);

  return res;
}

void mnet_identity_keys_to_public(struct mnet_identity_keys * k, struct mnet_identity ** pub)
{
  *pub = k->ident;
}
