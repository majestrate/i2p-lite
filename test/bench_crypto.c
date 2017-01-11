#include <i2pd/chacha.h>
#include <i2pd/crypto.h>
#include <i2pd/dsa.h>
#include <i2pd/eddsa.h>
#include <i2pd/elg.h>
#include <i2pd/log.h>
#include <i2pd/rand.h>


/*
static void benchmark_aes(size_t n)
{
  size_t fails = 0;
  struct tunnel_AES aes;
  aes_key k = {0};
  i2p_rand(k, sizeof(aes_key));
  tunnel_AES_init(&aes);
  aes_key_new(&aes.layer_key, &k);
  aes_key_new(&aes.iv_key, &k);

  tunnel_data_message msg_in = {0};
  tunnel_data_message msg_out = {0};

  while(n--) {
    aes.encrypt(&aes, &msg_in, &msg_out);
    aes.decrypt(&aes, &msg_in, &msg_out);
    size_t c = 0;
    while(c < sizeof(tunnel_data_message)) {
      if(msg_out[c]) {
        // fail (probably)
        fails ++;
        break;
      }
      c++;
    }
  }

  aes_key_free(&aes.layer_key);
  aes_key_free(&aes.iv_key);

  if(fails) {
    i2p_error(LOG_MAIN, "tunnel crypto fails: %lu", fails);
  }
}
*/

static void benchmark_chacha(size_t n)
{
  size_t encryptfails = 0;
  size_t decryptfails = 0;
  struct tunnel_ChaCha ch;
  tunnel_ChaCha_init(&ch);
  i2p_rand(ch.key, sizeof(tunnel_key_t));

  tunnel_data_message_v2 msg = {0};
  tunnel_data_block_v2 block = {0};

  i2p_rand(block, sizeof(tunnel_data_block_v2));

  while(n--) {
    if(ch.encrypt(&ch, &block, &msg)) encryptfails ++;
    if(ch.decrypt(&ch, &msg, &block)) decryptfails ++;
  }
  if(decryptfails) {
    i2p_error(LOG_MAIN, "%lu decrypt fails", decryptfails);
  }
  if(encryptfails) {
    i2p_error(LOG_MAIN, "%lu encrypt fails", encryptfails);
  }
}

static void benchmark_dsa(size_t n)
{
  struct dsa_Sign * s = NULL;
  struct dsa_Verify * v = NULL;
  dsa_signature sig = {0};
  uint8_t data [1024] = {0};
  size_t fails = 0;
      
  dsa_privkey priv = {0};
  dsa_pubkey pub = {0};
  
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
  elg_key priv = {0};
  elg_key pub = {0};
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

static void benchmark_eddsa(size_t n)
{
  eddsa_privkey priv = {0};
  eddsa_pubkey pub = {0};
  uint8_t data [1024] = {0};
  eddsa_sig sig = {0};
  size_t fails = 0;

  eddsa_keygen(&priv, &pub);

  struct eddsa_Sign * s;
  struct eddsa_Verify * v;

  eddsa_Verify_new(&v, &pub);
  eddsa_Sign_new(&s, &priv);

  while(n--) {
    i2p_rand(data, sizeof(data));
    eddsa_sign_data(s, data, sizeof(data), &sig);
    if (!eddsa_verify_signature(v, data, sizeof(data), &sig))
      fails ++;
  }
  
  eddsa_Sign_free(&s);
  eddsa_Verify_free(&v);
  if(fails > 0)
    i2p_error(LOG_MAIN, "%lu fails", fails);
}


int main(int argc, char * argv[])
{

  size_t rounds = 100;
  
  struct i2p_crypto_config cc;
  cc.sanity_check = 1;
  
  i2p_log_init();

  i2p_log_set_scope(LOG_ALL);
  i2p_log_set_level(L_INFO);
  i2p_info(LOG_MAIN, "start crypto benchmark");
  
  i2p_crypto_init(cc);

  while(rounds < 10000) {
    //i2p_info(LOG_MAIN, "benchmark aes %lu rounds", rounds * 10);
    //benchmark_aes(rounds * 10);
    i2p_info(LOG_MAIN, "benchmark chacha %lu rounds", rounds);
    benchmark_chacha(rounds);
    i2p_info(LOG_MAIN, "benchmark elg %lu rounds", rounds / 10);
    benchmark_elg(rounds / 10);
    i2p_info(LOG_MAIN, "benchmark dsa %lu rounds", rounds); 
    benchmark_dsa(rounds);
    i2p_info(LOG_MAIN, "benchmark eddsa %lu rounds", rounds);
    benchmark_eddsa(rounds);
    rounds *= 2;
  }
  
}
