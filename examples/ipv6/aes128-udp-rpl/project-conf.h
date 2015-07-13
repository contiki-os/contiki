//Be sure to be using CSMA for the mac (Typically default value for sky moteS)
#undef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC     csma_driver

//Be sure to be using the 802.15.4 for the frames 
#undef NETSTACK_CONF_FRAMER
#define NETSTACK_CONF_FRAMER  framer_802154

//The physical layer (the Radio) is the default for the sky motes (cc2420_driver)
#undef NETSTACK_CONF_RADIO
#define NETSTACK_CONF_RADIO   cc2420_driver

#undef AES_128_CONF 
//#define AES_128_CONF cc2420_aes_128_driver //Hardware
#define AES_128_CONF aes_128_driver	    //Software

/* NOTE: For debugging process the next variable can be declared. 
   but probably will be a good idea to remove an mote from the simulation.
*/
//#define DEBUG_AES128