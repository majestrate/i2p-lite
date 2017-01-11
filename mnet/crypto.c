#include <assert.h>
#include <i2pd/crypto.h>
#include <i2pd/log.h>
#include <i2pd/memory.h>
#include <i2pd/rand.h>
#include <mnet/eddsa.h>
#include <sodium/core.h>

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


  eddsa_Sign_new(&s, &priv);
  eddsa_Verify_new(&v, &pub);
  
  mnet_rand(data, sizeof(data));

  eddsa_sign_data(s, data, sizeof(data), &sig);

  ret = eddsa_verify_signature(v, data, sizeof(data), &sig);
  
  eddsa_Sign_free(&s);
  eddsa_Verify_free(&v);
  
  return ret;
}

int i2p_crypto_init(struct i2p_crypto_config cfg)
{
  int ret = 1;
  if(sodium_init() == -1) {
    return 0;
  }
  if (cfg.sanity_check) {
    log_info(LOG_CRYPTO, "doing crypto sanity check");
    if(!eddsa_test()) {
      log_error(LOG_CRYPTO, "eddsa test failure");
      ret = 0;
    }
    if(ret) i2p_info(LOG_CRYPTO, "crypto is sane :^D");
  }
  return ret;
}


void i2p_crypto_done()
{
}
