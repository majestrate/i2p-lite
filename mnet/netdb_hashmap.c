/*
  netdb_hashmap.c -- netdb hashmap based of generic hash table by Janne Kulmala
 
  Copyright (c) 2011, Janne Kulmala <janne.t.kulmala@tut.fi>.
  All rights reserved.
  
  Redistribution and use in source and binary forms, with or without 
  modification, are permitted provided that the following conditions are 
  met:
  
  * Redistributions of source code must retain the above copyright 
  notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright 
  notice, this list of conditions and the following disclaimer in the 
  documentation and/or other materials provided with the distribution.
  * Names of its contributors may not be used to endorse or promote 
  products derived from this software without specific prior written 
  permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS 
  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A 
  PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "netdb_hashmap.h"
#include <mnet/memory.h>

#define BUCKETS 128

/** TODO: implement as B-tree */
struct netdb_hash_node {
  ident_hash h;
  struct router_info * ri;
  struct netdb_hash_node * next;
};

typedef size_t (*netdb_hash_func_t)(ident_hash key);
typedef int (*netdb_cmp_func_t)(struct netdb_hash_node *node, ident_hash key);

struct netdb_hashmap {
  /* first entry is dummy entry, will not hold anything, acts as anchor for following nodes */
	struct netdb_hash_node table[BUCKETS];
	netdb_hash_func_t hash;
	netdb_cmp_func_t cmp;
  size_t sz;
};

static size_t netdb_hashmap_hash_ident(ident_hash k)
{
  size_t sz = 0;
  memcpy(&sz, k, sizeof(size_t));
  return sz % BUCKETS;
}

static int netdb_hashmap_cmp_ident(struct netdb_hash_node * node, ident_hash k)
{
  return memcmp(node->h, k, sizeof(ident_hash)) == 0;
}

/** @brief free hash node and return next hash node */
struct netdb_hash_node * netdb_hash_node_free(struct netdb_hash_node * n)
{
  struct netdb_hash_node * next = NULL;
  if(n) {
    if(n->ri) router_info_free(&n->ri);
    next = n->next;
    free(n);
  }
  return next;
}

void netdb_hashmap_init(struct netdb_hashmap **map)
{
  *map = xmalloc(sizeof(struct netdb_hashmap));
	(*map)->hash = netdb_hashmap_hash_ident;
	(*map)->cmp = netdb_hashmap_cmp_ident;
}

void netdb_hashmap_free(struct netdb_hashmap **map)
{  
  struct netdb_hash_node * node = NULL;
  for(size_t idx = 0; idx < BUCKETS; idx++) {
    node = (*map)->table[idx].next;
    while((node = netdb_hash_node_free(node))) continue;
  }
  free(*map);
  *map = NULL;
}

int netdb_hashmap_get(struct netdb_hashmap *map, ident_hash key, struct router_info ** ri)
{
  *ri = NULL;
  size_t slot = map->hash(key);
  struct netdb_hash_node *node = map->table[slot].next;
	while (node) {
		if (map->cmp(node, key)) {
      *ri = node->ri;
			return 1;
    }
		node = node->next;
	}
	return 0;
}

int netdb_hashmap_insert(struct netdb_hashmap *map, struct router_info * ri)
{
  ident_hash h;
  router_info_hash(ri, &h);
  size_t slot = map->hash(h);
  struct netdb_hash_node ** node = &map->table[slot].next;
  struct netdb_hash_node ** prev = NULL;
  while(*node) {
    if(map->cmp(*node, h)) {
      // duplicate
      return 0;
    }
    prev = node;
    node = &(*node)->next;
  }
  *node = mallocx(sizeof(struct netdb_hash_node), MALLOCX_ZERO);
  memcpy(&(*node)->h, &h, sizeof(ident_hash));
  (*node)->ri = ri;
  if(prev)
    (*prev)->next = *node;
  map->sz ++;
	return 1;
}

int netdb_hashmap_remove(struct netdb_hashmap *map, ident_hash k)
{
	size_t slot = map->hash(k);
  struct netdb_hash_node ** node = &map->table[slot].next;
  struct netdb_hash_node ** prev = NULL;
  while(*node) {
    if(map->cmp(*node, k)) {
      // found a match
      if(prev) (*prev)->next = (*node)->next;
      // free node
      netdb_hash_node_free(*node);
      map->sz --;
      return 1;
    }
    prev = node;
    node = &(*node)->next;
  }
  
	return 0;
}

void netdb_hashmap_for_each(struct netdb_hashmap *map, netdb_iterator i, void * u)
{
  struct netdb_hash_node ** node;
  size_t idx = BUCKETS;
  while(--idx) {
    node = &map->table[idx].next;
    while(*node) {
      i((*node)->h, (*node)->ri, u);
      node = &(*node)->next;
    }
  }
}

size_t netdb_hashmap_size(struct netdb_hashmap * map)
{
  return map->sz;
}
