
/**
 * Copyright (c) 2014, Willem-Hendrik Thiart
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file. 
 *
 * @file
 * @brief Read bencoded data
 * @author  Willem Thiart himself@willemthiart.com
 * @version 0.1
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <i2pd/memory.h>

#include "bencode.h"

bencode_t* bencode_new(
        int expected_depth,
        bencode_callbacks_t* cb,
        void* udata)
{
    bencode_t* me;

    me = xcalloc(1, sizeof(bencode_t));
    bencode_set_callbacks(me, cb);
    me->udata = udata;
    me->nframes = expected_depth;
    me->stk = xcalloc(10 + expected_depth, sizeof(bencode_frame_t));
    return me;
}

void bencode_init(bencode_t* me)
{
    memset(me,0,sizeof(bencode_t));
}

static bencode_frame_t* __push_stack(bencode_t* me)
{
    if (me->nframes <= me->d)
    {
        assert(0);
        return NULL;
    }

    me->d++;
    //memset(&me->stk[me->d], 0, sizeof(bencode_frame_t));

    bencode_frame_t* s = &me->stk[me->d];

    s->pos = 0;
    s->intval = 0;
    s->len = 0;
    s->type = 0;

    if (0 == s->sv_size)
    {
        s->sv_size = 20;
        s->strval = xmalloc(s->sv_size);
    }

    if (0 == s->k_size)
    {
        s->k_size = 20;
        s->key = xmalloc(s->k_size);
    }
    
    return &me->stk[me->d];
}

static bencode_frame_t* __pop_stack(bencode_t* me)
{
    bencode_frame_t* f;

    f = &me->stk[me->d];

    switch(f->type)
    {
        case BENCODE_TOK_LIST:
            if (me->cb.list_leave)
                me->cb.list_leave(me, f->key);
            break;
        case BENCODE_TOK_DICT:
            if (me->cb.dict_leave)
                me->cb.dict_leave(me, f->key);
            break;
    }

    if (me->d == 0)
        return NULL;

    f = &me->stk[--me->d];

    switch(f->type)
    {
        case BENCODE_TOK_LIST:
            if (me->cb.list_next)
                me->cb.list_next(me);
            break;
        case BENCODE_TOK_DICT:
            if (me->cb.dict_next)
                me->cb.dict_next(me);
            break;
    }

    return f;
}

static int __parse_digit(const int current_value, const char c)
{
    return (c - '0') + current_value * 10;
}

static void __start_int(bencode_frame_t* f)
{
    f->type = BENCODE_TOK_INT;
    f->pos = 0;
}

static bencode_frame_t* __start_dict(bencode_t* me, bencode_frame_t* f)
{
    f->type = BENCODE_TOK_DICT;
    f->pos = 0;
    if (me->cb.dict_enter)
        me->cb.dict_enter(me, f->key);

    /* key/value */
    f = __push_stack(me);
    f->type = BENCODE_TOK_DICT_KEYLEN;
    return f;
}

static void __start_list(bencode_t* me, bencode_frame_t* f)
{
    f->type = BENCODE_TOK_LIST;
    f->pos = 0;
    if (me->cb.list_enter)
        me->cb.list_enter(me, f->key);
}

static void __start_str(bencode_frame_t* f)
{
    f->type = BENCODE_TOK_STR_LEN;
    f->pos = 0;
}

