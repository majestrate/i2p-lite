#include <i2pd/crypto.h>
#include <i2pd/log.h>
#include <i2pd/memory.h>
#include <i2pd/netdb.h>
#include <stdio.h>
#include <string.h>

struct netdb_process_ctx
{
  size_t valid;
  size_t hashfail;
  size_t invalid;
};

void print_i2p_addr(struct router_info * ri, struct i2p_addr * addr, void * u)
{
  char * ident = router_info_base64_ident(ri);
  i2p_info(LOG_MAIN, "router info %s has %s address %s port %d ", ident, addr->style, addr->host, addr->port);
  free(ident);
}

void process_netdb_entry(netdb_entry * ent, void * u)
{
  struct netdb_process_ctx * ctx = (struct netdb_process_ctx * ) u;
  ident_hash h;
  if(router_info_verify(ent->ri)) {
    router_info_hash(ent->ri, &h);
    if (memcmp(h, ent->ident, sizeof(ident_hash))) {
      ctx->hashfail ++;
    } else {
      ctx->valid ++;
      router_info_iter_addrs(ent->ri, print_i2p_addr, NULL);
    }
  } else {
    ctx->invalid ++;
  }
}

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
  struct netdb_process_ctx ctx = {0, 0, 0};
  
  i2p_netdb_for_each(db, process_netdb_entry, &ctx);
  i2p_info(LOG_MAIN, "valid %lu", ctx.valid );
  i2p_info(LOG_MAIN, "invalid %lu", ctx.invalid);
  i2p_info(LOG_MAIN, "hashfail %lu", ctx.hashfail);
  i2p_netdb_free(&db);
}
