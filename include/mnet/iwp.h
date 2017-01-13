#ifndef MNET_IWP_H_
#define MNET_IWP_H_
#include <stdint.h>
#include <mnet/transport.h>
#include <mnet/garlic.h>
#include <mnet/bencode.h>

/**
   IWP -- invisible wire protocol
 */


struct iwp_config
{
  char * pubkey;
  char * addr;
  uint16_t port;
  uint16_t mtu;
};

#define default_iwp_config {NULL, NULL, 0, 1280}

void iwp_config_new(struct iwp_config ** cfg);
int iwp_config_load_dict(struct iwp_config * cfg, bencode_obj_t d);
void iwp_config_store_dict(struct iwp_config * cfg, bencode_obj_t d);
void iwp_config_free(struct iwp_config ** cfg);

/**
   @brief context for iwp garlic transport 
 */
struct iwp_server;

/**
   @brief allocate a new iwp_server
 */
void iwp_server_new(struct iwp_server ** serv);

/**
   @brief free an existing iwp_server
 */
void iwp_server_free(struct iwp_server ** serv);

/**
   @brief configure iwp_server settings before run
 */
void iwp_server_configure(struct iwp_server * serv, struct iwp_config cfg);

/**
   @brief attach a iwp_server to a transport, must be called before iwp_server_run
 */
void iwp_server_attach(struct iwp_server * serv, struct mnet_garlic_transport * t);

/**
   @brief start iwp_server main loop
*/
int iwp_server_run(struct iwp_server * serv);

/**
   @brief close running iwp_server socket
 */
void iwp_server_close(struct iwp_server * serv);

/**
   @brief hook called AFTER iwp_server socket closes
 */
typedef void(*iwp_server_close_hook)(struct iwp_server *, void *);

/**
   @brief add a hook to be called after iwp_server's socket is closed, associate user data with hook
 */
void iwp_server_add_close_hook(struct iwp_server * serv, iwp_server_close_hook hook, void * userdata);

struct iwp_session;

typedef void(*iwp_session_visitor)(struct iwp_server *, struct iwp_session *, const char *, void *);

void iwp_server_obtain_session_by_ident_hash(struct iwp_server * serv, ident_hash h, iwp_session_visitor v, void *u);

/**
   @brief queue a garlic message to be sent via a iwp_session
   @return 0 if queued successfully, -1 on i/o error, -2 on send queue overflow
 */
int iwp_session_queue_send(struct iwp_session * s, struct mnet_garlic_msg * msg);

#endif