static int __process_tok(
        bencode_t* me,
        const char** buf,
        unsigned int *len)
{
    bencode_frame_t* f = &me->stk[me->d];

    switch (f->type)
    {
    case BENCODE_TOK_LIST:
        switch (**buf)
        {
        /* end of list/dict */
        case 'e':
            f = __pop_stack(me);
            break;
        case 'i':
            f = __push_stack(me);
            __start_int(f);
            break;
        case 'd':
            f = __push_stack(me);
            f = __start_dict(me,f);
            break;
        case 'l':
            f = __push_stack(me);
            __start_list(me,f);
            break;
        /* calculating length of string */
        default:
            if (isdigit(**buf))
            {
                f = __push_stack(me);
                __start_str(f);
                f->len = __parse_digit(f->len, **buf);
            }
            else
            {
                return 0;
            }
            break;
        }
        break;

    case BENCODE_TOK_DICT_VAL:
        /* drop through */
    case BENCODE_TOK_NONE:
        switch (**buf)
        {
        case 'i':
            __start_int(f);
            break;
        case 'd':
            f = __start_dict(me,f);
            break;
        case 'l':
            __start_list(me,f);
            break;
        /* calculating length of string */
        default:
            if (isdigit(**buf))
            {
                __start_str(f);
                f->len = __parse_digit(f->len, **buf);
            }
            else
            {
                return 0;
            }
            break;

        }
        break;

    case BENCODE_TOK_INT:
        if ('e' == **buf)
        {
            me->cb.hit_int(me, f->key, f->intval);
            f = __pop_stack(me);
        }
        else if (isdigit(**buf))
        {
            f->intval = __parse_digit(f->intval, **buf);
        }
        else
        {
            assert(0);
        }
        break;

    case BENCODE_TOK_STR_LEN:
        if (':' == **buf)
        {
            if (0 == f->len)
            {
                me->cb.hit_str(me, f->key, 0, NULL, 0);
                f = __pop_stack(me);
            }
            else
            {
                f->type = BENCODE_TOK_STR;
                f->pos = 0;
            }
        }
        else if (isdigit(**buf))
        {
            f->len = __parse_digit(f->len, **buf);
        }
        else
        {
            assert(0);
        }

        break;
    case BENCODE_TOK_STR:

        /* resize string
         * +1 incase we also need to count for '\0' terminator */
        if (f->sv_size <= f->pos + 1)
        {
            f->sv_size = 4 + f->sv_size * 2;
            f->strval = realloc(f->strval,f->sv_size);
        }
        f->strval[f->pos++] = **buf;

        if (f->len == f->pos)
        {
            f->strval[f->pos] = 0;
            me->cb.hit_str(me, f->key, f->len,
                    (const unsigned char*)f->strval, f->len);
            f = __pop_stack(me);
        }

        break;
    case BENCODE_TOK_DICT:
        if ('e' == **buf)
        {
            f = __pop_stack(me);
            goto done;
        }

        f = __push_stack(me);
        f->type = BENCODE_TOK_DICT_KEYLEN;
        f->pos = 0;
        /* drop through */
    case BENCODE_TOK_DICT_KEYLEN:
        if (':' == **buf)
        {
            f->type = BENCODE_TOK_DICT_KEY;
            f->pos = 0;
        }
        else if (isdigit(**buf))
        {
            f->len = __parse_digit(f->len, **buf);
        }
        /* end of dictionary */
        else if ('e' == **buf)
        {
            //return 0;
            f = __pop_stack(me);
        }
        else
        {
            return 0;
        }

        break;
    case BENCODE_TOK_DICT_KEY:
        if (f->k_size <= f->pos + 1)
        {
            f->k_size = f->k_size * 2 + 4;
            f->key = realloc(f->key, f->k_size);
        }

        f->key[f->pos++] = **buf; 

        if (f->pos == f->len)
        {
            f->key[f->pos] = '\0';
            //f = __push_stack(me);
            f->type = BENCODE_TOK_DICT_VAL;
            f->pos = 0;
            f->len = 0;
        }
        break;

    default:
        assert(0); break;
    }

done:
    (*buf)++;
    *len -= 1;
    return 1;
}

int bencode_dispatch_from_buffer(
        bencode_t* me,
        const char* buf,
        unsigned int len)
{
    if (me->nframes <= me->d)
        return 0;

    while (0 < len)
    {
        switch(__process_tok(me, &buf, &len))
        {
            case 0:
                return 0;
                break;
        }
    }

    return 1;
}

void bencode_set_callbacks(
        bencode_t* me,
        bencode_callbacks_t* cb)
{
     memcpy(&me->cb,cb,sizeof(bencode_callbacks_t));
}

