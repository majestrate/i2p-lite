#include <i2pd/netdb.h>
#include <i2pd/memory.h>
#include <i2pd/util.h>
#include <i2pd/encoding.h>
#include <i2pd/log.h>
#include <i2pd/ri.h>
#include "netdb_hashmap.h"

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

struct i2p_netdb
{
  struct netdb_hashmap * map;
  char * rootdir;
};

/** @brief context for writing a netdb entry */
struct netdb_write_ctx
{
  int result;
  struct i2p_netdb * db;
};

/** open file for entry given its ident hash */
FILE * netdb_open_file(struct i2p_netdb * db, ident_hash ident, const char * mode)
{
  FILE * f = NULL;
  char * fpath = NULL;
  char buf[128] = {0};
  char skipdir[4] = {0};
  char base64[128] = {0};
  
  i2p_base64_encode(ident, sizeof(ident_hash), base64, sizeof(base64));

  if(snprintf(skipdir, sizeof(skipdir), "r%c", base64[0]) == -1)
    return f;
    
  if(snprintf(buf, sizeof(buf), "routerInfo-r%s.dat", base64) == -1)
    return f;
  
  fpath = path_join(db->rootdir, skipdir, buf, 0);
  f = fopen(fpath, mode);
  free(fpath);
  return f;
}

void netdb_write_entry(ident_hash h, struct router_info * ri, void * user)
{
  struct netdb_write_ctx * ctx = (struct netdb_write_ctx*) user;
  if(ctx->result) {
    FILE * f = netdb_open_file(ctx->db, h, "w+");
    if(f) {
      ctx->result = router_info_write(ri, f);
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
  FILE * f = fopen(filename, "r");
  if(f) {
    struct router_info * ri;
    // intialize entry
    router_info_new(&ri);
    // load entry
    if(router_info_load(ri, f)) {
      // add it to hashmap
      netdb_hashmap_insert(ctx->db->map, ri);
      ctx->loaded ++;
    } else {
      // bad entry
      ctx->failed ++;
      i2p_warn(LOG_NETDB, "bad netdb entry %s", filename);
      router_info_free(&ri);
    }
    fclose(f);
  } else {
    ctx->failed ++;
  }
}

void netdb_load_skiplist_subdir(char * dir, void * c)
{
  struct netdb_read_ctx * ctx = (struct netdb_read_ctx*) c;
  i2p_debug(LOG_NETDB, "load skiplist subdir %s", dir);
  iterate_all_files(dir, netdb_read_file, c);
}

int i2p_netdb_put_router_info(struct i2p_netdb * db, struct router_info * ri)
{
  // TODO: implement
  return 0;
}

int i2p_netdb_find_router_info(struct i2p_netdb * db, ident_hash ident, struct router_info ** ri)
{
  return netdb_hashmap_get(db->map, ident, ri);
}

int i2p_netdb_load_all(struct i2p_netdb * db)
{
  struct netdb_read_ctx c;
  c.loaded = 0;
  c.failed = 0;
  c.db = db;
  c.current_dir = NULL;
  i2p_info(LOG_NETDB, "loading netdb from %s", db->rootdir);
  iterate_all_dirs(db->rootdir, netdb_load_skiplist_subdir, &c);
  i2p_info(LOG_NETDB, "loaded %lu netdb entries", c.loaded);
  return c.loaded;
}

void i2p_netdb_new(struct i2p_netdb ** db, const char * dir)
{
  // alloc
  (*db) = mallocx(sizeof(struct i2p_netdb), MALLOCX_ZERO);
  (*db)->rootdir = strdup(dir);

  // init hashmap
  netdb_hashmap_init(&(*db)->map);
}

void i2p_netdb_free(struct i2p_netdb ** db)
{
  // free hashmap
  netdb_hashmap_free(&(*db)->map);
  // free path
  free((*db)->rootdir);
  // free netdb
  free(*db);
  *db = NULL;
}

void i2p_netdb_for_each(struct i2p_netdb * db, netdb_iterator i, void * user)
{
  netdb_hashmap_for_each(db->map, i, user);
}

size_t i2p_netdb_loaded_peer_count(struct i2p_netdb * db)
{
  return netdb_hashmap_size(db->map);
}

int i2p_netdb_ensure_skiplist(struct i2p_netdb * db)
{
  DIR * d = opendir(db->rootdir);
  if (d) closedir(d);
  else {
    i2p_info(LOG_NETDB, "create netdb base directory %s", db->rootdir);
    if ( mkdir(db->rootdir, 0700) == -1) return 0;
  }
  int ret = 1;
  char f[3] = { 0 };
  f[0] = 'r';
  const char * c = I2P_BASE64_CHARS;
  while(*c && ret) {
    f[1] = *c;
    char * path = path_join(db->rootdir, f, 0);
    d = opendir(path);
    if(d) closedir(d);
    else if (errno == ENOENT) {
      if ( mkdir(path, 0700) == -1) ret = 0;
    }
    c++ ;
    free(path);
  }
  return ret;
}
