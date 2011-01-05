#ifndef CC1020_INTERNAL_H
#define CC1020_INTERNAL_H

#include <signal.h>

#define CC1020_MAIN		0x00
#define CC1020_INTERFACE	0x01
#define CC1020_RESET		0x02
#define CC1020_SEQUENCING	0x03
#define CC1020_FREQ_2A		0x04
#define CC1020_FREQ_1A		0x05
#define CC1020_FREQ_0A		0x06
#define CC1020_CLOCK_A		0x07
#define CC1020_FREQ_2B		0x08
#define CC1020_FREQ_1B		0x09
#define CC1020_FREQ_0B		0x0A
#define CC1020_CLOCK_B		0x0B
#define CC1020_VCO		0x0C
#define CC1020_MODEM		0x0D
#define CC1020_DEVIATION	0x0E
#define CC1020_AFC_CONTROL	0x0F
#define CC1020_FILTER		0x10
#define CC1020_VGA1		0x11
#define CC1020_VGA2		0x12
#define CC1020_VGA3		0x13
#define CC1020_VGA4		0x14
#define CC1020_LOCK		0x15
#define CC1020_FRONTEND		0x16
#define CC1020_ANALOG		0x17
#define CC1020_BUFF_SWING	0x18
#define CC1020_BUFF_CURRENT	0x19
#define CC1020_PLL_BW		0x1A
#define CC1020_CALIBRATE	0x1B
#define CC1020_PA_POWER		0x1C
#define CC1020_MATCH		0x1D
#define CC1020_PHASE_COMP	0x1E
#define CC1020_GAIN_COMP	0x1F
#define CC1020_POWERDOWN	0x20
#define CC1020_TEST1		0x21
#define CC1020_TEST2		0x22
#define CC1020_TEST3		0x23
#define CC1020_TEST4		0x24
#define CC1020_TEST5		0x25
#define CC1020_TEST6		0x26
#define CC1020_TEST7		0x27
#define CC1020_STATUS		0x40
#define CC1020_RESET_DONE	0x41
#define CC1020_RSS		0x42
#define CC1020_AFC		0x43
#define CC1020_GAUSS_FILTER	0x44
#define CC1020_STATUS1		0x45
#define CC1020_STATUS2		0x46
#define CC1020_STATUS3		0x47
#define CC1020_STATUS4		0x48
#define CC1020_STATUS5		0x49
#define CC1020_STATUS6		0x4A
#define CC1020_STATUS7		0x4B

/* Flags for the MAIN register. */
#define RESET_N			1
#define BIAS_PD			(1<<1)
#define XOSC_PD			(1<<2)
#define FS_PD			(1<<3)
#define PD_MODE_1		(1<<4)
#define PD_MODE_2		(1<<5)
#define F_REG			(1<<6)
#define RXTX			(1<<7)

/* In power up mode, the MAIN register modifies some flags to the following. */
#define SEQ_PD			(1<<1)
#define SEQ_CAL_1		(1<<2)
#define SEQ_CAL_2		(1<<3)

// For CC1020_STATUS
#define CARRIER_SENSE		0x08
#define LOCK_CONTINUOUS		0x10
#define LOCK_INSTANT		0x20
#define SEQ_ERROR		0x40
#define CAL_COMPLETE		0x80

#define PA_POWER		0x0F	// initial default for output power
#define LOCK_NOK		0x00
#define LOCK_OK			0x01
#define LOCK_RECAL_OK		0x02
#define CAL_TIMEOUT		0x7FFE
#define LOCK_TIMEOUT		0x7FFE
#define RESET_TIMEOUT		0x7FFE
#define TX_CURRENT 		0x87
#define RX_CURRENT 		0x86

// CC1020 driver configuration

// PDI (Data in) is on P21
#define PDO		(P2IN & 0x01)

// PSEL is on P30 and low active
#define PSEL_ON		do { P3OUT &= ~0x01; } while(0)
#define PSEL_OFF	do { P3OUT |=  0x01; } while(0)
#define PCLK_HIGH	do { P2OUT |=  0x08; }  while(0)
#define PCLK_LOW	do { P2OUT &= ~0x08; }  while(0)

// PDO (Data out) is on P22
#define PDI_HIGH	do { P2OUT |=  0x02; }  while(0)

#define PDI_LOW		do { P2OUT &= ~0x02; }  while(0)

// Enable power for LNA (P24, low-active)
#define LNA_POWER_ON()	do { P2OUT &= ~0x10; }  while(0)

#define LNA_POWER_OFF() do { P2OUT |=  0x10; }  while(0)

#define CC_LOCK		(P2IN & 0x04)

#define DISABLE_RX_IRQ()				\
		do { IE1 &= ~(URXIE0); } while(0)

#define ENABLE_RX_IRQ()					\
	do { IFG1 &= ~URXIFG0; IE1 |= URXIE0; } while(0)

#define ACK_TIMEOUT_115		4	// In RADIO_STROKE ticks
#define ACK_TIMEOUT_19		16

#define MHZ_869525      	1