struct bencode_list_node
{
  bencode_obj_t val;
  struct bencode_list_node * next;
};

struct bencode_bytestring
{
  uint8_t * data;
  size_t sz;
};

struct bencode_dict_node
{
  struct bencode_bytestring key;
  bencode_obj_t val;
  struct bencode_dict_node * next;
};

struct bencode_obj
{
  struct bencode_list_node * list;
  struct bencode_dict_node * dict;
  struct bencode_bytestring * str;
  int64_t * integer;
};

static bencode_obj_t bencode_alloc()
{
  return xmalloc(sizeof(struct bencode_obj));
}

void bencode_obj_free(bencode_obj_t * o)
{
  bencode_obj_t obj = *o;
  // no object
  if(!obj) return;
  // free if string
  if(obj->str) {
    free(obj->str->data);
    free(obj->str);
    obj->str = NULL;
  }
  // free if list
  struct bencode_list_node * l = obj->list;
  while(l) {
    bencode_obj_free(&l->val);
    l = l->next;
  }
  // free if dict
  struct bencode_dict_node * d = obj->dict;
  while(d) {
    free(d->key.data);
    bencode_obj_free(&d->val);
    d = d->next;
  }
  // free integer
  free(obj->integer);
  // free object
  free(obj);
  *o = NULL;
}

void bencode_obj_dict(bencode_obj_t * o)
{
  *o = bencode_alloc();
  (*o)->dict = xmalloc(sizeof(struct bencode_dict_node));
}

void bencode_obj_str(bencode_obj_t * o, const uint8_t * str, size_t sz)
{
  *o = bencode_alloc();
  (*o)->str = xmalloc(sizeof(struct bencode_bytestring));
  (*o)->str->sz = sz;
  (*o)->str->data = xmalloc(sz);
  memcpy((*o)->str->data, str, sz);
}

void bencode_obj_int(bencode_obj_t *o, int64_t i)
{
  *o = bencode_alloc();
  (*o)->integer = xmalloc(sizeof(int64_t));
  *(*o)->integer = i;
}

ssize_t bencode_obj_getstr(bencode_obj_t o, uint8_t ** buf)
{
  if(!o->str) {
    return -1;
  }
  *buf = o->str->data;
  return o->str->sz;
}

int bencode_obj_is_dict(bencode_obj_t o)
{
  return o->dict != NULL;
}

void bencode_obj_iter_dict(bencode_obj_t o, bencode_obj_dict_iter i, void * u)
{
  struct bencode_dict_node * node = o->dict;
  while(node) {
    i(o, node->key.data, node->val, u);
    node = node->next;
  }
}

void bencode_obj_dict_set(bencode_obj_t d, const char * k, bencode_obj_t v)
{
  bencode_obj_t val = NULL;
  bencode_obj_clone(v, &val);
  struct bencode_dict_node * curr = d->dict;
  struct bencode_dict_node * prev = NULL;
  while(curr) {
    if(strcmp(k, curr->key.data) < 0) {
      break;
    }
    prev = curr;
    curr = curr->next;
  }
  struct bencode_dict_node * node = xmalloc(sizeof(struct bencode_dict_node));
  node->key.data = strdup(k);
  node->key.sz = strlen(k);
  node->val = val;
  if(prev) {
    prev->next = node;
    node->next = curr;
  } else {
    curr->next = node;
    node->next = NULL;
  }
}

int bencode_obj_is_list(bencode_obj_t o)
{
  return o->list != NULL;
}

void bencode_obj_iter_list(bencode_obj_t o, bencode_obj_list_iter i, void * u)
{
  struct bencode_list_node * list = o->list;
  while(list) {
    i(o, list->val, u);
    list = list->next;
  }
}

static void _bencode_list_copy(bencode_obj_t o, bencode_obj_t v, void * u)
{
  bencode_obj_t l = (bencode_obj_t) u;
  bencode_obj_t v_clone = NULL;
  bencode_obj_clone(v, &v_clone);
  bencode_obj_list_append(l, v_clone);
}

