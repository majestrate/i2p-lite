#include <assert.h>
#include <i2pd/crypto.h>
#include <i2pd/dsa.h>
#include <i2pd/eddsa.h>
#include <i2pd/elg.h>
#include <i2pd/log.h>
#include <i2pd/memory.h>
#include <openssl/crypto.h>
#include <openssl/dsa.h>
#include <openssl/ssl.h>
#include <openssl/rand.h>


// take care about openssl version
#include <openssl/opensslv.h>
#if (OPENSSL_VERSION_NUMBER < 0x010100000) || defined(LIBRESSL_VERSION_NUMBER) // 1.1.0 or LibreSSL
// define getters and setters introduced in 1.1.0
static int DSA_set0_pqg(DSA *d, BIGNUM *p, BIGNUM *q, BIGNUM *g) { d->p = p; d->q = q; d->g = g; return 1; }
static int DSA_set0_key(DSA *d, BIGNUM *pub_key, BIGNUM *priv_key) { d->pub_key = pub_key; d->priv_key = priv_key; return 1; } 
#endif

static void bn2buf(const BIGNUM * bn, uint8_t * buf, size_t len)
{
  int offset = len - BN_num_bytes(bn);
  assert(offset >= 0);
  BN_bn2bin(bn, buf+offset);
  memset(buf, 0, offset);
}



