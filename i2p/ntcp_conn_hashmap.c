/*
  ntcp_conn_hashmap.c -- ntcp connection hashmap based of generic hash table by Janne Kulmala
 
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

#include "ntcp_conn_hashmap.h"
#include <i2pd/memory.h>
#include <stdlib.h>
#include <string.h>


#define BUCKETS 64

/** TODO: implement as B-tree */
struct ntcp_hash_node {
  ident_hash h;
  struct ntcp_conn * conn;
  struct ntcp_hash_node * next;
};

typedef size_t (*ntcp_conn_hash_func_t)(ident_hash key);
typedef int (*ntcp_conn_cmp_func_t)(struct ntcp_hash_node *node, ident_hash key);

struct ntcp_conn_hashmap {
  /* first entry is dummy entry, will not hold anything, acts as anchor for following nodes */
	struct ntcp_hash_node table[BUCKETS];
	ntcp_conn_hash_func_t hash;
	ntcp_conn_cmp_func_t cmp;
};

static size_t ntcp_conn_hashmap_hash_ident(ident_hash k)
{
  size_t sz = 0;
  memcpy(&sz, k, sizeof(size_t));
  return sz % BUCKETS;
}

static int ntcp_conn_hashmap_cmp_ident(struct ntcp_hash_node * node, ident_hash k)
{
  return memcmp(node->h, k, sizeof(ident_hash)) == 0;
}

/** @brief free hash node and return next hash node */
struct ntcp_hash_node * ntcp_hash_node_free(struct ntcp_hash_node * n)
{
  struct ntcp_hash_node * next = NULL;
  if(n) {
    next = n->next;
    free(n);
  }
  return next;
}

void ntcp_conn_hashmap_init(struct ntcp_conn_hashmap **map)
{
  *map = mallocx(sizeof(struct ntcp_conn_hashmap), MALLOCX_ZERO);
	(*map)->hash = ntcp_conn_hashmap_hash_ident;
	(*map)->cmp = ntcp_conn_hashmap_cmp_ident;
}

void ntcp_conn_hashmap_free(struct ntcp_conn_hashmap **map)
{  
  struct ntcp_hash_node * node = NULL;
  for(size_t idx = 0; idx < BUCKETS; idx++) {
    node = (*map)->table[idx].next;
    while((node = ntcp_hash_node_free(node))) continue;
  }
  free(*map);
  *map = NULL;
}

int ntcp_conn_hashmap_get(struct ntcp_conn_hashmap *map, ident_hash key, struct ntcp_conn ** conn)
{
  *conn = NULL;
  size_t slot = map->hash(key);
  struct ntcp_hash_node *node = map->table[slot].next;
	while (node) {
		if (map->cmp(node, key)) {
      *conn = node->conn;
			return 1;
    }
		node = node->next;
	}
	return 0;
}

int ntcp_conn_hashmap_insert(struct ntcp_conn_hashmap *map, struct ntcp_conn * conn)
{
  ident_hash h;
  ntcp_conn_get_ident_hash(conn, &h);
  size_t slot = map->hash(h);
  struct ntcp_hash_node ** node = &map->table[slot].next;
  struct ntcp_hash_node ** prev = NULL;
  while(*node) {
    if(map->cmp(*node, h)) {
      // duplicate
      return 0;
    }
    prev = node;
    node = &(*node)->next;
  }
  *node = mallocx(sizeof(struct ntcp_hash_node), MALLOCX_ZERO);
  memcpy(&(*node)->h, &h, sizeof(ident_hash));
  (*node)->conn = conn;
  if(prev)
    (*prev)->next = *node;
	return 1;
}

int ntcp_conn_hashmap_remove(struct ntcp_conn_hashmap *map, ident_hash k)
{
	size_t slot = map->hash(k);
  struct ntcp_hash_node ** node = &map->table[slot].next;
  struct ntcp_hash_node ** prev = NULL;
  while(*node) {
    if(map->cmp(*node, k)) {
      // found a match
      if(prev) (*prev)->next = (*node)->next;
      // free node
      ntcp_hash_node_free(*node);
      return 1;
    }
    prev = node;
    node = &(*node)->next;
  }
  
	return 0;
}

void ntcp_conn_hashmap_for_each(struct ntcp_conn_hashmap *map, ntcp_conn_hashmap_iterator i, void * u)
{
  struct ntcp_hash_node ** node;
  size_t idx = BUCKETS;
  while(--idx) {
    node = &map->table[idx].next;
    while(*node) {
      i((*node)->h, (*node)->conn, u);
      node = &(*node)->next;
    }
  }
}