static void _bencode_dict_copy(bencode_obj_t o, const char * k, bencode_obj_t v, void * u)
{
  bencode_obj_t d = (bencode_obj_t) u;
  bencode_obj_t v_clone = NULL;
  bencode_obj_clone(v, &v_clone);
  bencode_obj_dict_set(d, k, v_clone);
}

void bencode_obj_clone(bencode_obj_t o, bencode_obj_t * clone)
{
  if(o->list) {
    bencode_obj_list(clone);
    bencode_obj_iter_list(o, _bencode_list_copy, *clone);
    return;
  }
  if(o->dict) {
    bencode_obj_dict(clone);
    bencode_obj_iter_dict(o, _bencode_dict_copy, *clone);
    return;
  }
  if(o->integer) {
    bencode_obj_int(clone, *o->integer);
    return;
  }
  if(o->str) {
    bencode_obj_str(clone, o->str->data, o->str->sz);
    return;
  }
}

void bencode_obj_list_append(bencode_obj_t l, bencode_obj_t v)
{
  struct bencode_list_node * node = l->list;
  while(node->next) {
    node = node->next;
  }
  node->val = v;
  node->next = xmalloc(sizeof(struct bencode_list_node));
}

void bencode_obj_list(bencode_obj_t * l)
{
  *l = bencode_alloc();
  (*l)->list = xmalloc(sizeof(struct bencode_list_node));
}

int bencode_obj_getint(bencode_obj_t o, int64_t * i)
{
  if(o->integer) {
   *i = *o->integer;
   return 1;
  }
  return 0;
}

typedef struct {
  FILE * f;
  ssize_t written;
} bencode_obj_write_ctx_t;

void _bencode_write(bencode_obj_t o, bencode_obj_write_ctx_t * ctx);

static ssize_t _bencode_write_string(bencode_obj_write_ctx_t * ctx, uint8_t * k, size_t sz)
{
  ssize_t res = 0;
  res = fprintf(ctx->f, "%ld", sz);
  if(res < 0) {
    return -1;
  }
  if(fputc(':', ctx->f) < 0) {
    return -1;
  } else {
    res += sizeof(char);
  }
  if(sz > 0) {
    if(fwrite(k, sz, 1, ctx->f) != sz) {
      return -1;
    }
  }
  res += sz;
  return res;
}

// write a dict key/value
static void _bencode_dict_write(bencode_obj_t d, const char * k, bencode_obj_t v, void * u)
{
  ssize_t res = 0;
  bencode_obj_write_ctx_t * ctx = (bencode_obj_write_ctx_t *) u;
  // write key

  res = _bencode_write_string(ctx, (uint8_t*)k, strlen(k));
  if(res < 0) {
    ctx->written = -1;
  } else {
    ctx->written += res;
  }
  // write value
  _bencode_write(v, ctx);
}

// write a list item
static void _bencode_list_write(bencode_obj_t l, bencode_obj_t i, void * u)
{
  bencode_obj_write_ctx_t * ctx = (bencode_obj_write_ctx_t *) u;
  _bencode_write(i, u);
}

void _bencode_write(bencode_obj_t o, bencode_obj_write_ctx_t * ctx)
{
  // check for write error
  ssize_t res = 0;
  if(ctx->written == - 1) return;
  uint8_t * str = NULL;
  ssize_t sz = bencode_obj_getstr(o, &str);
  if(sz == -1) {
    // not a string
    int64_t i = 0;
    if(bencode_obj_getint(o, &i)) {
      res = fprintf(ctx->f, "i%ld", i);
      if(res < 0) {
        // error
        ctx->written = -1;
      } else {
        ctx->written += res;
      }
    } else if(bencode_obj_is_dict(o)) {
      if(fputc('d', ctx->f) < 0) {
        // error
        ctx->written = -1;
      } else {
        ctx->written += sizeof(char);
        bencode_obj_iter_dict(o, _bencode_dict_write, ctx);
      }
    } else if (bencode_obj_is_list(o)) {
      if(fputc('l', ctx->f) < 0) {
        // error
        ctx->written = -1;
      } else {
        ctx->written += sizeof(char);
        bencode_obj_iter_list(o, _bencode_list_write, ctx);
      }
    }

    if(ctx->written > 0) {
      if(fputc('e', ctx->f) < 0) {
        // error
        ctx->written = -1;
      } else {
        ctx->written += sizeof(char);
      }
    }
  } else {
    res = _bencode_write_string(ctx, str, sz);
    if(res < 0) {
      // error
      ctx->written = -1;
    } else {
      ctx->written += res;
    }
  }
}

