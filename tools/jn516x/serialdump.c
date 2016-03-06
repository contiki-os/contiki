#define _GNU_SOURCE
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BAUDRATE B115200
#define BAUDRATE_S "115200"
#ifdef linux
#define MODEMDEVICE "/dev/ttyS0"
#else
#define MODEMDEVICE "/dev/com1"
#endif /* linux */

#define SLIP_END      0300
#define SLIP_ESC      0333
#define SLIP_ESC_END  0334
#define SLIP_ESC_ESC  0335

#define CSNA_INIT     0x01

#define BUFSIZE         40
#define HCOLS           20
#define ICOLS           18

#define MODE_START_DATE  0
#define MODE_DATE        1
#define MODE_START_TEXT  2
#define MODE_TEXT        3
#define MODE_INT         4
#define MODE_HEX         5
#define MODE_SLIP_AUTO   6
#define MODE_SLIP        7
#define MODE_SLIP_HIDE   8

static unsigned char rxbuf[2048];

static int
usage(int result)
{
  printf("Usage: serialdump [-x] [-s[on]] [-i] [-bSPEED] [SERIALDEVICE]\n");
  printf("       -x for hexadecimal output\n");
  printf("       -i for decimal output\n");
  printf("       -s for automatic SLIP mode\n");
  printf("       -so for SLIP only mode (all data is SLIP packets)\n");
  printf("       -sn to hide SLIP packages\n");
  printf("       -T[format] to add time for each text line\n");
  printf("         (see man page for strftime() for format description)\n");
  return result;
}

static void
print_hex_line(unsigned char *prefix, unsigned char *outbuf, int index)
{
  int i;

  printf("\r%s", prefix);
  for(i = 0; i < index; i++) {
    if((i % 4) == 0) {
      printf(" ");
    }
    printf("%02X", outbuf[i] & 0xFF);
  }
  printf("  ");
  for(i = index; i < HCOLS; i++) {
    if((i % 4) == 0) {
      printf(" ");
    }
    printf("  ");
  }
  for(i = 0; i < index; i++) {
    if(outbuf[i] < 30 || outbuf[i] > 126) {
      printf(".");
    } else {
      printf("%c", outbuf[i]);
    }
  }
}

int
main(int argc, char **argv)
{
  struct termios options;
  fd_set mask, smask;
  int fd;
  speed_t speed = BAUDRATE;
  char *speedname = BAUDRATE_S;
  char *device = MODEMDEVICE;
  char *timeformat = NULL;
  unsigned char buf[BUFSIZE], outbuf[HCOLS];
  unsigned char mode = MODE_START_TEXT;
  int nfound, flags = 0;
  unsigned char lastc = '\0';

  int index = 1;
  while(index < argc) {
    if(argv[index][0] == '-') {
      switch(argv[index][1]) {
        case 'b':
          /* set speed */
          if(strcmp(&argv[index][2], "38400") == 0) {
            speed = B38400;
            speedname = "38400";
          } else if(strcmp(&argv[index][2], "19200") == 0) {
            speed = B19200;
            speedname = "19200";
          } else if(strcmp(&argv[index][2], "57600") == 0) {
            speed = B57600;
            speedname = "57600";
          } else if(strcmp(&argv[index][2], "115200") == 0) {
            speed = B115200;
            speedname = "115200";
          } else if(strcmp(&argv[index][2], "230400") == 0) {
            speed = B230400;
            speedname = "230400";
          } else if(strcmp(&argv[index][2], "460800") == 0) {
            speed = B460800;
            speedname = "460800";
          } else if(strcmp(&argv[index][2], "500000") == 0) {
            speed = B500000;
            speedname = "500000";
          } else if(strcmp(&argv[index][2], "576000") == 0) {
            speed = B576000;
            speedname = "576000";
          } else if(strcmp(&argv[index][2], "921600") == 0) {
            speed = B921600;
            speedname = "921600";
          } else if(strcmp(&argv[index][2], "1000000") == 0) {
            speed = B1000000;
            speedname = "1000000";
          } else {
            fprintf(stderr, "unsupported speed: %s\n", &argv[index][2]);
            return usage(1);
          }
          break;
        case 'x':
          mode = MODE_HEX;
          break;
        case 'i':
          mode = MODE_INT;
          break;
        case 's':
          switch(argv[index][2]) {
            case 'n':
              mode = MODE_SLIP_HIDE;
              break;
            case 'o':
              mode = MODE_SLIP;
              break;
            default:
              mode = MODE_SLIP_AUTO;
              break;
          }
          break;
        case 'T':
          if(strlen(&argv[index][2]) == 0) {
            timeformat = "%Y-%m-%d %H:%M:%S";
          } else {
            timeformat = &argv[index][2];
          }
          mode = MODE_START_DATE;
          break;
        case 'h':
          return usage(0);
        default:
          fprintf(stderr, "unknown option '%c'\n", argv[index][1]);
          return usage(1);
      }
      index++;
    } else {
      device = argv[index++];
      if(index < argc) {
        fprintf(stderr, "too many arguments\n");
        return usage(1);
      }
    }
  }
  fprintf(stderr, "connecting to %s (%s)", device, speedname);

#ifdef O_SYNC
  fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY /*| O_DIRECT*/ | O_SYNC);
  if(fd < 0 && errno == EINVAL){ // O_SYNC not supported (e.g. raspberian)
    fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY | O_DIRECT);
  }
#else
  fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY | O_SYNC );
