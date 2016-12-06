#include <i2pd/tunnels.h>
#include <i2pd/memory.h>

#define TC_BUCKETS 256

struct i2np_tc_hashnode
{
  tunnel_id_t tid;
  struct i2np_tunnel * tunnel;
  struct i2np_tc_hashnode * next;
};

/** @brief hashmap for tunnel context */
struct i2np_tc_hashmap
{
  struct i2np_tc_hashnode nodes[TC_BUCKETS];
  size_t sz;
};

#define i2np_tc_hashnode_hash(tid) (tid % TC_BUCKETS)

static struct i2np_tc_hashnode * i2np_tc_hashnode_free(struct i2np_tc_hashnode * n)
{
  struct i2np_tc_hashnode * next = NULL;
  if(n) {
    if(n->tunnel) i2np_tunnel_free(&n->tunnel);
    next = n->next;
    free(n);
  }
  return next;
}
                                                
static void i2np_tc_hashmap_init(struct i2np_tc_hashmap ** h)
{
  *h = xmalloc(sizeof(struct i2np_tc_hashmap));
}

static void i2np_tc_hashmap_free(struct i2np_tc_hashmap ** h)
{
  struct i2np_tc_hashnode * node = NULL;
  for(size_t idx = 0; idx < TC_BUCKETS; idx++ ) {
    node = (*h)->nodes[idx].next;
    while((node = i2np_tc_hashnode_free(node))) continue;
  }
  free(*h);
  *h = NULL;
}

static int i2np_tc_hashmap_get(struct i2np_tc_hashmap * map, tunnel_id_t tid, struct i2np_tunnel ** tunnel)
{
  *tunnel = NULL;
  size_t slot = i2np_tc_hashnode_hash(tid);
  struct i2np_tc_hashnode * node = map->nodes[slot].next;
  while(node) {
    if(node->tid == tid) {
      *tunnel = node->tunnel;
      return 1;
    }
    node = node->next;
  }
  return 0;
}

static int i2np_tc_hashmap_put(struct i2np_tc_hashmap * map, tunnel_id_t tid, struct i2np_tunnel * tunnel)
{
  size_t slot = i2np_tc_hashnode_hash(tid);
  struct i2np_tc_hashnode * node = &map->nodes[slot];
  while(node->next) {
    if(node->tid == tid) return 0; // we already have it
    node = node->next;
  }
  node->next = xmalloc(sizeof(struct i2np_tc_hashnode));
  node->next->tid = tid;
  node->next->tunnel = tunnel;
  return 1;
}

static int i2np_tc_hashmap_remove(struct i2np_tc_hashmap * map, tunnel_id_t tid)
{
  size_t slot = i2np_tc_hashnode_hash(tid);
  struct i2np_tc_hashnode * node = map->nodes[slot].next;
  while(node) {
    if (node->tid == tid) {
      
    }
    node = node->next;
  }
  return 0;
}

struct i2np_tunnel_context
{
  struct i2np_message_router * i2np;
  struct router_context * router;
};


static void i2np_tunnel_context_dispatch(void * t, struct i2np_msg * msg, ident_hash from)
{
  struct i2np_tunnel_context * ctx = (struct i2np_tunnel_context *) t;
  
}

void i2np_tunnel_context_new(struct router_context * router, struct i2np_tunnel_context ** t)
{
  *t = xmalloc(sizeof(struct i2np_tunnel_context));
  (*t)->i2np = xmalloc(sizeof(struct i2np_message_router));
  (*t)->i2np->impl = *t;
  (*t)->i2np->dispatch = i2np_tunnel_context_dispatch;
  (*t)->router = router;
}

void i2np_tunnel_context_free(struct i2np_tunnel_context ** t)
{
  free((*t)->i2np);
  free(*t);
  *t = NULL;
}

struct i2np_message_router * i2np_tunnel_context_message_router(struct i2np_tunnel_context * t)
{
  return t->i2np;
}

void i2np_tunnel_context_attach(struct i2np_tunnel_context * t, struct router_context * r)
{
  t->router = r;
}
