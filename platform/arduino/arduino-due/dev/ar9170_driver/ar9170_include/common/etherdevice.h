#include "if_ether.h"
/*
 * etherdevice.h
 *
 * Created: 4/28/2013 11:48:31 PM
 *  Author: ioannisg
 */ 


#ifndef ETHERDEVICE_H_
#define ETHERDEVICE_H_


static inline unsigned compare_ether_addr(const U8 *addr1, const U8 *addr2)
{
         const U16 *a = (const U16 *) addr1;
         const U16 *b = (const U16 *) addr2;

         if(ETH_ALEN != 6)
			printf("BUG: Wrong Ethernet address length.\n");
         return ((a[0] ^ b[0]) | (a[1] ^ b[1]) | (a[2] ^ b[2])) != 0;
}

static inline bool ether_addr_equal(const U8 *addr1, const U8 *addr2)
{
         return !compare_ether_addr(addr1, addr2);
}

#endif /* ETHERDEVICE_H_ */