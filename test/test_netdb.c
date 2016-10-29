#include <i2pd/crypto.h>
#include <i2pd/log.h>
#include <i2pd/netdb.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char * argv[])
{
  if(argc != 2) {
    printf("usage: %s /path/to/netDb\n", argv[0]);
    return 1;
  }

  i2p_log_init();
  i2p_log_set_level(L_INFO);
  i2p_log_set_scope(LOG_ALL);

  struct i2p_crypto_config c = default_crypto_config;
  
  i2p_crypto_init(c);
  
  i2p_debug(LOG_MAIN, "starting netdb test");
  
  struct i2p_netdb * db;
  i2p_netdb_new(&db, argv[1]);

  int result;
  i2p_debug(LOG_MAIN, "ensure skiplist");
  result = i2p_netdb_ensure_skiplist(db);
  i2p_debug(LOG_MAIN, "load all");
  result = i2p_netdb_load_all(db);
  i2p_debug(LOG_MAIN, "i2p_netdb_load_all: %d", result);
  
  i2p_netdb_free(&db);
}
