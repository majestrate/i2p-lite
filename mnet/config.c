#include <mnet/config.h>
#include <mnet/crypto.h>
#include <mnet/log.h>
#include <mnet/memory.h>
#include <mnet/util.h>

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


static void skip_spaces(char ** str)
{
  while(**str && **str == ' ') (*str)++;
}

static void skip_chars(char ** str)
{
  while(**str && (**str == '.' || isalnum(**str))) (*str)++;
}

static void read_to_comment(char ** str)
{
  while(**str && **str != '#' && **str != '\n') (*str)++;
}

static int is_comment_line(char * str)
{
  skip_spaces(&str);
  return *str == '#';
}

static int is_emtpy_line(char * str)
{
  skip_spaces(&str);
  return *str == '\n';
}

// entry in config
struct mnet_config_entry
{
  char * name;
  char * value;
  struct mnet_config_entry * prev;
};

struct mnet_config
{
  struct mnet_config_entry * list;
  size_t size;
};

/** @brief allocate config structure */
static void mnet_config_alloc(struct mnet_config ** cfg)
{
  *cfg = xmalloc(sizeof(struct mnet_config));
}

/** @brief append key/value pair to config */
static void mnet_config_append_entry(struct mnet_config * cfg, char * name, char * value)
{
  struct mnet_config_entry * entry;
  size_t idx = cfg->size;
  entry = xmalloc(sizeof(struct mnet_config_entry));
  entry->name = strdup(name);
  entry->value = strdup(value);
  entry->prev = cfg->list;
  cfg->list = entry;
  cfg->size ++;
}

void mnet_config_for_each(struct mnet_config * cfg, mnet_config_iter_t iter, void * user)
{
  struct mnet_config_entry * entry = cfg->list;
  while(entry) {
    iter(entry->name, entry->value, user);
    entry = entry->prev;
  }
}

int mnet_config_load(struct mnet_config ** cfg, const char * filepath)
{

  // buffer for a line
  char linebuf[1024] = {0};
  // return value
  int ret = 1;
  // current line
  char * line = 0;
  // file handle for config file
  FILE * f;
  // current config key
  char * kstart, *kend, *k;
  // current config value
  char * vstart, *vend, *v;
  
  // allocate config
  mnet_config_alloc(cfg);
  
  
  // open file
  f = fopen(filepath, "r");
  if(!f) return 0; // cannot open file
  mnet_debug(LOG_CONFIG, "opened file %s", filepath);
  
  // read lines
  while ((line = fgets(linebuf, sizeof(linebuf), f))) {
    mnet_debug(LOG_CONFIG, "config line: %s", line);
    if(is_comment_line(line)) continue;
    if(is_emtpy_line(line)) continue;
    skip_spaces(&line);
    kstart = line;
    skip_chars(&line);
    kend = line;
    k = strndup(kstart, kend-kstart);
    skip_spaces(&line);
    if(*line != '=') {
      mnet_error(LOG_CONFIG, "bad config line: %s", line);
      ret = 0;
      break;
    }
    line ++;
    
    skip_spaces(&line);
    vstart = line;
    
    skip_chars(&line);
    read_to_comment(&line);
    vend = line;
    
    v = strndup(vstart, vend-vstart);
    mnet_config_append_entry(*cfg, k, v);
    free(k);
    free(v);
  }
  
  // close file
  fclose(f);
  return ret;
}

void mnet_config_free(struct mnet_config ** cfg)
{
  struct mnet_config_entry * entry = (*cfg)->list;
  struct mnet_config_entry * prev = NULL;
  while(entry) {
    free(entry->name);
    free(entry->value);
    prev = entry->prev;
    free(entry);
    entry = prev;
  }
  free (*cfg);
  *cfg = NULL;
}

void mnet_config_iter_write(char * k, char * v, void * user)
{
  FILE * f = (FILE *) user;
  assert(fprintf(f, "%s = %s\n", k, v) != -1);
}

int mnet_config_gen(char * filepath)
{
  FILE * f;
  struct mnet_config * c;

  f = fopen(filepath, "w");
  if(!f) return 0;

  fprintf(f, "# auto generated config file\n# generated at %lu\n\n", time(NULL));
  
  mnet_config_alloc(&c);

  char * home = getenv("HOME");
  char * datadir = path_join("/", "usr", "lib", "mnet", 0);
  if(home)
    datadir = path_join(home, ".config", "mnet", 0);
  
  // put default settings
  mnet_config_append_entry(c, MNET_CONFIG_CRYPTO_CHECK, "1");
  mnet_config_append_entry(c, "mnet.datadir", datadir);
  mnet_config_append_entry(c, MNET_CONFIG_LOG_LEVEL, "1");

  
  mnet_config_for_each(c, mnet_config_iter_write, f);
  
  mnet_config_free(&c);

  fclose(f);
  return 1;
}