const uint8_t elgp_[256]= {
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC9, 0x0F, 0xDA, 0xA2, 0x21, 0x68, 0xC2, 0x34, 
  0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1, 0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74, 
  0x02, 0x0B, 0xBE, 0xA6, 0x3B, 0x13, 0x9B, 0x22, 0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
  0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B, 0x30, 0x2B, 0x0A, 0x6D, 0xF2, 0x5F, 0x14, 0x37,
  0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45, 0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6,
  0xF4, 0x4C, 0x42, 0xE9, 0xA6, 0x37, 0xED, 0x6B, 0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
  0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5, 0xAE, 0x9F, 0x24, 0x11, 0x7C, 0x4B, 0x1F, 0xE6,
  0x49, 0x28, 0x66, 0x51, 0xEC, 0xE4, 0x5B, 0x3D, 0xC2, 0x00, 0x7C, 0xB8, 0xA1, 0x63, 0xBF, 0x05, 
  0x98, 0xDA, 0x48, 0x36, 0x1C, 0x55, 0xD3, 0x9A, 0x69, 0x16, 0x3F, 0xA8, 0xFD, 0x24, 0xCF, 0x5F,
  0x83, 0x65, 0x5D, 0x23, 0xDC, 0xA3, 0xAD, 0x96, 0x1C, 0x62, 0xF3, 0x56, 0x20, 0x85, 0x52, 0xBB,
  0x9E, 0xD5, 0x29, 0x07, 0x70, 0x96, 0x96, 0x6D, 0x67, 0x0C, 0x35, 0x4E, 0x4A, 0xBC, 0x98, 0x04, 
  0xF1, 0x74, 0x6C, 0x08, 0xCA, 0x18, 0x21, 0x7C, 0x32, 0x90, 0x5E, 0x46, 0x2E, 0x36, 0xCE, 0x3B,
  0xE3, 0x9E, 0x77, 0x2C, 0x18, 0x0E, 0x86, 0x03, 0x9B, 0x27, 0x83, 0xA2, 0xEC, 0x07, 0xA2, 0x8F,
  0xB5, 0xC5, 0x5D, 0xF0, 0x6F, 0x4C, 0x52, 0xC9, 0xDE, 0x2B, 0xCB, 0xF6, 0x95, 0x58, 0x17, 0x18,
  0x39, 0x95, 0x49, 0x7C, 0xEA, 0x95, 0x6A, 0xE5, 0x15, 0xD2, 0x26, 0x18, 0x98, 0xFA, 0x05, 0x10,
  0x15, 0x72, 0x8E, 0x5A, 0x8A, 0xAC, 0xAA, 0x68, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

const int elgg_ = 2;

const uint8_t dsap_[128]= {
  0x9c, 0x05, 0xb2, 0xaa, 0x96, 0x0d, 0x9b, 0x97, 0xb8, 0x93, 0x19, 0x63, 0xc9, 0xcc, 0x9e, 0x8c,
  0x30, 0x26, 0xe9, 0xb8, 0xed, 0x92, 0xfa, 0xd0, 0xa6, 0x9c, 0xc8, 0x86, 0xd5, 0xbf, 0x80, 0x15,
  0xfc, 0xad, 0xae, 0x31, 0xa0, 0xad, 0x18, 0xfa, 0xb3, 0xf0, 0x1b, 0x00, 0xa3, 0x58, 0xde, 0x23,
  0x76, 0x55, 0xc4, 0x96, 0x4a, 0xfa, 0xa2, 0xb3, 0x37, 0xe9, 0x6a, 0xd3, 0x16, 0xb9, 0xfb, 0x1c,
  0xc5, 0x64, 0xb5, 0xae, 0xc5, 0xb6, 0x9a, 0x9f, 0xf6, 0xc3, 0xe4, 0x54, 0x87, 0x07, 0xfe, 0xf8,
  0x50, 0x3d, 0x91, 0xdd, 0x86, 0x02, 0xe8, 0x67, 0xe6, 0xd3, 0x5d, 0x22, 0x35, 0xc1, 0x86, 0x9c,
  0xe2, 0x47, 0x9c, 0x3b, 0x9d, 0x54, 0x01, 0xde, 0x04, 0xe0, 0x72, 0x7f, 0xb3, 0x3d, 0x65, 0x11, 
  0x28, 0x5d, 0x4c, 0xf2, 0x95, 0x38, 0xd9, 0xe3, 0xb6, 0x05, 0x1f, 0x5b, 0x22, 0xcc, 0x1c, 0x93
};

const uint8_t dsaq_[20]={
  0xa5, 0xdf, 0xc2, 0x8f, 0xef, 0x4c, 0xa1, 0xe2, 0x86, 0x74, 0x4c, 0xd8, 0xee, 0xd9, 0xd2, 0x9d,
  0x68, 0x40, 0x46, 0xb7
};

const uint8_t dsag_[128]={
  0x0c, 0x1f, 0x4d, 0x27, 0xd4, 0x00, 0x93, 0xb4, 0x29, 0xe9, 0x62, 0xd7, 0x22, 0x38, 0x24, 0xe0,
  0xbb, 0xc4, 0x7e, 0x7c, 0x83, 0x2a, 0x39, 0x23, 0x6f, 0xc6, 0x83, 0xaf, 0x84, 0x88, 0x95, 0x81,
  0x07, 0x5f, 0xf9, 0x08, 0x2e, 0xd3, 0x23, 0x53, 0xd4, 0x37, 0x4d, 0x73, 0x01, 0xcd, 0xa1, 0xd2,
  0x3c, 0x43, 0x1f, 0x46, 0x98, 0x59, 0x9d, 0xda, 0x02, 0x45, 0x18, 0x24, 0xff, 0x36, 0x97, 0x52,
  0x59, 0x36, 0x47, 0xcc, 0x3d, 0xdc, 0x19, 0x7d, 0xe9, 0x85, 0xe4, 0x3d, 0x13, 0x6c, 0xdc, 0xfc,
  0x6b, 0xd5, 0x40, 0x9c, 0xd2, 0xf4, 0x50, 0x82, 0x11, 0x42, 0xa5, 0xe6, 0xf8, 0xeb, 0x1c, 0x3a,
  0xb5, 0xd0, 0x48, 0x4b, 0x81, 0x29, 0xfc, 0xf1, 0x7b, 0xce, 0x4f, 0x7f, 0x33, 0x32, 0x1c, 0x3c, 
  0xb3, 0xdb, 0xb1, 0x4a, 0x90, 0x5e, 0x7b, 0x2b, 0x3e, 0x93, 0xbe, 0x47, 0x08, 0xcb, 0xcc, 0x82  
};

const int rsae_ = 65537;	

struct crypto_consts
{
  // elgamal
  BIGNUM * elgp;
  BIGNUM * elgg;

  // DSA
  BIGNUM * dsap;
  BIGNUM * dsaq;
  BIGNUM * dsag;

  // RSA
  BIGNUM * rsae;
};


struct crypto_consts * cc = NULL;

// create new dsa context with i2p primes
DSA * createDSA()
{
  DSA * dsa = DSA_new();
  DSA_set0_pqg(dsa, BN_dup(cc->dsap), BN_dup(cc->dsaq), BN_dup(cc->dsag));
  DSA_set0_key(dsa, NULL, NULL);
  return dsa;
}


// init i2p elg parameters
static void i2p_elg_init()
{
  cc->elgp = BN_new();
  BN_bin2bn(elgp_, 256, cc->elgp);
  cc->elgg = BN_new();
  BN_set_word(cc->elgg, elgg_);
}

// deinit i2p elg parameters
static void i2p_elg_deinit()
{
  BN_free(cc->elgp);
  BN_free(cc->elgg);
}


// init dsa primes
static void i2p_dsa_init()
{
  cc->dsap = BN_new();
  cc->dsaq = BN_new();
  cc->dsag = BN_new();
  BN_bin2bn(dsap_, 128, cc->dsap);
  BN_bin2bn(dsaq_, 20, cc->dsaq);
  BN_bin2bn(dsag_, 128, cc->dsag);
}

// deinit dsa primes
static void i2p_dsa_deinit()
{
  BN_free(cc->dsap);
  BN_free(cc->dsag);
  BN_free(cc->dsaq);
}

static void i2p_rsa_init()
{
  cc->rsae = BN_new();
  BN_set_word(cc->rsae, rsae_);
}

static void i2p_rsa_deinit()
{
  BN_free(cc->rsae);
}

static int elg_test()
{
  int ret;
  elg_key priv = {0};
  elg_key pub = {0};
  elg_block block = {0};
  
  struct elg_Encryption * e;

  i2p_debug(LOG_CRYPTO, "test generate elg key");
  elg_keygen(&priv, &pub);

  memcpy(block, "test", 5);
  
  elg_Encryption_new(&e, pub);

  i2p_debug(LOG_CRYPTO, "test elg encrypt");
  elg_Encrypt(e, &block, 0);

  i2p_debug(LOG_CRYPTO, "test elg decrypt");
  ret = elg_Decrypt(priv, &block, 0);

  elg_Encryption_free(&e);

  i2p_debug(LOG_CRYPTO, "elg test %d", ret);
  
  return ret;
}

static int dsa_test()
{
  int ret = 0;
  // data to sign
  uint8_t block[1024] = {0};
  // signature
  dsa_signature sig = {0};

  // keypair
  dsa_pubkey pub = {0};
  dsa_privkey priv = {0};

  struct dsa_Sign * signer;
  struct dsa_Verify * verifier;
  
  i2p_debug(LOG_CRYPTO, "test dsa keygen");
  dsa_keygen(&priv, &pub);

  // generate random block
  RAND_bytes(block, sizeof(block));

  // new signer 
  dsa_Sign_new(&signer, priv);
  // new verifier
  dsa_Verify_new(&verifier, pub);

  // sign
  dsa_sign_data(signer, block, sizeof(block), &sig);
  // verify
  ret = dsa_verify_signature(verifier, block, sizeof(block), sig);

  // free
  dsa_Sign_free(&signer);
  dsa_Verify_free(&verifier);
  
  i2p_debug(LOG_CRYPTO, "dsa test %d", ret);
  return ret;
}

static int ecdsa_test()
{
  return 1;
}

static int eddsa_test()
{
  int ret;
  uint8_t data[1024] = {0};
  
  struct eddsa_Sign * s;
  struct eddsa_Verify * v;
  
  eddsa_sig sig = {0};

  eddsa_privkey priv = {0};
  eddsa_pubkey pub = {0};

  i2p_debug(LOG_CRYPTO, "eddsa keygen");
  eddsa_keygen(&priv, &pub);


  eddsa_Sign_new(&s, priv);
  eddsa_Verify_new(&v, pub);
  
  RAND_bytes(data, sizeof(data));

  eddsa_sign_data(s, data, sizeof(data), &sig);

  ret = eddsa_verify_signature(v, data, sizeof(data), sig);
  
  eddsa_Sign_free(&s);
  eddsa_Verify_free(&v);
  
  return ret;
}

int i2p_crypto_init(struct i2p_crypto_config cfg)
{
  int ret = 1;
  SSL_library_init();
  cc = mallocx(sizeof(struct crypto_consts), MALLOCX_ZERO);
  i2p_elg_init();
  i2p_dsa_init();
  i2p_rsa_init();
  if (cfg.sanity_check) {
    i2p_info(LOG_CRYPTO, "doing crypto sanity check");
    if(!elg_test()) {
      i2p_error(LOG_CRYPTO, "elg test failure");
      ret = 0;
    }
    if(!dsa_test()) {
      i2p_error(LOG_CRYPTO, "dsa test failure");
      ret = 0;
    }
    if (!ecdsa_test()) {
      i2p_error(LOG_CRYPTO, "ecdsa test failure");
      ret = 0;
    }
    if(!eddsa_test()) {
      i2p_error(LOG_CRYPTO, "eddsa test failure");
      ret = 0;
    }
    if(ret) i2p_info(LOG_CRYPTO, "crypto is sane :^D");
  }
  return ret;
}


void i2p_crypto_done()
{
  i2p_elg_deinit();
  i2p_dsa_deinit();
  i2p_rsa_deinit();
  free(cc);
}

void elg_keygen(elg_key * priv, elg_key * pub)
{
  BIGNUM * p;
  BN_CTX * c;
  c = BN_CTX_new();
  p = BN_new();
  BN_rand(p, 2048, -1, 1); // full exponent
  bn2buf(p, (*priv), 256);
  BN_mod_exp(p, cc->elgg, p, cc->elgp, c);
  bn2buf(p, (*pub), 256);
  BN_free(p);
  BN_CTX_free(c);
}

struct elg_Encryption
{
  BN_CTX * c;
  BIGNUM * a;
  BIGNUM * b1;
};

void elg_Encryption_new(struct elg_Encryption ** e, elg_key pub)
{
  BIGNUM * k, * y;
  *e = mallocx(sizeof(struct elg_Encryption), MALLOCX_ZERO);
  (*e)->c = BN_CTX_new();
  k = BN_new();
  y = BN_new();
  (*e)->a = BN_new();
  (*e)->b1 = BN_new();
  
  BN_rand(k, 2048, -1, 1); // full exponent
  BN_mod_exp((*e)->a, cc->elgg, k, cc->elgp, (*e)->c);
  BN_bin2bn(pub, 256, y);
  BN_mod_exp((*e)->b1, y, k, cc->elgp, (*e)->c);
  BN_free(k);
  BN_free(y);
}

void elg_Encryption_free(struct elg_Encryption ** e)
{
  BN_free((*e)->a);
  BN_free((*e)->b1);
  BN_CTX_free((*e)->c);
  free(*e);
  *e = NULL;
}

void elg_Encrypt(struct elg_Encryption * e, elg_block * block, int zeropad)
{
  // create m
  uint8_t m[255] = {0};
  BIGNUM * b = BN_new();
  uint8_t * d = *block;
  m[0] = 0xff;
  memcpy(m+33, d, ELG_DATA_SIZE);
  SHA256(m+33, ELG_DATA_SIZE, m+1);
  // calculate b = b1 * m mod p
  BN_bin2bn(m, 255, b);
  BN_mod_mul(b, e->b1, b, cc->elgp, e->c);
  elg_block_wipe(block);
  if(zeropad) {
    bn2buf(e->a, (*block+1), 256);
    bn2buf(b, (*block+258), 256);
  } else {
    bn2buf(e->a, (*block), 256);
    bn2buf(b, (*block+256), 256);
  }
  BN_free(b);
}

int elg_Decrypt(elg_key priv, elg_block * block, int zeropad)
{
  int ret;
  BN_CTX * c;
  BIGNUM * x, * a, * b;

  uint8_t * e;
  uint8_t m[255] = {0};
  uint8_t hash[32] = {0};

  c = BN_CTX_new();
  
  x = BN_new();
  a = BN_new();
  b = BN_new();

  BN_bin2bn(priv, 256, x);
  BN_sub(x, cc->elgp, x); BN_sub_word(x, 1); // x = elgp - x - 1

  e = (*block);
  BN_bin2bn (zeropad ? e + 1 : e, 256, a);
  BN_bin2bn (zeropad ? e + 258 : e + 256, 256, b);

  // m = b * (a ^ x mod p ) mod p
  BN_mod_exp(x, a, x, cc->elgp, c);
  BN_mod_mul(b, b, x, cc->elgp, c);

  bn2buf(b, m, 255);

  BN_free(x); BN_free(a); BN_free(b);
    
  SHA256(m + 33, ELG_DATA_SIZE, hash);

  ret = 0;
  
  if(memcmp(m + 1, hash, 32) == 0) {
    // decrypt success
    ret = 1;
    memcpy(e, m + 33, ELG_DATA_SIZE);
  }
  BN_CTX_free(c);
  return ret;
}


void elg_block_wipe(elg_block * block)
{
  memset((*block), 0, ELG_BLOCK_SIZE);
}


void dsa_keygen(dsa_privkey * priv, dsa_pubkey * pub)
{
  DSA * d = createDSA();

  DSA_generate_key(d);

  bn2buf(d->priv_key, *priv, sizeof(dsa_privkey));
  bn2buf(d->pub_key, *pub, sizeof(dsa_pubkey));
  
  DSA_free(d);
}

struct dsa_Sign
{
  DSA * d;
};

void dsa_Sign_new(struct dsa_Sign ** signer, dsa_privkey priv)
{
  *signer = mallocx(sizeof(struct dsa_Sign), MALLOCX_ZERO);
  (*signer)->d = createDSA();
  (*signer)->d->priv_key = BN_bin2bn(priv, DSA_PRIVKEY_LENGTH, NULL);
}

void dsa_Sign_free(struct dsa_Sign ** signer)
{
  // BN_free((*signer)->d->priv_key); // TODO: is this needed? 
  DSA_free((*signer)->d);
  free(*signer);
  *signer = NULL;
}

void dsa_sign_data(struct dsa_Sign * signer, const uint8_t * data, const size_t len, dsa_signature * sig)
{
  uint8_t digest[20] = {0};
  SHA1(data, len, digest);
  DSA_SIG * s = DSA_do_sign(digest, 20, signer->d);
  bn2buf(s->r, *sig, (DSA_SIG_LENGTH/2));
  bn2buf(s->s, (*sig+ (DSA_SIG_LENGTH/2)), DSA_SIG_LENGTH/2);
  DSA_SIG_free(s);
}

struct dsa_Verify
{
  DSA * d;
};

void dsa_Verify_new(struct dsa_Verify ** v, dsa_pubkey pub)
{
  *v = mallocx(sizeof(struct dsa_Sign), MALLOCX_ZERO);
  (*v)->d = createDSA();
  (*v)->d->pub_key = BN_bin2bn(pub, DSA_PUBKEY_LENGTH, NULL);
}

void dsa_Verify_free(struct dsa_Verify ** v)
{
  // BN_free((*v)->d->pub_key); // TODO: is this needed? 
  DSA_free((*v)->d);
  free(*v);
  *v = NULL;
}

int dsa_verify_signature(struct dsa_Verify * v, const uint8_t * data, const size_t len, dsa_signature sig)
{
  int ret = 0;
  uint8_t digest[20] = {0};
  SHA1(data, len, digest);
  DSA_SIG * s = DSA_SIG_new();
  s->r = BN_bin2bn(sig, DSA_SIG_LENGTH/2, NULL);
  s->s = BN_bin2bn(sig + (DSA_SIG_LENGTH/2), DSA_SIG_LENGTH/2, NULL);
  ret = DSA_do_verify(digest, 20, s, v->d) != -1;
  DSA_SIG_free(s);
  return ret;
}

