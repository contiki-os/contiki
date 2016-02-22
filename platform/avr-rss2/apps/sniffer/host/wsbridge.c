/*******************************************************************
    Copyright (C) 2009 FreakLabs
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name of the the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS'' AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.

    Originally written by Christopher Wang aka Akiba.
    Please post support questions to the FreakLabs forum.

 *******************************************************************/
/*!
    FreakLabs Freakduino/Wireshark Bridge

    This program allows data from the Freakduino to be piped into wireshark.
    When the sniffer firmware is loaded into the Freakduino, then the Freakduino
    will be in promiscuous mode and will just dump any frames it sees. This
    program takes the frame dump and sends it into Wireshark for analysis. The
    global header is already set up to inform wireshark that the link layer for
    all frames will be in IEEE 802.15.4 format. After that, it is up to the user
    to choose any higher layer protocols to decode above 802.15.4 via the
    wireshark "enable protocols" menu.
 */

/*

   Modified for sensniff format. Magic[4] + Vers[1] + CMD[1] + len[1]
   Robert Olsson <roolss@kth.se> 2015-11-10

 */

/**************************************************************************/
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>

#define PORTBUFSIZE     32
#define BUFSIZE         1024
#define PACKET_FCS      2
#define DEBUG           1
#define PIPENAME        "/tmp/wireshark"
#define BAUDRATE        B38400

enum FSM {
  START_CAPTURE,
  PACKET_CAPTURE
};

static int FD_pipe = -1;
static int FD_com = -1;
static uint8_t port_buf[PORTBUFSIZE];
static uint8_t circ_buf[BUFSIZE];
static uint16_t rd_idx = 0;
static uint16_t wr_idx = 0;
static uint8_t len;
static uint8_t state = START_CAPTURE;
static uint8_t file_write = 0;
static fd_set fds;
static const uint8_t magic[] = { 0xC1, 0x1F, 0xFE, 0x72 };

/**************************************************************************/
/*!
    Open the serial port that we'll be communicating with the Freakduino (sniffer)
    through.
 */
/**************************************************************************/

int
serial_open(char *portname)
{
  int baud = B38400;
  int fd; /* file descriptor for the serial port */
  struct termios tc, tp;

  fd = open(portname, O_RDONLY | O_NOCTTY | O_NDELAY);

  if(fd == -1) { /* if open is unsucessful */
    printf("serial_open: Unable to open %s.\n", portname);
    exit(-1);
  }

  fcntl(fd, F_GETFL);
  fcntl(fd, F_SETFL, O_RDWR);

  tp.c_cflag = baud | CS8 | CLOCAL | CREAD;
  tp.c_oflag = 0; /* Raw Input */
  tp.c_lflag = 0; /* No conoical */
  tp.c_oflag &= ~(OLCUC | OCRNL | ONOCR | ONLRET | OFILL | OFDEL | NLDLY | CRDLY);

  /* ignore CR, ignore parity */
  tp.c_iflag = ~(IGNBRK | PARMRK | INPCK | INLCR | IUCLC | IXOFF) |
    BRKINT | IGNPAR | ICRNL | IXON | ISIG | ICANON;

  tp.c_lflag &= ~(ECHO | ECHONL);
  tp.c_oflag = 0; /* Raw Input */
  tp.c_iflag &= ~ISTRIP;

  tcflush(fd, TCIFLUSH);

  cfsetospeed(&tp, baud);
  cfsetispeed(&tp, baud);

  if(tcsetattr(fd, TCSANOW, &tp) < 0) {
    perror("Couldn't set term attributes");
    return -1;
  }
  return fd;
}
/**************************************************************************/
/*!
    Create the named pipe that we will be communicating with wireshark through.
 */
/**************************************************************************/
static void
named_pipe_create(char *name)
{
  int rv = 0;
  rv = mkfifo(name, 0666);
  if((rv == -1) && (errno != EEXIST)) {
    perror("Error creating named pipe");
    exit(1);
  }

  FD_pipe = open(name, O_WRONLY);

  if(FD_pipe == -1) {
    perror("Error connecting to named pipe");
    exit(1);
  }
}
/**************************************************************************/
/*!
    Write data to the pipe
 */
