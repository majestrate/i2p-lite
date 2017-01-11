#ifndef I2PD_IWP_H_
#define I2PD_IWP_H_
#include <stdint.h>
#include <i2pd/transport.h>
#include <i2pd/i2np.h>

/**
   IWP -- invisible wire protocol, replacement for ssu
 */


struct iwp_config
{
  char * addr;
  uint16_t port;
  uint16_t mtu;
};

void iwp_config_new(struct iwp_config ** cfg);
void iwp_config_free(struct iwp_config ** cfg);

/**
   @brief context for iwp i2np transport 
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
   @brief attach a iwp_server to an i2np transport, must be called before iwp_server_run
 */
void iwp_server_attach(struct iwp_server * serv, struct i2np_transport * t);

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
   @brief queue an i2np message to be sent via a iwp_session
   @return 0 if queued successfully, -1 on i/o error, -2 on send queue overflow
 */
int iwp_session_queue_send(struct iwp_session * s, struct i2np_msg * msg);

#endif