const uint8_t cc1020_config_19200[41] = {
  0x01,   // 0x00, MAIN
  0x0F,   // 0x01, INTERFACE
  0xFF,   // 0x02, RESET
  0x8F,   // 0x03, SEQUENCING
  // 869.525 at 50kHz
  0x3A,   // 0x04, FREQ_2A
  0x32,   // 0x05, FREQ_1A
  0x97,   // 0x06, FREQ_0A // 19200
  0x38,   // 0x07, CLOCK_A // 19200
  0x3A,   // 0x08, FREQ_2B
  0x37,   // 0x09, FREQ_1B
  0xEB,   // 0x0A, FREQ_0B // 19200
  0x38,   // 0x0B, CLOCK_B // 19200
  0x44,   // 0x0C, VCO     44
  0x51,   // 0x0D, MODEM   Manchester
  0x2B,   // 0x0E, DEVIATION // FSK
  0x4C,   // 0x0F, AFC_CONTROL Ruetten 0xCC
  0x25,   // 0x10, FILTER Bandwith 51.2 kHz i.e. channel spacing 100kHz
  0x61,   // 0x11, VGA1
  0x55,   // 0x12, VGA2
  0x2D,   // 0x13, VGA3
  0x37,   // 0x14, VGA4 // 0x29, VGA4 ADJUSTED CS to 23!
  0x40,   // 0x15, LOCK is Carrier SENSE
  0x76,   // 0x16, FRONTEND
  0x87,   // 0x17, ANALOG, RX=86/TX=87
  0x10,   // 0x18, BUFF_SWING
  0x25,   // 0x19, BUFF_CURRENT
  0xAE,   // 0x1A, PLL_BW
  0x34,   // 0x1B, CALIBRATE
  PA_POWER, // 0x1C, PA_POWER AN025 = 0xA0
  0xF0,   // 0x1D, MATCH
  0x00,   // 0x1E, PHASE_COMP
  0x00,   // 0x1F, GAIN_COMP
  0x00,   // 0x20, POWERDOWN
  0x4d,   // 0x4d,  // 0x21,
  0x10,   // 0x10,  // 0x22,
  0x06,   // 0x06,  // 0x23,
  0x00,   // 0x00,  // 0x24,
  0x40,   // 0x40,  // 0x25,
  0x00,   // 0x00,  // 0x26,
  0x00,   // 0x00,   // 0x27,
  // Not in real config of chipCon from here!!!
  ACK_TIMEOUT_19
};

const uint8_t cc1020_config_115200[41] = {
  0x01,   // 0x00, MAIN
  0x0F,   // 0x01, INTERFACE
  0xFF,   // 0x02, RESET
  0x8F,   // 0x03, SEQUENCING
  // 869.525 at 200kHz
  0x3A,   // 0x04, FREQ_2A
  0x32,   // 0x05, FREQ_1A
  0x97,   // 0x06, FREQ_0A // 19200
  0x29,   // 0x07, CLOCK_A // 19200
  0x3A,   // 0x08, FREQ_2B
  0x37,   // 0x09, FREQ_1B
  0xEB,   // 0x0A, FREQ_0B // 19200
  0x29,   // 0x0B, CLOCK_B // 19200
  0x44,   // 0x0C, VCO 44
  0x51,   // 0x0D, MODEM Manchester
  0x58,   // 0x0E, DEVIATION // FSK
  0x4C,   // 0x0F, AFC_CONTROL Ruetten 0xCC
  0x80,   // 0x10, FILTER Bandwith 307.2kHz, i.e. channel spacing 500 kHz
  0x61,   // 0x11, VGA1
  0x57,   // 0x12, VGA2
  0x30,   // 0x13, VGA3
  0x35,   // 0x14, VGA4
  0x20,   // 0x15, LOCK is Carrier SENSE
  0x76,   // 0x16, FRONTEND
  0x87,   // 0x17, ANALOG, RX=86/TX=87
  0x10,   // 0x18, BUFF_SWING
  0x25,   // 0x19, BUFF_CURRENT
  0xAE,   // 0x1A, PLL_BW
  0x34,   // 0x1B, CALIBRATE
  PA_POWER, // 0x1C, PA_POWER AN025 = 0xA0
  0xF0,   // 0x1D, MATCH
  0x00,   // 0x1E, PHASE_COMP
  0x00,   // 0x1F, GAIN_COMP
  0x00,   // 0x20, POWERDOWN
  0x4d,   // 0x21,
  0x10,   // 0x22,
  0x06,   // 0x23,
  0x00,   // 0x24,
  0x40,   // 0x25,
  0x00,   // 0x26,
  0x00,   // 0x27,             
  // Not in real config of chipCon from here!!!
  ACK_TIMEOUT_115
};

/// cc1020 state
enum cc1020_state {
  CC1020_OFF = 0,
  CC1020_RX  = 0x01,
  CC1020_TX  = 0x02,
  
  CC1020_RX_SEARCHING = 0x10,		// searching for preamble + sync word
  CC1020_RX_RECEIVING = 0x20,		// receiving bytes
  CC1020_RX_PROCESSING = 0x40,		// processing data in buffer
  
  CC1020_OP_STATE = 0x73,
  
  CC1020_TURN_OFF = 0x80,
};

#define CC1020_SET_OPSTATE(opstate)		cc1020_state = ((cc1020_state & ~CC1020_OP_STATE) | (opstate))

/******************************************************************************
 * @name	Packet specification
 * @{
 */

// header: number of bytes in packet including header
struct cc1020_header {
  uint8_t  pad;
  uint8_t  length;
} __attribute__((packed));


#define CC1020_BUFFERSIZE	128


#define PREAMBLE_SIZE		6
#define PREAMBLE		0xAA

#define SYNCWORD_SIZE		2
#define HDR_SIZE		(sizeof (struct cc1020_header))

#define CRC_SIZE		2

#define TAIL_SIZE		2
#define	TAIL			0xFA


///@}

#endif /* CC1020_INTERNAL_H */
