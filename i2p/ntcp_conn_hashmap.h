/*
  ntcp_conn_hashmap.h -- i2p ntcp connection hashmap implementation based off Janne Kulama's
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

#ifndef _I2PD_NTCP_CONN_HASHMAP_H_
#define _I2PD_NTCP_CONN_HASHMAP_H_

#include <i2pd/datatypes.h>
#include <i2pd/ntcp.h>

struct ntcp_conn_hashmap;

void ntcp_conn_hashmap_init(struct ntcp_conn_hashmap **map);
void ntcp_conn_hashmap_free(struct ntcp_conn_hashmap **map);

int ntcp_conn_hashmap_get(struct ntcp_conn_hashmap *map, ident_hash key, struct ntcp_conn ** c);

int ntcp_conn_hashmap_insert(struct ntcp_conn_hashmap *map, struct ntcp_conn * c);

/** @brief remove connection by ident hash, returns 1 if removed otherwise return 0 */
int ntcp_conn_hashmap_remove(struct ntcp_conn_hashmap *map, ident_hash key);

typedef void (*ntcp_conn_hashmap_iterator)(ident_hash, struct ntcp_conn *, void *);

/** @brief iterate over every item in hashmap */
void ntcp_conn_hashmap_for_each(struct ntcp_conn_hashmap *map, ntcp_conn_hashmap_iterator i, void * u);

#endif
