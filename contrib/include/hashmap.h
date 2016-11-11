/*
  hashmap.h -- generic hash table
 
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

#ifndef _HASHMAP_H_
#define _HASHMAP_H_

#include <string.h>

struct hash_node {
	size_t hash;
	struct hash_node *next;
};

typedef size_t (*hash_func_t)(void *key);
typedef int (*cmp_func_t)(struct hash_node *node, void *key);

struct hashmap {
	struct hash_node **table;
	size_t len, count;
	hash_func_t hash;
	cmp_func_t cmp;
};

void hashmap_init(struct hashmap *map, hash_func_t hash, cmp_func_t cmp);
void hashmap_free(struct hashmap *map);
struct hash_node *hashmap_get(struct hashmap *map, void *key);
int hashmap_insert(struct hashmap *map, struct hash_node *node, void *key);
struct hash_node *hashmap_remove(struct hashmap *map, void *key);

#endif
