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
  size_t vl = strlen(v);
  i2p_debug(LOG_MAIN, "config value: %s=%s", k, v);
  if(!strcmp(k, I2P_CONFIG_CRYPTO_CHECK))
    config->crypto.sanity_check = strcmp(v, "1") == 0;
  if(!strcmp(k, I2P_CONFIG_LOG_LEVEL))
    i2p_log_set_level(atoi(v));
  if(!strcmp(k, I2P_CONFIG_ROUTER_DIR))
    memcpy(config->router.datadir, v, vl < sizeof(i2p_filename) ? vl : sizeof(i2p_filename));
}

static void printhelp(const char * argv0)
{
  printf("usage: %s [-h | -f i2p.conf]\n", argv0);
}


int main(int argc, char * argv[])
{

  int opt;
  char * configfile = NULL;
  char * floodfill_bootstrap = NULL;
  char * reseed_url = NULL;
  int loglevel = L_INFO;
  
  struct router_context * router = NULL;
  
  while((opt = getopt(argc, argv, "vhf:b:u:")) != -1) {
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
    case 'b':
      floodfill_bootstrap = argv[optind-1];
      break;
    case 'u':
      reseed_url = argv[optind-1];
    default:
      printhelp(argv[0]);
      return 1;
    }
  }
  if (optind >= argc && !configfile && argc > 1) {
    if(loglevel != L_DEBUG)
      return -1;
  }
  if(!configfile) {
    char * home = getenv("HOME");
    if(home)
      configfile = path_join(home, ".i2p.conf", 0);
    else
      configfile = path_join(".", "i2p.conf", 0); // no $HOME ? lolz okay
  }
  
  i2p_log_init();
  
  i2p_log_set_level(loglevel);
  // set default scope to all
  i2p_log_set_scope(LOG_ALL);

  // generate config file if it's not there
  if (!check_file(configfile)) {
    i2p_info(LOG_MAIN, "generate new config file %s", configfile);
    if(!i2p_config_gen(configfile)) {
      i2p_error(LOG_MAIN, "failed to write to %s: %s", configfile, strerror(errno));
      return -1;
    }
  }
  
  struct i2p_config * cfg = NULL;
  if(!i2p_config_load(&cfg, configfile)) {
    i2p_error(LOG_MAIN, "failed to load %s", configfile);
    return 1;
  }
  
  free(configfile);
  
  struct main_config config = {
    default_router_context_config,
    default_crypto_config
  };
  
  uv_loop_t * loop = uv_default_loop();
  config.router.loop = loop;
  
  i2p_config_for_each(cfg, iter_config_main, &config);

  if(config.router.datadir[0] == 0) {
    // set default data dir
    char * dir = getenv("I2P_ROOT");
    if(dir) {
      size_t dlen = strlen(dir);
      if(dlen < sizeof(i2p_filename))
        memcpy(config.router.datadir, dir, dlen);
    } else {
      char * home = getenv("HOME");
      dir = path_join(home, ".i2p-lite", 0);
      size_t dlen = strlen(dir);
      if(dlen < sizeof(i2p_filename))
        memcpy(config.router.datadir, dir, dlen);
      free(dir);
    }
  }
  /** floodfill from bootstrap specified */
  if(floodfill_bootstrap) {
    int flen = strlen(floodfill_bootstrap);
    if(flen < sizeof(i2p_filename)) {
      memcpy(config.router.floodfill, floodfill_bootstrap, flen);
    }
  }

  /** reseed url manually specified */
  if(reseed_url) {
    config.router.reseed_url = reseed_url;
  }
  
  i2p_info(LOG_MAIN, "%s %s starting up", I2PD_NAME, I2PD_VERSION);

  
  if(!i2p_crypto_init(config.crypto)) {
    i2p_error(LOG_MAIN, "crypto init failed");
    return 1;
  }
  // turn off crypto logging now that we enter setup
  i2p_log_set_scope(LOG_ALL | (~LOG_CRYPTO));

  i2p_info(LOG_MAIN, "i2p router context initialize");
  // init router context
  router_context_new(&router, config.router);
  
  if(router_context_load(router))
  {
    i2p_info(LOG_MAIN, "i2p router context loaded up");
    router_context_run(router);
    uv_run(loop, UV_RUN_DEFAULT);
    i2p_info(LOG_MAIN, "i2p router shutting down");
  }
  

  // free router context
  router_context_free(&router);

  // we done
  i2p_crypto_done();
  return 0;
}
