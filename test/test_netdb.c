#include <mnet/crypto.h>
#include <mnet/encoding.h>
#include <mnet/log.h>
#include <mnet/memory.h>
#include <mnet/netdb.h>
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
  struct mnet_netdb * db;
  size_t hit;
  size_t miss;
};

void test_find_netdb_entry(ident_hash h, struct router_info * drop, void * u)  
{
  (void) drop;
  ident_hash ih;
  struct netdb_find_ctx * ctx = (struct netdb_find_ctx *) u;
  struct router_info * ri = NULL;
  if(mnet_netdb_find_router_info(ctx->db, h, &ri)) {
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
  
void print_mnet_addr(struct router_info * ri, struct mnet_addr * addr, void * u)
{
  char * ident = router_info_base64_ident(ri);
  mnet_info(LOG_MAIN, "router info %s has %s address %s port %d ", ident, addr->style, addr->host, addr->port);
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
      router_info_iter_addrs(ri, print_mnet_addr, NULL);
    }
  } else {
    ctx->invalid ++;
  }
}

int main(int argc, char * argv[])
{
  if(argc != 2) {
    printf("usage: %s /path/to/nodes\n", argv[0]);
    return 1;
  }

  mnet_log_init();
  mnet_log_set_level(L_INFO);
  mnet_log_set_scope(LOG_ALL);

  struct mnet_crypto_config c = default_crypto_config;
  
  mnet_crypto_init(c);
  
  mnet_debug(LOG_MAIN, "starting netdb test");
  
  struct mnet_netdb * db = NULL;
  mnet_netdb_new(&db, argv[1]);

  int result;
  mnet_debug(LOG_MAIN, "ensure skiplist");
  result = mnet_netdb_ensure_skiplist(db);
  mnet_debug(LOG_MAIN, "load all");
  result = mnet_netdb_load_all(db);
  struct netdb_process_ctx p_ctx = {0, 0, 0};
  
  mnet_netdb_for_each(db, process_netdb_entry, &p_ctx);
  mnet_info(LOG_MAIN, "valid %lu", p_ctx.valid );
  mnet_info(LOG_MAIN, "invalid %lu", p_ctx.invalid);
  mnet_info(LOG_MAIN, "hashfail %lu", p_ctx.hashfail);

  struct netdb_find_ctx f_ctx = {db, 0, 0};
  
  mnet_netdb_for_each(db, test_find_netdb_entry, &f_ctx);
  mnet_info(LOG_MAIN, "lookup hit %lu", f_ctx.hit);
  if (f_ctx.miss > 0) {
    mnet_error(LOG_MAIN, "lookup miss %lu", f_ctx.miss);
  }
  mnet_netdb_free(&db);
}
