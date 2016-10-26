#ifndef I2PD_LOG_H_
#define I2PD_LOG_H_
#include <stdint.h>

#define I2P_CONFIG_LOG_LEVEL "i2p.log.level"

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

// log all subsystems
#define LOG_ALL (LOG_NET | LOG_CRYPTO | LOG_I2NP | LOG_NTCP | LOG_SSU | LOG_MAIN | LOG_CONFIG )

/** initiailze logging */
void i2p_log_init();

void i2p_log_set_level(int level);
void i2p_log_set_scope(int scope);

/** log function */
void __i2p_log(int level, int lineno, const char * f, int scope, const char * fmt, ...);

void __i2p_debug_memory(int lineno, const char * f, int scope, const uint8_t * begin, const uint8_t * end);

#define i2p_error(...) __i2p_log(L_ERROR, __LINE__, __FILENAME__, __VA_ARGS__)
#define i2p_warn(...) __i2p_log(L_WARN, __LINE__, __FILENAME__, __VA_ARGS__)
#define i2p_info(...) __i2p_log(L_INFO, __LINE__, __FILENAME__, __VA_ARGS__)
#define i2p_debug(...) __i2p_log(L_DEBUG, __LINE__, __FILENAME__, __VA_ARGS__)
#define i2p_debug_memory(scope, begin, end)  __i2p_debug_memory(__LINE__, __FILENAME__, scope, begin, end)


#endif
