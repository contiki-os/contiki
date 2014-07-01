/*

  HMAC-SHA1-96 Wrapper around the MIRCLE SHA1 implementation

  Follows the definition in RFC2104. Should be tested against RFC2202 section 3.

  Tony Chung <a.chung at lancaster ac.uk>
  =====
  Can now wrap around ContikiECC too - Vilhelm Jutvik 120226

*/


// OPTIONS:


// Enable test function
//#define HMAC_SHA1_96_TEST

#include "contiki.h"
#include "hmac-sha1.h"

#define MIRACL 1
#define CONTIKIECC 2

#define SHA_LIB CONTIKIECC // or MIRACL

#if SHA_LIB == CONTIKIECC
  #include "contikiecc/ecc/ecc_sha1.h"
#elif SHA_LIB == MIRACL
  #include "miracle.h"
#else
	#error "sha_hmac: What SHA implementation should I wrap around? SHA_LIB string not recognized."
#endif


#define dy_printf(...)
//#define dy_printf(...) printf(__VA_ARGS__)

// Statically defined padding
#define SHA1_B 64 // Byte length of blocks
#define SHA1_L 20 // Byte length of hash output
uint8_t ipad[ SHA1_B ];
uint8_t opad[ SHA1_B ];


/**
 * Initalises the ipad and opad with the relevant number of repeating values.
 */
static void init(){

	uint8_t i;

	for( i = 0; i < SHA1_B; i++ ){
		ipad[i] = 0x36;
		opad[i] = 0x5c;
	}

}



/**
 * Creates a padded key of SHA1_B bytes length. Keys longer than 64 bytes will
 * be truncated. (According to the HMAC spec, this should be caught earlier
 * and such keys hashed first to produce a SHA1_L length key which is then
 * fed into this function.) outkey can be the same as inkey, in which case
 * this function will just end up setting zeros beyond inkeylen bytes.
 *
 * @param outkey  The output key: must be an initialised array of SHA1_B length.
 * @param inkey   The input key: must be at least inkeylen length.
 * @param inkeylen  The input key length (bytes).
 */
static void create_padded_key( uint8_t * outkey, uint8_t * inkey, uint16_t inkeylen ){

	uint16_t i;


	dy_printf("inkeylen:%d\n",inkeylen);

	// Copy bytes - truncate if necessary
	for( i = 0; ( i < SHA1_B ) && ( i < inkeylen ); i++ ){
		outkey[i] = inkey[i];
	}

	dy_printf("ilk loop:%d\n",i);

	// Set zeros (if applicable)
	for( ; i < SHA1_B; i++ ){
		outkey[i] = 0x00;
	}


}


/**
 * XORs each byte in ina with the corresponding byte in inb. The output is
 * written into the corresponding bytes in out.
 *
 * All arrays must be at least len bytes in length. It is safe to output
 * to one of the input arrays.
 *
 * @param out  The output array
 * @param ina  The first input array
 * @param inb  The second input array
 * @param len  The length of each array.
 */
static void xor_array( uint8_t * out, uint8_t * ina, uint8_t * inb, uint16_t len ){

	uint16_t i;

	for( i = 0; i < len; i++ ){
		out[i] = ina[i] ^ inb[i];
	}

}


/**
 * Merges the contents of inb to the end of ina. Output written to out.
 * out must be at least lena + lenb. Output can safely be written into the
 * same array as ina (if it is long enough) but not into inb.
 *
 * @param out  The output array
 * @param ina  The first array
 * @param lena The length of the first array
 * @param inb  The second array
 * @param lenb The length of the second array
 *
 */
static void merge_arrays( uint8_t * out, uint8_t * ina, uint16_t lena, uint8_t * inb, uint16_t lenb ){

	uint16_t i;

	for( i = 0; i < lena; i++ ){
		out[i] = ina[i];
	}

	for( i = 0; i < lenb; i++ ){
		out[i + lena] = inb[i];
	}

}


/**
 * Calls the SHA1 hash function from the MIRACL (or ContikiECC) library on the in array
 * and writes the result to the out array.
 *
 * @param  out   The output array (must be at least SHA1_B length)
 * @param  in    The input array
 * @param  inlen The length of the input array
 */
static void hash_sha1( uint8_t * out, uint8_t * in, uint16_t inlen)
{
  #if SHA_LIB == CONTIKIECC  // Using ContikiECC for SHA1
  contikiecc_sha1(in, inlen, out);

  #else // Using MIRACL for SHA1
	// Local variables
	sha sh;
	uint16_t i;

	// Call relevant MIRACL functions
	shs_init( &sh );

	for( i = 0; i < inlen; i++ ){
		shs_process( &sh, in[i] );
	}

	shs_hash( &sh, out );
	
	#endif
}

