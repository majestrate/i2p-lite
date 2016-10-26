#include <i2pd/netdb.h>
#include <i2pd/memory.h>
#include <i2pd/util.h>
#include <i2pd/encoding.h>
#include <i2pd/log.h>
#include <i2pd/ri.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct netdb_tree
{
  netdb_entry entry;
  struct netdb_tree ** left;
  struct netdb_tree ** right;
};

// compare netdb entries
int compare_netdb_entry(struct netdb_tree * left, struct netdb_tree * right)
{
  if(!left) return -1;
  if(!right) return 1;
  return memcmp(left->entry.ident, right->entry.ident, sizeof(ident_hash));
}


/** @brief heapify a netdb tree */
void netdb_tree_heapify(struct netdb_tree ** root)
{
  
}

void netdb_tree_init(struct netdb_tree ** tree)
{
  (*tree) = mallocx(sizeof(struct netdb_tree *), MALLOCX_ZERO);
}

void netdb_tree_free(struct netdb_tree ** tree)
{
  if((*tree)->left) netdb_tree_free((*tree)->left);
  if((*tree)->right) netdb_tree_free((*tree)->right);
  free(*tree);
  *tree = NULL;
}

   

/** @brief recursively insert an entry into the netdb tree blindly, does not rebalance */
void netdb_tree_insert(struct netdb_tree * root, netdb_entry * entry)
{
  if(!root) {
    // empty tree
    root = mallocx(sizeof(struct netdb_tree), MALLOCX_ZERO);
    memcpy(&root->entry, entry, sizeof(netdb_entry));
  } else {
    if (rand() % 2)
      netdb_tree_insert(*root->left, entry);
    else
      netdb_tree_insert(*root->right, entry);
  }
}

struct i2p_netdb
{
  struct netdb_tree ** entries;
  i2p_filename rootdir;
};

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
  netdb_entry entry;
  struct netdb_read_ctx * ctx = (struct netdb_read_ctx*) c;
  char * fpath = path_join(ctx->current_dir, filename, 0);
  FILE * f = fopen(fpath, "rb");
  if(f) {
    router_info_new(&entry.ri);
    if(router_info_load(entry.ri, f)) {
      // hash it
      router_info_calculate_hash(entry.ri, &entry.ident);
      // insert it
      netdb_tree_insert(*ctx->db->entries, &entry);
    } else {
      // bad entry
      ctx->failed ++;
      i2p_warn(LOG_NETDB, "bad netdb entry %s", fpath);
      router_info_free(&entry.ri);
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
  // rebalance netdb tree for each skiplist subdir
  netdb_tree_heapify(ctx->db->entries);
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
  return 1;
}

void i2p_netdb_new(struct i2p_netdb ** db, struct i2p_netdb_config c)
{
  // alloc
  (*db) = mallocx(sizeof(struct i2p_netdb), MALLOCX_ZERO);
  memcpy((*db)->rootdir, c.rootdir, sizeof(i2p_filename));
  // init tree
  netdb_tree_init((*db)->entries);
}

void i2p_netdb_free(struct i2p_netdb ** db)
{
  // free tree
  netdb_tree_free((*db)->entries);
  // free netdb
  free(*db);
  *db = NULL;
}
