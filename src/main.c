#include <i2pd/crypto.h>
#include <i2pd/log.h>
#include <i2pd/version.h>

int main(int argc, char * argv[])
{
  i2p_log_init(L_INFO, LOG_ALL);
  i2p_info(LOG_MAIN, "i2pd-uv %s", I2PD_VERSION);
  
  i2p_crypto_init();
  if(!i2p_crypto_test()) {
    i2p_error(LOG_CRYPTO, "crypto test failed");
    return 1;
  }
  i2p_info(LOG_MAIN, "crypto test success");
  
  i2p_crypto_done();
  return 0;
}
