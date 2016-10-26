#include <i2pd/config.h>
#include <i2pd/crypto.h>
#include <i2pd/log.h>
#include <i2pd/memory.h>
#include <i2pd/ntcp.h>
#include <i2pd/router.h>
#include <i2pd/ssu.h>
#include <i2pd/transport.h>
#include <i2pd/util.h>
#include <i2pd/version.h>

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/** main configuration */
struct main_config
{
  struct router_context_config router;
  struct i2p_crypto_config crypto;
};

/** config iterator for main */
static void iter_config_main(char * k, char * v, void * c)
{
  struct main_config * config = (struct main_config *) c;
  i2p_debug(LOG_MAIN, "config value: %s=%s", k, v);
  if(!strcmp(k, I2P_CONFIG_CRYPTO_CHECK))
    config->crypto.sanity_check = strcmp(v, "1") == 0;
  if(!strcmp(k, I2P_CONFIG_LOG_LEVEL))
    i2p_log_set_level(atoi(v));
}

static void printhelp(const char * argv0)
{
  printf("usage: %s [-h | -f i2p.conf]\n", argv0);
}

// check if a file exists 
static int file_exists(char * path)
{
  FILE * f = fopen(path, "r");
  if(!f) return 0;
  fclose(f);
  return 1;
}


int main(int argc, char * argv[])
{

  int opt;
  char * configfile;
  int loglevel = L_INFO;
  
  char * home = getenv("HOME");
  if(home)
    configfile = path_join(home, ".i2p.conf", 0);
  
  while((opt = getopt(argc, argv, "vhf:")) != -1) {
    switch(opt) {
    case 'h':
      printhelp(argv[0]);
      return 1;
    case 'f':
      configfile = strdup(argv[optind-1]);
      break;
    case 'v':
      loglevel = L_DEBUG;
      break;
    }
  }
  
  
  i2p_log_init();
  
  i2p_log_set_level(loglevel);
  // set default scope to all
  i2p_log_set_scope(LOG_ALL);

  // generate config file if it's not there
  if (!file_exists(configfile)) {
    if(!i2p_config_gen(configfile)) {
      i2p_error(LOG_MAIN, "failed to write to %s: %s", configfile, strerror(errno));
      return -1;
    }
  }
  
  struct i2p_config * cfg;
  if(!i2p_config_load(&cfg, configfile)) {
    i2p_error(LOG_MAIN, "failed to load %s", configfile);
    return 1;
  }
  free(configfile);

  struct main_config * config = mallocx(sizeof(struct main_config), MALLOCX_ZERO);
  i2p_config_for_each(cfg, iter_config_main, config);
  i2p_info(LOG_MAIN, "i2pd-uv %s starting up", I2PD_VERSION);

  
  if(!i2p_crypto_init(config->crypto)) {
    i2p_error(LOG_MAIN, "crypto init failed");
    return 1;
  }
  // turn off crypto logging now that we enter setup
  i2p_log_set_scope(LOG_ALL | (~LOG_CRYPTO));
  
  i2p_crypto_done();
  free(config);
  return 0;
}
