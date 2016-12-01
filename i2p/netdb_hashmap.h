/*
  netdb_hashmap.h -- i2p netdb hashmap implementation based off Janne Kulama's
                     generic hashmap implementation
 
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

#ifndef _I2PD_NETDB_HASHMAP_H_
#define _I2PD_NETDB_HASHMAP_H_

#include <i2pd/datatypes.h>
#include <i2pd/netdb.h>
#include <i2pd/ri.h>

struct netdb_hashmap;

void netdb_hashmap_init(struct netdb_hashmap **map);
void netdb_hashmap_free(struct netdb_hashmap **map);

/** @brief get router info by ident hash, returns 1 if found otherwise returns 0 */
int netdb_hashmap_get(struct netdb_hashmap *map, ident_hash key, struct router_info ** ri);

/** @brief insert router info into hashmap, returns 0 if already in hashmap otherwise returns 1 */
int netdb_hashmap_insert(struct netdb_hashmap *map, struct router_info *ri);

/** @brief remove rotuer info by ident hash, returns 1 if removed otherwise return 0 */
int netdb_hashmap_remove(struct netdb_hashmap *map, ident_hash key);

/** @brief iterate over every item in hashmap */
void netdb_hashmap_for_each(struct netdb_hashmap *map, netdb_iterator i, void * u);

/** @brief return how many router infos we have loaded */
size_t netdb_hashmap_size(struct netdb_hashmap * map);

#endif
