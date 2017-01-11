#include <mnet/tunnels.h>
#include <mnet/memory.h>

#define TC_BUCKETS 256

struct mnet_garlic_tc_hashnode
{
  tunnel_id_t tid;
  struct mnet_garlic_tunnel * tunnel;
  struct mnet_garlic_tc_hashnode * next;
};

/** @brief hashmap for tunnel context */
struct mnet_garlic_tc_hashmap
{
  struct mnet_garlic_tc_hashnode nodes[TC_BUCKETS];
  size_t sz;
};

#define mnet_garlic_tc_hashnode_hash(tid) (tid % TC_BUCKETS)

static struct mnet_garlic_tc_hashnode * mnet_garlic_tc_hashnode_free(struct mnet_garlic_tc_hashnode * n)
{
  struct mnet_garlic_tc_hashnode * next = NULL;
  if(n) {
    if(n->tunnel) mnet_garlic_tunnel_free(&n->tunnel);
    next = n->next;
    free(n);
  }
  return next;
}

static void mnet_garlic_tc_hashmap_init(struct mnet_garlic_tc_hashmap ** h)
{
  *h = xmalloc(sizeof(struct mnet_garlic_tc_hashmap));
}

static void mnet_garlic_tc_hashmap_free(struct mnet_garlic_tc_hashmap ** h)
{
  struct mnet_garlic_tc_hashnode * node = NULL;
  for(size_t idx = 0; idx < TC_BUCKETS; idx++ ) {
    node = (*h)->nodes[idx].next;
    while((node = mnet_garlic_tc_hashnode_free(node))) continue;
  }
  free(*h);
  *h = NULL;
}

static int mnet_garlic_tc_hashmap_get(struct mnet_garlic_tc_hashmap * map, tunnel_id_t tid, struct mnet_garlic_tunnel ** tunnel)
{
  *tunnel = NULL;
  size_t slot = mnet_garlic_tc_hashnode_hash(tid);
  struct mnet_garlic_tc_hashnode * node = map->nodes[slot].next;
  while(node) {
    if(node->tid == tid) {
      *tunnel = node->tunnel;
      return 1;
    }
    node = node->next;
  }
  return 0;
}

static int mnet_garlic_tc_hashmap_put(struct mnet_garlic_tc_hashmap * map, tunnel_id_t tid, struct mnet_garlic_tunnel * tunnel)
{
  size_t slot = mnet_garlic_tc_hashnode_hash(tid);
  struct mnet_garlic_tc_hashnode * node = &map->nodes[slot];
  while(node->next) {
    if(node->tid == tid) return 0; // we already have it
    node = node->next;
  }
  node->next = xmalloc(sizeof(struct mnet_garlic_tc_hashnode));
  node->next->tid = tid;
  node->next->tunnel = tunnel;
  return 1;
}

static int mnet_garlic_tc_hashmap_remove(struct mnet_garlic_tc_hashmap * map, tunnel_id_t tid)
{
  size_t slot = mnet_garlic_tc_hashnode_hash(tid);
  struct mnet_garlic_tc_hashnode * node = map->nodes[slot].next;
  while(node) {
    if (node->tid == tid) {
      
    }
    node = node->next;
  }
  return 0;
}

struct mnet_garlic_tunnel_context
{
  struct mnet_garlic_message_router * i2np;
  struct router_context * router;
};


static void mnet_garlic_tunnel_context_dispatch(void * t, struct mnet_garlic_msg * msg, ident_hash from)
{
  struct mnet_garlic_tunnel_context * ctx = (struct mnet_garlic_tunnel_context *) t;
  
}

void mnet_garlic_tunnel_context_new(struct router_context * router, struct mnet_garlic_tunnel_context ** t)
{
  *t = xmalloc(sizeof(struct mnet_garlic_tunnel_context));
  (*t)->i2np = xmalloc(sizeof(struct mnet_garlic_message_router));
  (*t)->i2np->impl = *t;
  (*t)->router = router;
}

void mnet_garlic_tunnel_context_free(struct mnet_garlic_tunnel_context ** t)
{
  free((*t)->i2np);
  free(*t);
  *t = NULL;
}

struct mnet_garlic_message_router * mnet_garlic_tunnel_context_message_router(struct mnet_garlic_tunnel_context * t)
{
  return t->i2np;
}

void mnet_garlic_tunnel_context_attach(struct mnet_garlic_tunnel_context * t, struct router_context * r)
{
  t->router = r;
}
