#include <i2pd/config.h>
#include <i2pd/crypto.h>
#include <i2pd/log.h>
#include <i2pd/memory.h>
#include <i2pd/util.h>

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

// entry in i2p config
struct i2p_config_entry
{
  char * name;
  char * value;
  struct i2p_config_entry * prev;
};

struct i2p_config
{
  struct i2p_config_entry * list;
  size_t size;
};

/** @brief allocate i2p config structure */
static void i2p_config_alloc(struct i2p_config ** cfg)
{
  *cfg = mallocx(sizeof(struct i2p_config), MALLOCX_ZERO);
}

/** @brief append key/value pair to i2p config */
static void i2p_config_append_entry(struct i2p_config * cfg, char * name, char * value)
{
  struct i2p_config_entry * entry;
  size_t idx = cfg->size;
  entry = mallocx(sizeof(struct i2p_config_entry), MALLOCX_ZERO);
  entry->name = strdup(name);
  entry->value = strdup(value);
  entry->prev = cfg->list;
  cfg->list = entry;
  cfg->size ++;
}

void i2p_config_for_each(struct i2p_config * cfg, i2p_config_iter_t iter, void * user)
{
  struct i2p_config_entry * entry = cfg->list;
  while(entry) {
    iter(entry->name, entry->value, user);
    entry = entry->prev;
  }
}

int i2p_config_load(struct i2p_config ** cfg, const char * filepath)
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
  i2p_config_alloc(cfg);
  
  
  // open file
  f = fopen(filepath, "r");
  if(!f) return 0; // cannot open file
  i2p_debug(LOG_CONFIG, "opened file %s", filepath);
  
  // read lines
  while ((line = fgets(linebuf, sizeof(linebuf), f))) {
    i2p_debug(LOG_CONFIG, "config line: %s", line);
    if(is_comment_line(line)) continue;
    if(is_emtpy_line(line)) continue;
    skip_spaces(&line);
    kstart = line;
    skip_chars(&line);
    kend = line;
    k = strndup(kstart, kend-kstart);
    skip_spaces(&line);
    if(*line != '=') {
      i2p_error(LOG_CONFIG, "bad config line: %s", line);
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
    i2p_config_append_entry(*cfg, k, v);
    free(k);
    free(v);
  }
  
  // close file
  fclose(f);
  return ret;
}

void i2p_config_free(struct i2p_config ** cfg)
{
  struct i2p_config_entry * entry = (*cfg)->list;
  struct i2p_config_entry * prev = NULL;
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

void i2p_config_iter_write(char * k, char * v, void * user)
{
  FILE * f = (FILE *) user;
  assert(fprintf(f, "%s = %s\n", k, v) != -1);
}

int i2p_config_gen(char * filepath)
{
  FILE * f;
  struct i2p_config * c;

  f = fopen(filepath, "w");
  if(!f) return 0;

  fprintf(f, "# auto generated config file\n# generated at %lu\n\n", time(NULL));
  
  i2p_config_alloc(&c);

  char * home = getenv("HOME");
  char * datadir = path_join("/", "usr", "lib", "i2p", 0);
  if(home)
    datadir = path_join(home, ".config", "i2p", 0);
  
  // put default settings
  i2p_config_append_entry(c, I2P_CONFIG_CRYPTO_CHECK, "1");
  i2p_config_append_entry(c, "i2p.datadir", datadir);
  i2p_config_append_entry(c, I2P_CONFIG_LOG_LEVEL, "2");

  
  i2p_config_for_each(c, i2p_config_iter_write, f);
  
  i2p_config_free(&c);

  fclose(f);
  return 1;
}
