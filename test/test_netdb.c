#include <i2pd/crypto.h>
#include <i2pd/encoding.h>
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

struct netdb_find_ctx
{
  struct i2p_netdb * db;
  size_t hit;
  size_t miss;
};

void test_find_netdb_entry(ident_hash h, struct router_info * drop, void * u)  
{
  (void) drop;
  ident_hash ih;
  struct netdb_find_ctx * ctx = (struct netdb_find_ctx *) u;
  struct router_info * ri = NULL;
  if(i2p_netdb_find_router_info(ctx->db, h, &ri)) {
    // found
    if(ri) {
      router_info_hash(ri, &ih);
      if(memcmp(h, ih, sizeof(ident_hash)) == 0) {
        ctx->hit ++;
        return;
      }
    }
  }
  ctx->miss ++;
}
  
void print_i2p_addr(struct router_info * ri, struct i2p_addr * addr, void * u)
{
  char * ident = router_info_base64_ident(ri);
  i2p_info(LOG_MAIN, "router info %s has %s address %s port %d ", ident, addr->style, addr->host, addr->port);
  free(ident);
}

void process_netdb_entry(ident_hash h, struct router_info * ri, void * u)
{
  ident_hash ih;
  struct netdb_process_ctx * ctx = (struct netdb_process_ctx * ) u;
  if(router_info_verify(ri)) {
    router_info_hash(ri, &ih);
    if (memcmp(h, ih, sizeof(ident_hash))) {
      ctx->hashfail ++;
    } else {
      ctx->valid ++;
      router_info_iter_addrs(ri, print_i2p_addr, NULL);
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
  i2p_log_set_level(L_DEBUG);
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
  struct netdb_process_ctx p_ctx = {0, 0, 0};
  
  i2p_netdb_for_each(db, process_netdb_entry, &p_ctx);
  i2p_info(LOG_MAIN, "valid %lu", p_ctx.valid );
  i2p_info(LOG_MAIN, "invalid %lu", p_ctx.invalid);
  i2p_info(LOG_MAIN, "hashfail %lu", p_ctx.hashfail);

  struct netdb_find_ctx f_ctx = {db, 0, 0};
  
  i2p_netdb_for_each(db, test_find_netdb_entry, &f_ctx);
  i2p_info(LOG_MAIN, "lookup hit %lu", f_ctx.hit);
  if (f_ctx.miss > 0) {
    i2p_error(LOG_MAIN, "lookup miss %lu", f_ctx.miss);
  }
  i2p_netdb_free(&db);
}