void PRINT_BUF(const char* name,unsigned char* buf, unsigned int size)
{
  int i=0;
  dy_printf("BUF:%s\n",name);
  for(;i<size;i++)
  {
    dy_printf("%.2x ", buf[i]);
  }
  dy_printf("\n");
}

/**
 * Performs HMAC-SHA1-96. Call this function from your program.
 *
 * @param out     Output from HMAC-SHA1-96 (must be at least 96/8=12 bytes)
 * @param key     The key to use
 * @param keylen  The length of the key
 * @param data    The data to hash
 * @param datalen The length of the data
 */
//void hmac_sha1_96( uint16_t datalen, uint8_t * out, uint8_t * key, uint8_t keylen, uint8_t * data )
void hmac_sha1( hmac_data_t *hmac_data )
{
  uint8_t * out=hmac_data->out;
  uint8_t * key= (uint8_t *) hmac_data->key;
  uint8_t keylen=hmac_data->keylen;
  uint8_t * data=hmac_data->data;
  uint16_t datalen=hmac_data->datalen;

  dy_printf( "Data after %d %d %d %d %d\n", (int)out, (int)key,(int)keylen,(int)data, (int)datalen );
	// Variables
	uint8_t padded_key[ SHA1_B ];
	uint8_t padded_key_xor_ipad[ SHA1_B ];
	uint8_t sentence_a[ SHA1_B + datalen ];
	uint8_t hash_output_a[ SHA1_L ];
	uint8_t padded_key_xor_opad[ SHA1_B ];
	uint8_t sentence_b[ SHA1_B + SHA1_L ];
	uint8_t hash_output_b[ SHA1_L ];

	init();

	// Hash overlength keys
	if( keylen > SHA1_B ){
		hash_sha1( padded_key, key, keylen ); // Assumes SHA1_L <= SHA_B
		create_padded_key( padded_key, padded_key, SHA1_L ); // Adds the zeros
	}
	else{
		create_padded_key( padded_key, key, keylen );
	}

	dy_printf("datelen:%d\n",(int)datalen);
	PRINT_BUF("padded_key",padded_key,SHA1_B);

	xor_array( padded_key_xor_ipad, padded_key, ipad, SHA1_B );
	PRINT_BUF("padded_key_xor_ipad",padded_key_xor_ipad,SHA1_B);

	merge_arrays( sentence_a, padded_key_xor_ipad, SHA1_B, data, datalen );
	PRINT_BUF("sentence_a",sentence_a,SHA1_B+datalen);

	hash_sha1( hash_output_a, sentence_a, SHA1_B + datalen );
	PRINT_BUF("hash_output_a",hash_output_a,SHA1_L);

	xor_array( padded_key_xor_opad, padded_key, opad, SHA1_B );
	PRINT_BUF("padded_key_xor_opad",padded_key_xor_opad,SHA1_B);

	merge_arrays( sentence_b, padded_key_xor_opad, SHA1_B, hash_output_a, SHA1_L );

  hash_sha1(hmac_data->out, sentence_b, SHA1_B + SHA1_L );
	// Old truncate code
	//hash_sha1( hash_output_b, sentence_b, SHA1_B + SHA1_L );
	//truncate( out, hash_output_b, 96/8 );
}

#ifdef HMAC_SHA1_96_TEST

int main(){


	// Change these (each line is eight bytes):
	/*
	uint8_t test_key[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		};
	#define KEY_LENGTH 64

	uint8_t test_data[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		};
	#define DATA_LENGTH 64
	*/

	uint8_t test_key[] = {
		0x01, 0x02, 0x03, 0x04, 0x05, 
		0x06, 0x07, 0x08, 0x09, 0x0a,
		0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
		0x10, 0x11, 0x12, 0x13, 0x14,
		0x15, 0x16, 0x17, 0x18, 0x19
	};
	#define KEY_LENGTH 25

	uint8_t test_data[] = {
		0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd,
		0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd,
		0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd,
		0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd,
		0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd
	};
	#define DATA_LENGTH 50

	// Don't change the rest:
	#define OUT_LENGTH 12 // 96/8 = 12
	uint8_t test_output[ OUT_LENGTH ];

	uint16_t i;


	hmac_sha1( test_output, test_key, KEY_LENGTH, test_data, DATA_LENGTH );

	dy_printf( "TEST PROGRAM FOR HMAC-SHA1-96\n" );


	dy_printf( "\nInput Key:    " );
	for( i = 0; i < KEY_LENGTH; i ++ ){
		dy_printf( "%.2x", test_key[i] );
	}

	dy_printf( "\nInput data:   " );
	for( i = 0; i < DATA_LENGTH; i ++ ){
		dy_printf( "%.2x", test_data[i] );
	}

	dy_printf( "\nOutput hash:  " );
	for( i = 0; i < OUT_LENGTH; i ++ ){
		dy_printf( "%.2x", test_output[i] );
	}

	dy_printf( "\n\n" );
}

#endif // HMAC_SHA1_96_TEST
