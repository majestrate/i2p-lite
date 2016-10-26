#include <i2pd/crypto.h>
#include <i2pd/dsa.h>
#include <i2pd/elg.h>
#include <i2pd/log.h>

static void benchmark_dsa(size_t n)
{
  struct dsa_Sign * s;
  struct dsa_Verify * v;
  dsa_signature sig;
  uint8_t data [1024] = {0};
  size_t fails = 0;
      
  dsa_privkey priv;
  dsa_pubkey pub;
  
  dsa_keygen(&priv, &pub);
  
  dsa_Sign_new(&s, &priv);
  dsa_Verify_new(&v, &pub);
  
    while(n--) {
      dsa_sign_data(s, data, sizeof(data), &sig);
      if(!dsa_verify_signature(v, data, sizeof(data), &sig))
        fails ++;
    }
    
    dsa_Sign_free(&s);
    dsa_Verify_free(&v);

  if(fails > 0)
    i2p_error(LOG_MAIN, "dsa fails: %lu", fails);
  
}

static void benchmark_elg(size_t n)
{
  elg_key priv, pub;
  elg_block block = {0};
  size_t fails = 0;
  elg_keygen(&priv, &pub);

  struct elg_Encryption * enc;

  elg_Encryption_new(&enc, &pub);
  
  while(n--) {
    elg_Encrypt(enc, &block, 0);
    if(!elg_Decrypt(&priv, &block, 0))
      fails ++;
  }

  elg_Encryption_free(&enc);
}


int main(int argc, char * argv[])
{

  size_t rounds = 100;
  
  struct i2p_crypto_config cc;
  cc.sanity_check = 0;
  
  i2p_log_init();

  i2p_log_set_scope(LOG_ALL);
  i2p_log_set_level(L_INFO);
  i2p_info(LOG_MAIN, "start crypto benchmark");
  
  i2p_crypto_init(cc);

  while(rounds < 10000) {
    i2p_info(LOG_MAIN, "benchmark elg %lu rounds", rounds);
    benchmark_elg(rounds);
    i2p_info(LOG_MAIN, "benchmark dsa %lu rounds", rounds); 
    benchmark_dsa(rounds);
    rounds *= 2;
  }
  
}
