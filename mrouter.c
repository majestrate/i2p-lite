#include <mnet/config.h>
#include <mnet/crypto.h>
#include <mnet/log.h>
#include <mnet/memory.h>
#include <mnet/router.h>
#include <mnet/transport.h>
#include <mnet/util.h>
#include <mnet/version.h>

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
  struct mnet_crypto_config crypto;
};

/** config iterator for main */
static void iter_config_main(char * k, char * v, void * c)
{
  struct main_config * config = (struct main_config *) c;
  size_t vl = strlen(v);
  mnet_debug(LOG_MAIN, "config value: %s=%s", k, v);
  if(!strcmp(k, MNET_CONFIG_CRYPTO_CHECK))
    config->crypto.sanity_check = strcmp(v, "1") == 0;
  if(!strcmp(k, MNET_CONFIG_LOG_LEVEL))
    mnet_log_set_level(atoi(v));
  if(!strcmp(k, MNET_CONFIG_ROUTER_DIR))
    config->router.datadir = strdup(k);
}

static void printhelp(const char * argv0)
{
  printf("usage: %s [-h | -f i2p.conf]\n", argv0);
}


int main(int argc, char * argv[])
{

  struct main_config config = {
    default_router_context_config,
    default_crypto_config
  };
  
  int opt;
  char * configfile = NULL;
  char * bootstrap = NULL;
  int loglevel = L_INFO;
  
  struct router_context * router = NULL;
  
  while((opt = getopt(argc, argv, "vhf:b:")) != -1) {
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
      bootstrap = argv[optind-1];
      break;
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
      configfile = path_join(home, ".mnet.conf", 0);
    else
      configfile = path_join(".", "mnet.conf", 0); // no $HOME ? lolz okay
  }
  
  mnet_log_init();
  
  mnet_log_set_level(loglevel);
  // set default scope to all
  mnet_log_set_scope(LOG_ALL);

  // generate config file if it's not there
  if (!check_file(configfile)) {
    mnet_info(LOG_MAIN, "generate new config file %s", configfile);
    if(!mnet_config_gen(configfile)) {
      mnet_error(LOG_MAIN, "failed to write to %s: %s", configfile, strerror(errno));
      return -1;
    }
  }
  
  struct mnet_config * cfg = NULL;
  if(!mnet_config_load(&cfg, configfile)) {
    mnet_error(LOG_MAIN, "failed to load %s", configfile);
    return 1;
  }
  
  free(configfile);
  
  uv_loop_t * loop = uv_default_loop();
  config.router.loop = loop;
  
  mnet_config_for_each(cfg, iter_config_main, &config);

  if(config.router.datadir == NULL) {
    // set default data dir
    char * dir = getenv("MNET_ROOT");
    if(dir) {
      size_t dlen = strlen(dir);
      if(dlen < sizeof(mnet_filename))
        memcpy(config.router.datadir, dir, dlen);
    } else {
      char * home = getenv("HOME");
      config.router.datadir = path_join(home, ".mnet", 0);
    }
  }
  

  
  mnet_info(LOG_MAIN, "%s %s starting up", MNET_NAME, MNET_VERSION);

  
  if(!mnet_crypto_init(config.crypto)) {
    mnet_error(LOG_MAIN, "crypto init failed");
    return 1;
  }
  // turn off crypto logging now that we enter setup
  mnet_log_set_scope(LOG_ALL | (~LOG_CRYPTO));

  mnet_info(LOG_MAIN, "M router context initialize");
  // init router context
  router_context_new(&router, config.router);
  
  if(router_context_load(router))
  {
    mnet_info(LOG_MAIN, "M router context loaded up");
    router_context_run(router);
    uv_run(loop, UV_RUN_DEFAULT);
    mnet_info(LOG_MAIN, "M router shutting down");
  }
  

  // free router context
  router_context_free(&router);

  // we done
  mnet_crypto_done();
  return 0;
}
