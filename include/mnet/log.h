#ifndef MNET_LOG_H_
#define MNET_LOG_H_
#include <stdint.h>

#define MNET_CONFIG_LOG_LEVEL "mnet.log.level"

// log debug spam
#define L_DEBUG 1
// log finer 
#define L_INFO 2
// log warnings only
#define L_WARN 3
// log errors only
#define L_ERROR 4

// log network io
#define LOG_NET (1 << 0)
// log crypto io
#define LOG_CRYPTO (1 << 1)
// log i2np messages
#define LOG_I2NP (1 << 2)
// log ntcp 
#define LOG_NTCP (1 <<  3)
// log ssu
#define LOG_SSU (1 << 4)
// log i2p client
#define LOG_MAIN (1 << 5)
// log config
#define LOG_CONFIG (1 << 6)
// log router context
#define LOG_ROUTER (1 << 7)
// log netdb io
#define LOG_NETDB (1 << 8)
// log datatypes parsing
#define LOG_DATA (1 << 9)
// log utils
#define LOG_UTIL (1 << 9)


// log all subsystems
#define LOG_ALL (LOG_NET | LOG_CRYPTO | LOG_I2NP | LOG_NTCP | LOG_SSU | LOG_MAIN | LOG_CONFIG | LOG_ROUTER | LOG_NETDB | LOG_DATA | LOG_UTIL)

/** initiailze logging */
void mnet_log_init();

void mnet_log_set_level(int level);
void mnet_log_set_scope(int scope);

/** log function */
void __mnet_log(int level, int lineno, const char * f, int scope, const char * fmt, ...);

void __mnet_debug_memory(int lineno, const char * f, int scope, const uint8_t * begin, const uint8_t * end);

#define mnet_error(...) __mnet_log(L_ERROR, __LINE__, __FILENAME__, __VA_ARGS__)
#define mnet_warn(...) __mnet_log(L_WARN, __LINE__, __FILENAME__, __VA_ARGS__)
#define mnet_info(...) __mnet_log(L_INFO, __LINE__, __FILENAME__, __VA_ARGS__)
#define mnet_debug(...) __mnet_log(L_DEBUG, __LINE__, __FILENAME__, __VA_ARGS__)
#define mnet_debug_memory(scope, begin, end)  __mnet_debug_memory(__LINE__, __FILENAME__, scope, begin, end)


#endif
