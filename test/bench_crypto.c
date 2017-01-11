#include <mnet/chacha.h>
#include <mnet/crypto.h>
#include <mnet/eddsa.h>
#include <mnet/log.h>
#include <mnet/rand.h>


/*
static void benchmark_aes(size_t n)
{
  size_t fails = 0;
  struct tunnel_AES aes;
  aes_key k = {0};
  mnet_rand(k, sizeof(aes_key));
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
    mnet_error(LOG_MAIN, "tunnel crypto fails: %lu", fails);
  }
}
*/

static void benchmark_chacha(size_t n)
{
  size_t encryptfails = 0;
  size_t decryptfails = 0;
  struct tunnel_ChaCha ch;
  tunnel_ChaCha_init(&ch);
  mnet_rand(ch.key, sizeof(tunnel_key_t));

  tunnel_data_message_v2 msg = {0};
  tunnel_data_block_v2 block = {0};

  mnet_rand(block, sizeof(tunnel_data_block_v2));

  while(n--) {
    if(ch.encrypt(&ch, &block, &msg)) encryptfails ++;
    if(ch.decrypt(&ch, &msg, &block)) decryptfails ++;
  }
  if(decryptfails) {
    mnet_error(LOG_MAIN, "%lu decrypt fails", decryptfails);
  }
  if(encryptfails) {
    mnet_error(LOG_MAIN, "%lu encrypt fails", encryptfails);
  }
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
    mnet_rand(data, sizeof(data));
    eddsa_sign_data(s, data, sizeof(data), &sig);
    if (!eddsa_verify_signature(v, data, sizeof(data), &sig))
      fails ++;
  }
  
  eddsa_Sign_free(&s);
  eddsa_Verify_free(&v);
  if(fails > 0)
    mnet_error(LOG_MAIN, "%lu fails", fails);
}


int main(int argc, char * argv[])
{

  // starting point
  size_t rounds = 1000;
  
  struct mnet_crypto_config cc;
  cc.sanity_check = 1;
  
  mnet_log_init();

  mnet_log_set_scope(LOG_ALL);
  mnet_log_set_level(L_INFO);
  mnet_info(LOG_MAIN, "start crypto benchmark");
  
  mnet_crypto_init(cc);

  while(rounds < 200000) {
    mnet_info(LOG_MAIN, "benchmark chacha %lu rounds", rounds);
    benchmark_chacha(rounds);
    mnet_info(LOG_MAIN, "benchmark eddsa %lu rounds", rounds);
    benchmark_eddsa(rounds);
    rounds *= 2;
  }
  
}