ssize_t bencode_write_fd(bencode_obj_t o, int fd)
{
  ssize_t sz = -1;
  FILE * f = fdopen(fd, "w");
  if(f) {
    sz = bencode_write_file(o, f);
    fclose(f);
  }
  return sz;
}

ssize_t bencode_write_file(bencode_obj_t o, FILE * f)
{
  bencode_obj_write_ctx_t ctx ={
    .f = f,
    .written = 0,
  };
  _bencode_write(o, &ctx);
  return ctx.written;
}

ssize_t bencode_write_buffer(bencode_obj_t o, uint8_t ** buf)
{
  // TODO: implement
  *buf = NULL;
  return -1;
}

typedef struct {
  FILE * f;
  ssize_t read;
} bencode_obj_read_ctx_t;


static void _bencode_read_str(bencode_obj_t * o, bencode_obj_read_ctx_t * ctx)
{
  size_t n = 0;
  char * line = NULL;
  if(getdelim(&line, &n, ':', ctx->f) != -1)
  {
    int slen = strlen(line);
    ctx->read += slen;
    line[slen-1] = 0; // minus delimiter
    int sz = atoi(line);
    if(sz > 0) {
      uint8_t * data = xmalloc(sz);
      if(fread(data, sz, 1, ctx->f) == sz) {
        // read okay
        bencode_obj_str(o, data, sz);
        ctx->read += sz;
      }
      free(data);
    }
    free(line);
  }
}

static void _bencode_read(bencode_obj_t * o, bencode_obj_read_ctx_t * ctx)
{
  size_t bufidx = 0;
  char buf[16] = {0}; // for num
  int c;
  while(ctx->read > 0) {
    c = fgetc(ctx->f);
    if(c <= 0) {
      ctx->read = -1;
      return;
    }
    ctx->read += 1;
    if(c == 'l') {
      // read list
      bencode_obj_list(o);
    } else if (c == 'd') {
      // read dict
      
      bencode_obj_t k = NULL;
      bencode_obj_t v = NULL;
      _bencode_read_str(&k, ctx);
      _bencode_read(&v, ctx);
      c = fgetc(ctx->f);
      if(c == 'e') {
        bencode_obj_dict(o);
        bencode_obj_t d = *o;
        //TODO: don't assume null terminated
        _bencode_obj_dict_set(d, k->str->data, v);
      }
      bencode_obj_free(&k);
      bencode_obj_free(&v);
    } else if (c == ':') {
      // read string
      int strsz = atoi(buf);
      if(strsz < 0) {
        // bad value
        return;
      } else if (strsz) {
        uint8_t * strdat = xmalloc(strsz);
        if(fread(strdat, strsz, 1, ctx->f) != strsz) {
          // error reading
        }
      } else {
        // empty string

      }
    }
  }
}

ssize_t bencode_read_file(bencode_obj_t * o, FILE * f)
{
  bencode_obj_read_ctx_t ctx = {
    .f = f,
    .read = 0,
  };
  _bencode_read(o, &ctx);
  return ctx.read;
}

ssize_t bencode_read_fd(bencode_obj_t * o, int fd)
{
  ssize_t sz = -1;
  FILE * f = fdopen(fd, "r");
  if(f) {
    sz = bencode_read_file(o, f);
    fclose(f);
  }
  return sz;
}
