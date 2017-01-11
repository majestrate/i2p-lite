
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <mnet/bencode.h>
#include <mnet/memory.h>

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
  if(sz)
  {
    (*o)->str->sz = sz;
    (*o)->str->data = xmalloc(sz);
    memcpy((*o)->str->data, str, sz);
  }
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
    if(node->key.data && node->val)
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
    if(!curr->key.data) break;
    if(strcmp(k, curr->key.data) < 0) break;
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
  if(sz) {
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

void _bencode_read(bencode_obj_t * o, bencode_obj_read_ctx_t * ctx, char prefix);

static void _bencode_read_str(bencode_obj_t * o, bencode_obj_read_ctx_t * ctx, char prefix)
{
  char numbuf[32] = {0};
  if(prefix) {
    numbuf[0] = prefix;
  }

  size_t n = 0;
  char * line = NULL;
  if(getdelim(&line, &n, ':', ctx->f) != -1){
    int slen = strlen(line);
    ctx->read += slen;
    line[slen-sizeof(char)] = 0; // minus delimiter
    char * num = strncpy(numbuf+sizeof(char), line, sizeof(numbuf)-sizeof(char));
    int sz = atoi(num);
    if(sz >= 0) {
      if(sz) {
        uint8_t * data = xmalloc(sz);
        if(fread(data, sz, 1, ctx->f) == sz) {
          // read okay
          bencode_obj_str(o, data, sz);
          ctx->read += sz;
        } else {
          ctx->read = -1;
        }
        free(data);
      } else {
        // empty string
        bencode_obj_str(o, NULL, 0);
      }
      // good read
      return;
    }
    free(line);
  }
  // bad read
  ctx->read = -1;
}

static void _bencode_read_dict_item(bencode_obj_t d, bencode_obj_read_ctx_t * ctx, char prefix)
{
  bencode_obj_t k = NULL;
  bencode_obj_t v = NULL;
  _bencode_read_str(&k, ctx, prefix);
  if(ctx->read < 0) {
    // read error
    return;
  }
  _bencode_read(&v, ctx, 0);
  if(ctx->read > 0) {
    // read okay
    bencode_obj_dict_set(d, k->str->data, v);
  }
  if(k) bencode_obj_free(&k);
  if(v) bencode_obj_free(&v);
}

void _bencode_read(bencode_obj_t * o, bencode_obj_read_ctx_t * ctx, char prefix)
{
  int c;
  if(prefix) {
    c = prefix;
  } else {
    c = fgetc(ctx->f);
    ctx->read += sizeof(char);
  }
  if(c == 'l') {
    // read list
    bencode_obj_list(o);
    bencode_obj_t l = *o;
    while((c = fgetc(ctx->f)) != 'e') {
      if (c < 0) {
        // bad read
        ctx->read = -1;
        return;
      } else {
        ctx->read += sizeof(char);
      }
      bencode_obj_t li = NULL;
      _bencode_read(&li, ctx, c);
      bencode_obj_list_append(l, li);
      if(li) bencode_obj_free(&li);
    }
  } else if (c == 'd') {
    // read dict
    bencode_obj_dict(o);
    bencode_obj_t d = *o;
    while((c = fgetc(ctx->f)) != 'e') {
      if(c < 0) {
        ctx->read = -1;
        return;
      } else {
        ctx->read += sizeof(char);
      }
      _bencode_read_dict_item(d, ctx, c);
    }
  } else if (c >= '0' && c <= '9') {
    // is a number
    _bencode_read_str(o, ctx, c);
  } else {
    // wtf lol bad char
    ctx->read = -1;
  }
}

ssize_t bencode_read_file(bencode_obj_t * o, FILE * f)
{
  bencode_obj_read_ctx_t ctx = {
    .f = f,
    .read = 0,
  };
  _bencode_read(o, &ctx, 0);
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
