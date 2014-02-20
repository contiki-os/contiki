/*
 * if_ether.h
 *
 * Created: 4/12/2013 8:09:53 PM
 *  Author: Ioannis Glaropoulos
 */ 


#ifndef IF_ETHER_H_
#define IF_ETHER_H_

/*
 *	IEEE 802.3 Ethernet magic constants.  The frame sizes omit the preamble
 *	and FCS/CRC (frame check sequence).
 */

#define ETH_ALEN	6		/* Octets in one ethernet addr	 */
#define ETH_HLEN	14		/* Total octets in header.	 */
#define ETH_ZLEN	60		/* Min. octets in frame sans FCS */
#define ETH_DATA_LEN	1500		/* Max. octets in payload	 */
#define ETH_FRAME_LEN	1514		/* Max. octets in frame sans FCS */
#define ETH_FCS_LEN	4		/* Octets in the FCS		 */


#endif /* IF_ETHER_H_ */