/**************************************************************************/
size_t
data_write(const void *ptr, size_t size)
{
  ssize_t bytes = 0;
  if(FD_pipe != -1) {
    bytes = write(FD_pipe, ptr, size);
  }
}
/**************************************************************************/
/*!
    Write the global header to wireshark. This is only done once at the
    beginning of the capture.
 */
/**************************************************************************/
static void
write_global_hdr()
{
  uint32_t magic_number = 0xa1b2c3d4;   /* magic number */
  uint16_t version_major = 2;   /* major version number */
  uint16_t version_minor = 4;   /* minor version number */
  int32_t thiszone = 0;         /* GMT to local correction */
  uint32_t sigfigs = 0;         /* accuracy of timestamps */
  uint32_t snaplen = 65535;     /* max length of captured packets, in octets */
  uint32_t network = 195;       /* data link type (DLT) - IEEE 802.15.4 */

  data_write(&magic_number, sizeof(magic_number));
  data_write(&version_major, sizeof(version_major));
  data_write(&version_minor, sizeof(version_minor));
  data_write(&thiszone, sizeof(thiszone));
  data_write(&sigfigs, sizeof(sigfigs));
  data_write(&snaplen, sizeof(snaplen));
  data_write(&network, sizeof(network));
}
/**************************************************************************/
/*!
    Write the frame header into wireshark. This is required for the libpcap
    format and informs wireshark that a new frame is coming.
 */
/**************************************************************************/
static void
write_frame_hdr(uint8_t len)
{
  uint32_t ts_sec;      /* timestamp seconds */
  uint32_t ts_usec;     /* timestamp microseconds */
  uint32_t incl_len;    /* number of octets of packet saved in file */
  uint32_t orig_len;    /* actual length of packet */
  struct timeval tv;

  gettimeofday(&tv, NULL);
  ts_sec = tv.tv_sec;
  ts_usec = tv.tv_usec;
  incl_len = len;
  orig_len = len + PACKET_FCS;

  data_write(&ts_sec, sizeof(ts_sec));
  data_write(&ts_usec, sizeof(ts_usec));
  data_write(&incl_len, sizeof(incl_len));
  data_write(&orig_len, sizeof(orig_len));
}
/**************************************************************************/
/*!
    Write one frame into wireshark (via the pipe).
 */
/**************************************************************************/
static void
write_frame(uint8_t frame_len)
{
  uint8_t i;

  /* actual frame length for wireshark should not include FCS */
  frame_len -= PACKET_FCS;

  /* write header to inform WS that new frame has arrived */
  write_frame_hdr(frame_len);

  /* bump rd_idx. we don't want to write the length byte */
  rd_idx = (rd_idx + 1) % BUFSIZE;

  /* write frame into wireshark */
  for(i = 0; i < frame_len; i++) {
    data_write(&circ_buf[rd_idx], 1);
    rd_idx = (rd_idx + 1) % BUFSIZE;
  }

  /* bump rd_idx. we're not using the trailing FCS value */
  rd_idx = (rd_idx + 1) % BUFSIZE;
}
/**************************************************************************/
/*!
    Calculate total number of bytes in buffer.
 */
/**************************************************************************/
static uint16_t
calc_bytes_in_buf()
{
  if(rd_idx > wr_idx) {
    /* read index is greater than write. we must have wrapped around */
    return BUFSIZE - (rd_idx - wr_idx);
  } else {
    return wr_idx - rd_idx;
  }
}
/**************************************************************************/
/*!
    Deal with any received signals. This includes ctrl-C to stop the program.
 */
/**************************************************************************/
static void
sig_int(int signo)
{
  (void)signo;
  if(FD_pipe != -1) {
    printf("\nClosing pipe.\n");
    close(FD_pipe);
  }

  if(FD_com != -1) {
    printf("\nClosing serial port.\n");
    close(FD_com);
  }

  printf("\nSignal captured and devices shut down.\n");

  exit(0);
}
/**************************************************************************/
/*!
    Init the signals we'll be checking for.
 */
/**************************************************************************/
static void
signal_init(void)
{
  signal(SIGINT, sig_int);
  signal(SIGHUP, sig_int);
  signal(SIGTERM, sig_int);
}
int got;
int debug;

/**************************************************************************/
/*!
    Here's the meat of the code.
 */