#endif
  if(fd < 0) {
    fprintf(stderr, "\n");
    perror("open");
    exit(-1);
  }
  fprintf(stderr, " [OK]\n");

  if(fcntl(fd, F_SETFL, 0) < 0) {
    perror("could not set fcntl");
    exit(-1);
  }

  if(tcgetattr(fd, &options) < 0) {
    perror("could not get options");
    exit(-1);
  }
  /*   fprintf(stderr, "serial options set\n"); */
  cfsetispeed(&options, speed);
  cfsetospeed(&options, speed);
  /* Enable the receiver and set local mode */
  options.c_cflag |= (CLOCAL | CREAD);
  /* Mask the character size bits and turn off (odd) parity */
  options.c_cflag &= ~(CSIZE | PARENB | PARODD);
  /* Select 8 data bits */
  options.c_cflag |= CS8;

  /* Raw input */
  options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
  /* Raw output */
  options.c_oflag &= ~OPOST;

  if(tcsetattr(fd, TCSANOW, &options) < 0) {
    perror("could not set options");
    exit(-1);
  }

  /* Make read() return immediately */
  /*    if (fcntl(fd, F_SETFL, FNDELAY) < 0) { */
  /*      perror("\ncould not set fcntl"); */
  /*      exit(-1); */
  /*    } */

  FD_ZERO(&mask);
  FD_SET(fd, &mask);
  FD_SET(fileno(stdin), &mask);

  index = 0;
  for(;;) {
    smask = mask;
    nfound = select(FD_SETSIZE, &smask, (fd_set *) 0, (fd_set *) 0, (struct timeval *) 0);
    if(nfound < 0) {
      if(errno == EINTR) {
        fprintf(stderr, "interrupted system call\n");
        continue;
      }
      /* something is very wrong! */
      perror("select");
      exit(1);
    }

    if(FD_ISSET(fileno(stdin), &smask)) {
      /* data from standard in */
      int n = read(fileno(stdin), buf, sizeof(buf));
      if(n < 0) {
        perror("could not read");
        exit(-1);
      } else if(n > 0) {
        /* because commands might need parameters, lines needs to be
         separated which means the terminating LF must be sent */
        /*   while(n > 0 && buf[n - 1] < 32) { */
        /*     n--; */
        /*   } */
        if(n > 0) {
          int i;
          /*    fprintf(stderr, "SEND %d bytes\n", n);*/
          /* write slowly */
          for(i = 0; i < n; i++) {
            if(write(fd, &buf[i], 1) <= 0) {
              perror("write");
              exit(1);
            } else {
              fflush(NULL);
              usleep(6000);
            }
          }
        }
      } else {
        /* End of input, exit. */
        exit(0);
      }
    }

    if(FD_ISSET(fd, &smask)) {
      int i, j, n = read(fd, buf, sizeof(buf));
      if(n < 0) {
        perror("could not read");
        exit(-1);
      }

      for(i = 0; i < n; i++) {
        switch(mode) {
          case MODE_START_TEXT:
          case MODE_TEXT:
            printf("%c", buf[i]);
            break;
          case MODE_START_DATE: {
            time_t t;
            t = time(&t);
            strftime(outbuf, HCOLS, timeformat, localtime(&t));
            printf("%s|", outbuf);
            mode = MODE_DATE;
          }
            /* continue into the MODE_DATE */
          case MODE_DATE:
            printf("%c", buf[i]);
            if(buf[i] == '\n') {
              mode = MODE_START_DATE;
            }
            break;
          case MODE_INT:
            printf("%03d ", buf[i]);
            if(++index >= ICOLS) {
              index = 0;
              printf("\n");
            }
            break;
          case MODE_HEX:
            rxbuf[index++] = buf[i];
            if(index >= HCOLS) {
              print_hex_line("", rxbuf, index);
              index = 0;
              printf("\n");
            }
            break;

          case MODE_SLIP_AUTO:
          case MODE_SLIP_HIDE:
            if(!flags && (buf[i] != SLIP_END)) {
              /* Not a SLIP packet? */
              printf("%c", buf[i]);
              break;
            }
            /* continue to slip only mode */
          case MODE_SLIP:
            switch(buf[i]) {
              case SLIP_ESC:
                lastc = SLIP_ESC;
                break;

              case SLIP_END:
                if(index > 0) {
                  if(flags != 2 && mode != MODE_SLIP_HIDE) {
                    /* not overflowed: show packet */
                    print_hex_line("SLIP: ", rxbuf, index > HCOLS ? HCOLS : index);
                    printf("\n");
                  }
                  lastc = '\0';
                  index = 0;
                  flags = 0;
                } else {
                  flags = !flags;
                }
                break;

              default:
                if(lastc == SLIP_ESC) {
                  lastc = '\0';

                  /* Previous read byte was an escape byte, so this byte will be
                   interpreted differently from others. */
                  switch(buf[i]) {
                    case SLIP_ESC_END:
                      buf[i] = SLIP_END;
                      break;
                    case SLIP_ESC_ESC:
                      buf[i] = SLIP_ESC;
                      break;
                  }
                }

                rxbuf[index++] = buf[i];
                if(index >= sizeof(rxbuf)) {
                  fprintf(stderr, "**** slip overflow\n");
                  index = 0;
                  flags = 2;
                }
                break;
            }
            break;
        }
      }

      /* after processing for some output modes */
      if(index > 0) {
        switch(mode) {
          case MODE_HEX:
            print_hex_line("", rxbuf, index);
            break;
        }
      }
      fflush(stdout);
    }
  }
}
