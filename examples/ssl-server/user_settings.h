#include <stdint.h>
#if defined CONTIKI_TARGET_NATIVE || defined CONTIKI_TARGET_MINIMAL_NET
    #define WOLFSSL_GENERAL_ALIGNMENT 8
    #define TFM_X86_64
#elif defined CONTIKI_TARGET_SKY
#   error "16-bit platforms not supported!"
#else
    #define NO_64BIT
    #define WOLFSSL_GENERAL_ALIGNMENT 4
    #define TFM_ARM
#endif

//#define DEBUG_WOLFSSL
//#define WOLFSSL_LOG_PRINTF

#define WOLFSSL_CONTIKI
#define WOLFSSL_UIP
#define USER_TICKS
#define WOLFSSL_USER_CURRTIME
#define NO_WOLFSSL_MEMORY
#define RSA_LOW_MEM
#define NO_OLD_RNGNAME
#define SMALL_SESSION_CACHE
#define WOLFSSL_SMALL_STACK

#define SINGLE_THREADED
#define NO_SIG_WRAPPER

#define HAVE_FFDHE_2048
#define HAVE_CHACHA
#define HAVE_POLY1305
#define HAVE_ECC
#define HAVE_CURVE25519
#define CURVED25519_SMALL
#define HAVE_ONE_TIME_AUTH
#define WOLFSSL_DH_CONST
#define WORD64_AVAILABLE

#define HAVE_ED25519
#define HAVE_POLY1305
#define HAVE_SHA512
#define WOLFSSL_SHA512

#define TFM_TIMING_RESISTANT
#define ECC_TIMING_RESISTANT
#define WC_RSA_BLINDING

#define NO_WRITEV
#define NO_DEV_RANDOM
#define NO_FILESYSTEM
#define NO_MAIN_DRIVER
#define NO_MD4
#define NO_RABBIT
#define NO_HC128

#include <stdlib.h>
#include "contiki-net.h"
#include "sys/cc.h"
#include "wolfssl.h"
