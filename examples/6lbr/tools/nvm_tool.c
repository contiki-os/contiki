#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "nvm-config.h"
#include "native-nvm.h"

#define NVM_SIZE 4096
static uint8_t nvm_mem[NVM_SIZE];

nvm_data_t *  nvm_params = (nvm_data_t *)nvm_mem;

uint8_t empty[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

void create_empty_nvm(void)
{
     memset(nvm_mem, 0xff, NVM_SIZE);
}


void load_nvm_file(char const *  nvm_file)
{
     printf("Reading nvm file '%s'\n", nvm_file);

     create_empty_nvm();

     int s = open(nvm_file, O_RDONLY);
     if ( s > 0 )
     {
	  read(s, nvm_mem, NVM_SIZE);
	  close(s);
     }
     else
     {
	  perror("Error reading nvm file");
	  exit(1);
     }
}

void store_nvm_file(char const *  nvm_file, uint8_t fit)
{
     printf("Writing nvm file '%s'\n", nvm_file);
     int s = open(nvm_file, O_WRONLY | O_TRUNC | O_CREAT, 0644 );
     if ( s > 0 )
     {
	  write(s, nvm_mem, fit ? sizeof(nvm_data_t) : NVM_SIZE);
	  close(s);
     }
     else
     {
	  perror("Error writing nvm file");
	  exit(1);
     }
}

int
uiplib_ipaddrconv(const char *addrstr, uint8_t *ipaddr)
{
     uint16_t value;
     int tmp, zero;
     unsigned int len;
     char c = 0;

     value = 0;
     zero = -1;
     if(*addrstr == '[') addrstr++;

     for(len = 0; len < 16 - 1; addrstr++) {
	  c = *addrstr;
	  if(c == ':' || c == '\0' || c == ']' || c == '/') {
	       ipaddr[len] = (value >> 8) & 0xff;
	       ipaddr[len + 1] = value & 0xff;
	       len += 2;
	       value = 0;

	       if(c == '\0' || c == ']' || c == '/') {
		    break;
	       }

	       if(*(addrstr + 1) == ':') {
		    /* Zero compression */
		    if(zero < 0) {
			 zero = len;
		    }
		    addrstr++;
	       }
	  } else {
	       if(c >= '0' && c <= '9') {
		    tmp = c - '0';
	       } else if(c >= 'a' && c <= 'f') {
		    tmp = c - 'a' + 10;
	       } else if(c >= 'A' && c <= 'F') {
		    tmp = c - 'A' + 10;
	       } else {
		    return 0;
	       }
	       value = (value << 4) + (tmp & 0xf);
	  }
     }
     if(c != '\0' && c != ']' && c != '/') {
	  return 0;
     }
     if(len < 16) {
	  if(zero < 0) {
	       return 0;
	  }
	  memmove(&ipaddr[zero + 16 - len],
		  &ipaddr[zero], len - zero);
	  memset(&ipaddr[zero], 0, 16 - len);
     }

     return 1;
}

int
lib_euiconv(const char *addrstr, uint8_t max, uint8_t *eui)
{
     uint16_t value;
     int tmp;
     unsigned int len;
     char c = 0;

     value = 0;

     for(len = 0; len < max; addrstr++) {
	  c = *addrstr;
	  if(c == ':' || c == '\0' ) {
	       eui[len] = value & 0xff;
	       len++;
	       value = 0;

	       if(c == '\0') {
		    break;
	       }
	  } else {
	       if(c >= '0' && c <= '9') {
		    tmp = c - '0';
	       } else if(c >= 'a' && c <= 'f') {
		    tmp = c - 'a' + 10;
	       } else if(c >= 'A' && c <= 'F') {
		    tmp = c - 'A' + 10;
	       } else {
		    return 0;
	       }
	       value = (value << 4) + (tmp & 0xf);
	  }
     }
     if(c != '\0') {
	  return 0;
     }
     if(len < max) {
	  return 0;
     }

     return 1;
}

void
ipaddrconv(const char *  parameter, const char *addrstr, uint8_t *ipaddr)
{
     int result = uiplib_ipaddrconv( addrstr, ipaddr );
     if ( result == 0 ) {
	  fprintf(stderr, "Error: %s: invalid argument\n", parameter );
	  exit(1);
     }
}

void
euiconv(const char *  parameter, const char *addrstr,  uint8_t max, uint8_t *ipaddr)
{
     int result = lib_euiconv( addrstr, max, ipaddr );
     if ( result == 0 ) {
	  fprintf(stderr, "Error: %s: invalid argument\n", parameter);
	  exit(1);
     }
}

int
intconv( const char * parameter, const char *datastr )
{
     char * endptr;
     int value = strtol( datastr, &endptr, 10);
     if ( *endptr != '\0' ) {
	  fprintf(stderr, "Error: %s: invalid argument\n", parameter);
	  exit(1);
     }
     return value;
}

int
boolconv( const char * parameter, const char *datastr )
{
     char * endptr;
     int value = strtol( datastr, &endptr, 10);
     if ( *endptr != '\0' || (value != 0 && value != 1) ) {
	  fprintf(stderr, "Error: %s: invalid argument\n", parameter);
	  exit(1);
     }
     return value;
}

int
boolconvinv( const char * parameter, const char *datastr )
{
     char * endptr;
     int value = strtol( datastr, &endptr, 10);
     if ( *endptr != '\0' || (value != 0 && value != 1) ) {
	  fprintf(stderr, "Error: %s: invalid argument\n", parameter);
	  exit(1);
     }
     return value ? 0 : 1;
}

void
mode_update( uint8_t mask, uint8_t value )
{
     nvm_params->mode = (nvm_params->mode & (~ mask)) | (value ? mask : 0);
}

void
print_eui48addr(const uint8_t addr[6])
{
     if (memcmp( addr, empty, 6) == 0 ) {
	  printf("Default");
     } else {
	  printf("%02x:%02x:%02x:%02x:%02x:%02x",
		 addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
     }
}

void
print_eui64addr(const uint8_t addr[8])
{
     if (memcmp( addr, empty, 8) == 0 ) {
	  printf("Default");
     } else {
	  printf("%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
		 addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
     }
}

void
print_ipaddr(const uint8_t addr[16])
{
     if (memcmp( addr, empty, 16) == 0 ) {
	  printf("Default");
     } else {
	  uint16_t a;
	  unsigned int i;
	  int f;
	  for(i = 0, f = 0; i < 16; i += 2) {
	       a = (addr[i] << 8) + addr[i + 1];
	       if(a == 0 && f >= 0) {
		    if(f++ == 0) {
			 printf("::");
		    }
	       } else {
		    if(f > 0) {
			 f = -1;
		    } else if(i > 0) {
			 printf(":");
		    }
		    printf("%x", a);
	       }
	  }
     }
}

void
print_int(uint8_t value)
{
     if (value == 0xff ) {
	  printf("Default");
     } else {
	  printf("%d",value);
     }
}

void
print_bool(uint8_t value, uint8_t mask)
{
     if ( (value & mask) == 0 ) {
	  printf( "False" );
     } else {
	  printf( "True" );
     }
}

void
print_bool_inv(uint8_t value, uint8_t mask)
{
     if ( (value & mask) == 0 ) {
	  printf( "True" );
     } else {
	  printf( "False" );
     }
}

void print_nvm(void)
{
     printf( "Channel: " );
     print_int( nvm_params->channel );
     printf("\n");
     printf("\n");
     printf( "WSM MAC address : " );
     print_eui64addr( nvm_params->rime_addr);
     printf("\n");
     printf( "WSN network prefix : " );
     print_ipaddr( nvm_params->wsn_net_prefix );
     printf("\n");
     printf( "WSN IP address : " );
     print_ipaddr( nvm_params->wsn_ip_addr );
     printf("\n");
     printf( "WSN accept RA : " );
     print_bool( nvm_params->mode, CETIC_MODE_WAIT_RA_MASK );
     printf("\n");
     printf( "WSN IP address autoconf : " );
     print_bool( nvm_params->mode, CETIC_MODE_WSN_AUTOCONF );
     printf("\n");
     printf("\n");
     printf( "Eth MAC address : " );
     print_eui48addr( nvm_params->eth_mac_addr );
     printf("\n");
     printf( "Eth network prefix : " );
     print_ipaddr( nvm_params->eth_net_prefix );
     printf("\n");
     printf( "Eth IP address : " );
     print_ipaddr( nvm_params->eth_ip_addr );
     printf("\n");
     printf( "Eth default router : " );
     print_ipaddr( nvm_params->eth_dft_router);
     printf("\n");
     printf( "Eth RA daemon : " );
     print_bool( nvm_params->mode, CETIC_MODE_ROUTER_SEND_CONFIG );
     printf("\n");
     printf( "Eth IP address autoconf : " );
     print_bool_inv( nvm_params->mode, CETIC_MODE_ETH_MANUAL );
     printf("\n");
     printf("\n");
     printf( "Local address rewrite : " );
     print_bool( nvm_params->mode, CETIC_MODE_REWRITE_ADDR_MASK );
     printf("\n");
     printf( "Filter RPL : " );
     print_bool( nvm_params->mode, CETIC_MODE_FILTER_RPL_MASK );
     printf("\n");
     printf( "Filter NDP : " );
     print_bool( nvm_params->mode, CETIC_MODE_FILTER_NDP_MASK );
     printf("\n");
     printf("\n");
     printf( "RPL version_id : " );
     print_int( nvm_params->rpl_version_id );
     printf("\n");
}

#define new_nvm_action 1000
#define print_nvm_action 1001
#define update_nvm_action 1002

#define channel_option 2000
#define rime_addr_option 2001
#define wsn_prefix_option 2002
#define wsn_ip_option 2003
#define wsn_accept_ra_option 2004
#define wsn_addr_autoconf_option 2005

#define eth_mac_option 3000
#define eth_net_prefix_option 3001
#define eth_ip_option 3002
#define eth_dft_router_option 3003
#define eth_ra_daemon_option 3004
#define eth_addr_autoconf_option 3005


#define local_addr_rewrite_option 4000
#define rpl_filter_option 4001
#define ndp_filter_option 4002

#define rpl_version_option 5000

#define fit_option 6000

static struct option long_options[] = {
     { "help", no_argument, 0, 'h' },
     { "new", no_argument, 0, new_nvm_action },
     { "print", no_argument, 0, print_nvm_action },
     { "update", no_argument, 0, update_nvm_action },

     { "channel", required_argument, 0, channel_option },
     { "wsn-mac", required_argument, 0, rime_addr_option },
     { "wsn-prefix", required_argument, 0, wsn_prefix_option },
     { "wsn-ip", required_argument, 0, wsn_ip_option },
     { "wsn-accept-ra", required_argument, 0, wsn_accept_ra_option },
     { "wsn-ip-autoconf", required_argument, 0, wsn_addr_autoconf_option },

     { "eth-mac", required_argument, 0, eth_mac_option },
     { "eth-prefix", required_argument, 0, eth_net_prefix_option },
     { "eth-ip", required_argument, 0, eth_ip_option },
     { "dft-router", required_argument, 0, eth_dft_router_option },
     { "eth-ra-daemon", required_argument, 0, eth_ra_daemon_option },
     { "eth-ip-autoconf", required_argument, 0, eth_addr_autoconf_option },

     { "addr-rewrite", required_argument, 0, local_addr_rewrite_option },
     { "filter-rpl", required_argument, 0, rpl_filter_option },
     { "filter-ndp", required_argument, 0, ndp_filter_option },

     { "rpl-version", required_argument, 0, rpl_version_option },

     { "fit", no_argument, 0, fit_option },
};

void usage(char const *  name)
{
     fprintf(stderr, "Usage: %s --help|--new|--print|--update [OPTIONS] <nvm file> [<nvm_file>]\n", name);
     exit(1);
}

void help(char const * name)
{
     printf("Usage: %s COMMAND [OPTIONS]... <nvm file> [<nvm_file>]\n\n", name);
     printf("COMMANDS :\n");
     printf("\t--help\t\t\t\t This help\n");
     printf("\t--new [OPTIONS]... <nvm file>\t Create a new NVM file\n");
     printf("\t--print <nvm file>\t\t Dump the content of the given NVM file\n");
     printf("\t--update [OPTIONS]... <source nvm file> [<dest nvm file>]\n\t\t\t\t\t Update the given NVM file, the source file is reused if no destination file is given\n");
     printf("\n");

     printf("OPTIONS :\n");
     printf("\t--channel <11..26>\t\t 802.15.4 channel ID\n");
     printf("\t--wsn-mac <EUI-64>\t\t 80.15.4 MAC address\n");
     printf("\t--wsn-prefix <IPv6 prefix>\t IPv6 prefix to generate global adresses on the WSN network\n");
     printf("\t--wsn-ip <IPv6 address>\t\t IPv6 global address of the WSN interface\n");
     printf("\t--wsn-accept-ra <0|1>\t\t Use RA to configure WSN network\n");
     printf("\t--wsn-ip-autoconf <0|1>\t\t Use EUI-64 address to create global address\n");
     printf("\n");

     printf("\t--eth-mac <EUI-48>\t\t Eth MAC address\n");
     printf("\t--eth-prefix <IPv6 prefix>\t IPv6 prefix to generate global adress on the Eth interface\n");
     printf("\t--eth-ip <IPv6 address>\t\t IPv6 global address of the Eth interface\n");
     printf("\t--dft-router <IPv6 address>\t IPv6 address of the default router\n");
     printf("\t--eth-ra-daemon <0|1>\t\t Activate RA daemon\n");
     printf("\t--eth-ip-autoconf <0|1>\t\t Use EUI-48 address to create global address\n");
     printf("\n");

     printf("\t--addr-rewrite <0|1>\t\t Rewrite outgoing local addresses\n");
     printf("\t--filter-rpl <0|1>\t\t Filter out RPL messages\n");
     printf("\t--filter-NDP <0|1>\t\t Filter out NDP RA/RS messages\n");
     printf("\n");

     printf("\t--rpl-version <version>\t\t Current RPL DODAG version ID\n");
     printf("\n");
     printf("\t--fit\t\t\t\t Size NVM output file to the actual size of the NVM data\n");
     printf("\n");
     printf("Please read the 6LBR documentation for a complete description of all the modes and parameters and their interactions\n");
     exit(0);
}

int main(int argc, char * argv[])
{
     int c;
     int option_index = 0;
     int new_nvm_file = 0;
     int print_nvm_file = 0;
     int update_nvm_file = 0;
     int fit = 0;

     char *  source_nvm_file = NULL;
     char *  dest_nvm_file = NULL;

     char *  channel = NULL;
     char *  rime_addr = NULL;
     char *  wsn_prefix = NULL;
     char *  wsn_ip_addr = NULL;
     char *  wsn_accept_ra = NULL;
     char *  wsn_addr_autoconf = NULL;

     char *  eth_mac_addr = NULL;
     char *  eth_net_prefix = NULL;
     char *  eth_ip_addr = NULL;
     char *  eth_dft_router = NULL;
     char *  eth_ra_daemon = NULL;
     char *  eth_addr_autoconf = NULL;

     char *  local_addr_rewrite = NULL;
     char *  rpl_filter = NULL;
     char *  ndp_filter = NULL;

     char *  rpl_version = NULL;

     int file_nb;

     while((c = getopt_long(argc, argv, "h", long_options, &option_index)) != -1) {
	  switch(c) {
	  case 'h':
	       help(argv[0]);
	       break;

	  case new_nvm_action:
	       new_nvm_file = 1;
	       break;

	  case update_nvm_action:
	       update_nvm_file = 1;
	       break;

	  case print_nvm_action:
	       print_nvm_file = 1;
	       break;

	  case channel_option:
	       channel = optarg;
	       break;

	  case rime_addr_option:
	       rime_addr = optarg;
	       break;
	  case wsn_prefix_option:
	       wsn_prefix = optarg;
	       break;
	  case wsn_ip_option:
	       wsn_ip_addr = optarg;
	       break;
	  case wsn_accept_ra_option:
	       wsn_accept_ra = optarg;
	       break;
	  case wsn_addr_autoconf_option:
	       wsn_addr_autoconf = optarg;
	       break;

	  case eth_mac_option:
	       eth_mac_addr = optarg;
	       break;
	  case eth_net_prefix_option:
	       eth_net_prefix = optarg;
	       break;
	  case eth_ip_option:
	       eth_ip_addr = optarg;
	       break;
	  case eth_dft_router_option:
	       eth_dft_router = optarg;
	       break;

	  case eth_ra_daemon_option:
	       eth_ra_daemon = optarg;
	       break;
	  case eth_addr_autoconf_option:
	       eth_addr_autoconf = optarg;
	       break;

	  case local_addr_rewrite_option:
	       local_addr_rewrite = optarg;
	       break;
	  case rpl_filter_option:
	       rpl_filter = optarg;
	       break;

	  case rpl_version_option:
	       rpl_version = optarg;
	       break;

	  case fit_option:
	       fit = 1;
	       break;
	 
	  default:
	       usage(argv[0]);
	  }
     }
     file_nb = argc - optind;

     if ( new_nvm_file ) {
	  if( file_nb == 1 ) {
	       dest_nvm_file = argv[optind];
	  } else {
	       usage(argv[0]);
	  }
     } else if ( update_nvm_file ){
	  if( file_nb == 1 ) {
	       source_nvm_file = argv[optind];
	       dest_nvm_file = argv[optind];
	  } else if ( file_nb == 2 ) {
	       source_nvm_file = argv[optind];
	       dest_nvm_file = argv[optind + 1];
	  } else {
	       usage(argv[0]);
	  }
     } else if ( print_nvm_file ) {
	  if( file_nb == 1 ) {
	       source_nvm_file = argv[optind];
	  } else {
	       usage(argv[0]);
	  }
     } else {
	  usage(argv[0]);
     }

     if ( source_nvm_file ) {
	  load_nvm_file( source_nvm_file );
     } else {
	  create_empty_nvm();
     }

     if ( update_nvm_file || new_nvm_file ) {
	  if( channel ) {
	       nvm_params->channel = intconv( "channel", channel );
	  }
	  if( rime_addr ) {
	       euiconv( "wsn-mac", rime_addr, 8, nvm_params->rime_addr );
	  }
	  if ( wsn_prefix) {
	       ipaddrconv( "wsn-prefix", wsn_prefix, nvm_params->wsn_net_prefix );
	  }
	  if( wsn_ip_addr ) {
	       ipaddrconv("wsn-ip", wsn_ip_addr, nvm_params->wsn_ip_addr );
	  }
	  if ( wsn_accept_ra ) {
	       mode_update( CETIC_MODE_WAIT_RA_MASK, boolconv( "wsn-accept-ra", wsn_accept_ra) );
	  }
	  if ( wsn_addr_autoconf ) {
	       mode_update( CETIC_MODE_WSN_AUTOCONF, boolconv( "wsn-ip-autoconf", wsn_addr_autoconf) );
	  }

	  if( eth_mac_addr ) {
	       euiconv( "eth-mac", eth_mac_addr, 6, nvm_params->eth_mac_addr );
	  }
	  if( eth_net_prefix ) {
	       ipaddrconv("eth-prefix", eth_net_prefix, nvm_params->eth_net_prefix );
	  }
	  if( eth_ip_addr ) {
	       ipaddrconv("eth-ip", eth_ip_addr, nvm_params->eth_ip_addr );
	  }
	  if( eth_dft_router ) {
	       ipaddrconv("dft-router", eth_dft_router, nvm_params->eth_dft_router );
	  }
	  if ( eth_ra_daemon ) {
	       mode_update( CETIC_MODE_ROUTER_SEND_CONFIG, boolconv( "eth-ra-daemon", eth_ra_daemon) );
	  }
	  if ( eth_addr_autoconf ) {
	       mode_update( CETIC_MODE_ETH_MANUAL, boolconvinv( "eth-ip-autoconf", eth_addr_autoconf) );
	  }

	  if ( local_addr_rewrite ) {
	       mode_update( CETIC_MODE_REWRITE_ADDR_MASK, boolconv( "addr-rewrite", local_addr_rewrite) );
	  }
	  if ( rpl_filter ) {
	       mode_update( CETIC_MODE_FILTER_RPL_MASK, boolconv( "filter-rpl", rpl_filter) );
	  }
	  if ( ndp_filter ) {
	       mode_update( CETIC_MODE_FILTER_NDP_MASK, boolconv( "filter-ndp", ndp_filter) );
	  }

	  if( rpl_version ) {
	       nvm_params->rpl_version_id = intconv( "rpl-version", rpl_version );
	  }

     } else if ( print_nvm_file ) {
	  print_nvm();
     } else {
	  usage(argv[0]);
     }

     if ( dest_nvm_file ) {
	  print_nvm();
	  store_nvm_file( dest_nvm_file, fit );
     }
}
