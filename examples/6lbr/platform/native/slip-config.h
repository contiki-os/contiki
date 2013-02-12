#ifndef SLIP_CONFIG_H_
#define SLIP_CONFIG_H_

#include <stdint.h>
#include <termios.h>

extern int slip_config_verbose;
extern int slip_config_flowcontrol;
extern int slip_config_timestamp;
extern const char *slip_config_siodev;
extern const char *slip_config_host ;
extern const char *slip_config_port;
extern char slip_config_tundev[32];
extern uint16_t slip_config_basedelay;
extern char const * default_nvm_file;
extern uint8_t use_raw_ethernet;
extern uint8_t ethernet_has_fcs;
extern speed_t slip_config_b_rate;

#endif
