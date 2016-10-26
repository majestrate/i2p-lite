#include <i2pd/netdb.h>
#include <i2pd/memory.h>
#include <i2pd/util.h>
#include <i2pd/encoding.h>
#include <i2pd/log.h>
#include <i2pd/ri.h>

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

// compare netdb trees
int compare_netdb_entries(netdb_entry * left, netdb_entry * right)
{
  return memcmp(left->ident, right->ident, sizeof(ident_hash));
}

struct i2p_netdb
{
  netdb_entry * data;
  size_t sz;
  size_t cap;
  i2p_filename rootdir;
};

void i2p_netdb_ensure_capacity(struct i2p_netdb * db)
{
  if(!db->data) {
    // initial state, emtpy
    db->data = mallocx(sizeof(netdb_entry) * db->cap, MALLOCX_ZERO);
  } else if(db->sz == db->cap) {
    // full
    size_t newsize = db->cap * 2;
    db->data = realloc(db->data, newsize * sizeof(netdb_entry));
    db->sz = newsize;
  }
}

/** @brief context for writing a netdb entry */
struct netdb_write_ctx
{
  int result;
  struct i2p_netdb * db;
};

/** open file for entry given its ident hash */
FILE * netdb_open_file(struct i2p_netdb * db, ident_hash * ident, const char * mode)
{
  FILE * f = NULL;
  char * fpath = NULL;
  char buf[128] = {0};
  char skipdir[4] = {0};
  char base32[120] = {0};
  
  i2p_base32_encode(base32, sizeof(base32), *ident, sizeof(ident_hash));

  if(snprintf(skipdir, sizeof(skipdir), "r%c", base32[0]) == -1)
    return f;
    
  if(snprintf(buf, sizeof(buf), "routerInfo-r%s.dat", base32) == -1)
    return f;
  
  fpath = path_join(db->rootdir, skipdir, buf, 0);
  f = fopen(fpath, mode);
  free(fpath);
}

void netdb_write_entry(netdb_entry * e, void * user)
{
  struct netdb_write_ctx * ctx = (struct netdb_write_ctx*) user;
  if(ctx->result) {
    FILE * f = netdb_open_file(ctx->db, &e->ident, "w");
    if(f) {
      ctx->result = router_info_write(e->ri, f);
      fclose(f);
    } else ctx->result = 0; // fail
  }
}

int i2p_netdb_flush_to_disk(struct i2p_netdb * db)
{
  struct netdb_write_ctx c;
  // starts out okay
  c.result = 1;
  c.db = db;
  i2p_netdb_for_each(db, netdb_write_entry, &c);
  return c.result;
}

struct netdb_read_ctx
{
  size_t failed;
  size_t loaded;
  struct i2p_netdb * db;
  char * current_dir;
};

void netdb_read_file(char * filename, void * c)
{
  struct netdb_read_ctx * ctx = (struct netdb_read_ctx*) c;
  char * fpath = path_join(ctx->current_dir, filename, 0);
  FILE * f = fopen(fpath, "rb");
  if(f) {
    // obtain pointer to next entry 
    netdb_entry * e;
    i2p_netdb_ensure_capacity(ctx->db);
    e = ctx->db->data + (ctx->db->sz * sizeof(netdb_entry));
    // intialize entry
    router_info_new(&e->ri);
    // load entry
    if(router_info_load(e->ri, f)) {
      // hash it
      router_info_calculate_hash(e->ri, &e->ident);
      // we added it :-D
      ctx->db->sz ++;
    } else {
      // bad entry
      ctx->failed ++;
      i2p_warn(LOG_NETDB, "bad netdb entry %s", fpath);
      router_info_free(&e->ri);
    }
    fclose(f);
    
  } else {
    ctx->failed ++;
  }
  free(fpath);
}

void netdb_load_skiplist_subdir(char * dir, void * c)
{
  struct netdb_read_ctx * ctx = (struct netdb_read_ctx*) c;
  char * subdir = path_join(ctx->db->rootdir, dir, 0);

  i2p_debug(LOG_NETDB, "load skiplist subdir %s", subdir);
  
  ctx->current_dir = subdir;
  iterate_all_files(subdir, netdb_read_file, c);
  free(subdir);
}

static int netdb_compare(const void * a, const void * b)
{
  return compare_netdb_entries((netdb_entry*)a, (netdb_entry*)b);
}

// sort netdb entries
void netdb_sort(struct i2p_netdb * db)
{
  qsort(db->data, db->sz, sizeof(netdb_entry), netdb_compare);
}

int i2p_netdb_find_router_info(struct i2p_netdb * db, ident_hash * ident, struct router_info ** ri)
{
  netdb_entry k;
  memcpy(k.ident, *ident, sizeof(ident_hash));
  
  netdb_entry * e = (netdb_entry *) bsearch(&k, db->data, db->sz, sizeof(netdb_entry), netdb_compare);
  if(!e) {
    // not found in memory, try loading
    FILE * f = netdb_open_file(db, ident, "rb");
    if(!f) return 0; // not on disk or memory
    e = db->data + ( db->sz*sizeof(netdb_entry));
    router_info_new(&e->ri);
    if(!router_info_load(e->ri, f)) {
      // bad entry
      router_info_free(&e->ri);
      e = NULL;
    }
    // increase size
    db->sz ++;
    i2p_netdb_ensure_capacity(db);
    fclose(f);
  }
  if(e) {
    *ri = e->ri;
    return 1;
  }
  return 0;
}

int i2p_netdb_load_all(struct i2p_netdb * db)
{
  struct netdb_read_ctx c;
  c.loaded = 0;
  c.failed = 0;
  c.db = db;
  c.current_dir = NULL;
  iterate_all_dirs(db->rootdir, netdb_load_skiplist_subdir, db);
  i2p_info(LOG_NETDB, "loaded %lu netdb entries", c.loaded);
  netdb_sort(db);
  return 1;
}

void i2p_netdb_new(struct i2p_netdb ** db, struct i2p_netdb_config c)
{
  // alloc
  (*db) = mallocx(sizeof(struct i2p_netdb), MALLOCX_ZERO);
  memcpy((*db)->rootdir, c.rootdir, sizeof(i2p_filename));

  (*db)->cap = 128;
  i2p_netdb_ensure_capacity(*db);
}

void netdb_free_entry(netdb_entry * e, void * u)
{
  (void) u;
  if(e->ri) router_info_free(&e->ri);
  e->ri = NULL;
}

void i2p_netdb_free(struct i2p_netdb ** db)
{
  // free all entries
  i2p_netdb_for_each(*db, netdb_free_entry, NULL);
  // free data
  free((*db)->data);
  // free netdb
  free(*db);
  *db = NULL;
}

void i2p_netdb_for_each(struct i2p_netdb * db, netdb_itr i, void * user)
{
  size_t idx = 0;
  while(idx < db->sz) {
    i(db->data+(idx * sizeof(netdb_entry)), user);
    idx ++;
  }
}
