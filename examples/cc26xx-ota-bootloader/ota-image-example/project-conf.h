#undef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE           1280

#undef REST_MAX_CHUNK_SIZE
//#define REST_MAX_CHUNK_SIZE            256

/*---------------------------------------------------------------------------*/
/* COAP                                                                      */
/*---------------------------------------------------------------------------*/

#define COAP_SERVER_PORT 5683

/* Uncomment to remove /.well-known/core resource to save code */
//#define WITH_WELL_KNOWN_CORE            0

/* COAP content type definition */
#ifndef COAP_CONF_DATA_FORMAT
#define COAP_CONF_DATA_FORMAT coap_data_format_text
#endif
#define REST_MAX_CHUNK_SIZE     64

/**
*  For srf06 board we must specify SPI connections
*/

#define BOARD_IOID_FLASH_CS       IOID_14
#define BOARD_FLASH_CS            (1 << BOARD_IOID_FLASH_CS)
#define BOARD_IOID_SPI_CLK_FLASH  IOID_10
#define BOARD_IOID_SPI_MOSI       IOID_9
#define BOARD_IOID_SPI_MISO       IOID_8
