#ifndef _RTK_TYPES_H_
#define _RTK_TYPES_H_

#include <c_types.h>


typedef uint64_t    	uint64;
typedef int64_t			int64;
typedef uint32_t		uint32;
typedef int32_t         int32;
typedef uint16_t        uint16;
typedef int16_t         int16;
typedef uint8_t         uint8;
typedef int8_t          int8;

#ifndef PROGMEM
# define PROGMEM   ICACHE_RODATA_ATTR
#endif

#define CONST_T     const PROGMEM	// for esp8266



typedef uint32               ipaddr_t;
typedef uint32				 memaddr;	

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN		6
#endif

typedef struct ether_addr_s {
	uint8 octet[ETHER_ADDR_LEN];
} ether_addr_t;

#define swapl32(x)\
        ((((x) & 0xff000000U) >> 24) | \
         (((x) & 0x00ff0000U) >>  8) | \
         (((x) & 0x0000ff00U) <<  8) | \
         (((x) & 0x000000ffU) << 24))
#define swaps16(x)        \
        ((((x) & 0xff00) >> 8) | \
         (((x) & 0x00ff) << 8))  
		 
#ifdef LITTLE_ENDIAN
# ifndef _LITTLE_ENDIAN
#  define _LITTLE_ENDIAN			// 
# endif
#endif		 

#ifndef ntohs
# ifdef LITTLE_ENDIAN
	#define ntohs(x)   (swaps16(x))
	#define ntohl(x)   (swapl32(x))
	#define htons(x)   (swaps16(x))
	#define htonl(x)   (swapl32(x))
# else
	#define ntohs(x)	(x)
	#define ntohl(x)	(x)
	#define htons(x)	(x)
	#define htonl(x)	(x)
# endif
#endif


#ifdef LITTLE_ENDIAN
	#define MEM16(x)		(x)
#else
	#define MEM16(x)		(swaps16(x)) 
#endif

/* type abstraction */

typedef int32                   rtk_api_ret_t;
typedef int32                   ret_t;
typedef uint64                  rtk_u_long_t;


#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef FAILED
#define FAILED -1
#endif

#if 0		//
	#ifdef __KERNEL__
		#define rtlglue_printf printk
	#else
		#define rtlglue_printf printf
	#endif
#endif

#define rtlglue_printf os_printf



#endif /* _RTL8370_TYPES_H_ */