/**************************************************************************/
int
main(int argc, char *argv[])
{
  int nbytes;
  uint8_t i;

  got = 0;

  /* capture any signals that will terminate program */
  signal_init();

  /* make sure the COM port is specified */
  if(argc == 2) {
    /* open the COM port */
    if((FD_com = serial_open(argv[1])) == -1) {
      printf("Serial port not opened.\n");
      return 0;
    } else {
      /* set up the select statement for the COM port. */
      FD_ZERO(&fds);
      FD_SET(FD_com, &fds);

      printf("Serial port connected. Waiting for wireshark connection.\n");
      printf("Open wireshark and connect to local interface: %s\n", PIPENAME);
    }
  } else {
    printf("Usage: wsbridge <portname>.\n");
    return 0;
  }

  /* create and open pipe for wireshark */
  named_pipe_create(PIPENAME);

  /* wait for wireshark to connect to pipe. Once wireshark */
  /* connects, then the global header will be written to it. */
  if(FD_pipe != -1) {
    write_global_hdr();
    printf("Client connected to pipe.\n");
  }

  for(;;) {
    uint16_t bytes_in_buf;
    uint8_t frame_len, byte_ctr;

    /* block until there is data in the serial port */
    select(FD_com + 1, &fds, NULL, NULL, NULL);
    if(FD_ISSET(FD_com, &fds)) {
      int ii;
      /* wait for data to come in on the serial port */
      if((nbytes = read(FD_com, port_buf, PORTBUFSIZE)) > 0) {

        if(debug) {
          uint8_t p;
          printf("read nbytes=%d\n", nbytes);
          for(i = 0; i < nbytes; i++) {
            printf(" %02X", port_buf[i]);
          }
          printf("\n");
        }
        /* write data to circular buffer. loop through all received bytes */
        for(i = 0; i < nbytes; i++) {
          switch(state) {
          case START_CAPTURE:
            /* new frame starting */
            if((got == 0) && (port_buf[i] == magic[0])) {
              got = 1;
            } else if((got == 1) && (port_buf[i] == magic[1])) {
              got = 2;
            } else if((got == 2) && (port_buf[i] == magic[2])) {
              got = 3;
            } else if((got == 3) && (port_buf[i] == magic[3])) {
              got = 4;
              if(debug) {
                printf("GOT MAGIC i=%d\n", i);
              }
            } else if((got == 4) && (port_buf[i] == 1)) {
              got = 5;
              if(debug) {
                printf("GOT VERSION i=%d\n", port_buf[i]);
              }
            } else if((got == 5) && (port_buf[i] == 0)) {
              got = 6;
              if(debug) {
                printf("GOT COMMAND i=%d\n", port_buf[i]);
              }
            } else if(got == 6) {
              len = port_buf[i];
              byte_ctr = 0;
              if(debug) {
                printf("Len = %02X.\n", len);
              }
              circ_buf[wr_idx] = len;
              wr_idx = (wr_idx + 1) % BUFSIZE;
              state = PACKET_CAPTURE;
            } else {
              got = 0;
            }
            break;

          case PACKET_CAPTURE:
            /* continue capturing bytes until end of frame */
            /* write data to circular buffer and increment index */

            circ_buf[wr_idx] = port_buf[i];
            /* ////printf("%02X ", circ_buf[wr_idx]); */

            wr_idx = (wr_idx + 1) % BUFSIZE;
            /* track number of received bytes. when received bytes */
            /* equals frame length, then restart state machine and */
            /* write bytes to wireshark */
            byte_ctr++;
            if(byte_ctr == (len - 1)) {
              state = START_CAPTURE;
              file_write = 1;
              /* printf("\n"); */
              got = 0;
            }
            break;
          }
          fflush(stdout);
        }
        /* at least one frame has been written. loop through circular buffer */
        /* and write out all completed frames */
        while(file_write) {
          /* capture frame length and check buffer to see if one or more frames */
          /* are available. */
          frame_len = circ_buf[rd_idx];
          bytes_in_buf = calc_bytes_in_buf();

          if(bytes_in_buf > frame_len) {
            /* if more than one frame is available, then write one frame to */
            /* wireshark and then see if any more are available. */
            write_frame(frame_len);
          } else if(bytes_in_buf == frame_len) {
            /* only one frame is available. write to wireshark and then quit */
            /* the loop */
            write_frame(frame_len);
            file_write = 0;
          } else {
            /* less than one frame is available. quit the loop and collect more */
            /* bytes. we normally should not get here. */
            file_write = 0;
          }
        }
      }
    }
  }
}
