#include <assert.h>
#include <mnet/crypto.h>
#include <mnet/log.h>
#include <mnet/memory.h>
#include <mnet/rand.h>
#include <mnet/eddsa.h>
#include <sodium/core.h>

static int eddsa_test()
{
  int ret = 0;
  uint8_t data[1024] = {0};
  
  struct eddsa_Sign * s = NULL;
  struct eddsa_Verify * v = NULL;
  
  eddsa_sig sig = {0};

  eddsa_privkey priv = {0};
  eddsa_pubkey pub = {0};

  mnet_debug(LOG_CRYPTO, "eddsa keygen");
  eddsa_keygen(&priv, &pub);


  eddsa_Sign_new(&s, &priv);
  eddsa_Verify_new(&v, &pub);
  
  mnet_rand(data, sizeof(data));

  eddsa_sign_data(s, data, sizeof(data), &sig);
  ret = eddsa_verify_signature(v, data, sizeof(data), &sig);
  
  eddsa_Sign_free(&s);
  eddsa_Verify_free(&v);
  
  return ret;
}

int mnet_crypto_init(struct mnet_crypto_config cfg)
{
  int ret = 1;
  if(sodium_init() == -1) {
    return 0;
  }
  if (cfg.sanity_check) {
    mnet_info(LOG_CRYPTO, "doing crypto sanity check");
    if(!eddsa_test()) {
      mnet_error(LOG_CRYPTO, "eddsa test failure");
      ret = 0;
    }
    if(ret) mnet_info(LOG_CRYPTO, "crypto is sane :^D");
  }
  return ret;
}


void mnet_crypto_done()
{
}
