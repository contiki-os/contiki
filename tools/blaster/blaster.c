/*
 * Copyright (c) 2014, Lars Schmertmann <SmallLars@t-online.de>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <uuid/uuid.h>
#include <libconfig.h>
#include <time.h>
#include <qrencode.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "blaster.h"

char *anschars = "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_-";

/* ---------------------------------------------------------------------------- */

FILE *openFile(const char *name, const char *appendix, const char *mode);
void writeStandardConfig();
void writeImg(FILE *file, unsigned char *data, int width);

/* ---------------------------------------------------------------------------- */

int
main(int nArgs, char **argv)
{
  unsigned int c, i, config;
  unsigned int buf[64];

  if(nArgs < 2) {
    writeStandardConfig();

    fprintf(stderr, "Missing parameter: ./blaster <config.cfg> [<config.cfg> <config.cfg> ...]\n");
    fprintf(stderr, "Configuration template was created ib config.cfg.\n");

    exit(EXIT_FAILURE);
  }

  /* qrdata = "UUID:PSK\0" */
  char qrdata[54];
  qrdata[36] = ':';
  qrdata[53] = '\0';

  config_t cfg;
  for(config = 1; config < nArgs; config++) {
    config_init(&cfg);
    config_setting_t *setting;
    const char *str_val;
    if(access(argv[config], F_OK) == 0) {
      config_read_file(&cfg, argv[config]);
    } else {
      fprintf(stderr, "Unable to read config file.\n");
      exit(EXIT_FAILURE);
    }
    fprintf(stdout, "Working on %s ... ", argv[config]);

    config_lookup_string(&cfg, "input", &str_val);
    FILE *in_bin = openFile(str_val, ".bin", "r");

    config_lookup_string(&cfg, "output", &str_val);
    FILE *out_bin = openFile(str_val, ".bin", "w");
    FILE *out_txt = openFile(str_val, ".txt", "w");
    FILE *out_pbm = openFile(str_val, ".pbm", "w");

    char output[131072];
    for(i = 8; (c = fgetc(in_bin)) != EOF; i++) {
      output[i] = (unsigned char)c;
    }
    /* Set original length of firmware in little endian format  ------------------- */
    unsigned int length = i - 8;
    memcpy(output + 4, (const void *)&length, 4);
    fprintf(out_txt, "Length: %u = 0x%08x\n", length, length);

    /* Fill additional flash with zeros for initialisation */
    for(; i < 0x1F000; i++) {
      output[i] = 0x00;
    }

    /* Example: Write an CoRE-Link-Answer for CoAP -------------------------------- */
    char *buffer = "</d/name>;rt=\"dev.info\";if=\"core.rp\","
                   "</d/model>;rt=\"dev.info\";if=\"core.rp\","
                   "</d/uuid>;rt=\"dev.info\";if=\"core.rp\"";
    memcpy(output + RES_D_CORE, buffer, LEN_D_CORE);

    /* Contiki configuration ------------------------------------------------------ */
    output[RES_CONFIG + 0] = 0x22;
    output[RES_CONFIG + 1] = 0x13;
    output[RES_CONFIG + 2] = 1;
    output[RES_CONFIG + 3] = 0;

    setting = config_lookup(&cfg, "eui");
    for(i = 0; i < 8; i++) {
      output[RES_CONFIG + 8 + i] = config_setting_get_int_elem(setting, 7 - i);
    }
    fprintf(out_txt,
            "EUI: %02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x\n",
            (uint8_t)output[RES_CONFIG + 15],
            (uint8_t)output[RES_CONFIG + 14],
            (uint8_t)output[RES_CONFIG + 13],
            (uint8_t)output[RES_CONFIG + 12],
            (uint8_t)output[RES_CONFIG + 11],
            (uint8_t)output[RES_CONFIG + 10],
            (uint8_t)output[RES_CONFIG + 9],
            (uint8_t)output[RES_CONFIG + 8]
            );

    output[RES_CONFIG + 16] = 15;
    output[RES_CONFIG + 17] = 17;
    output[RES_CONFIG + 18] = 0;
    output[RES_CONFIG + 19] = 0;
    output[RES_CONFIG + 20] = 5;
    output[RES_CONFIG + 21] = 0;
    output[RES_CONFIG + 22] = 0;
    output[RES_CONFIG + 23] = 0;

    /* Example: Set UUID ---------------------------------------------------------- */
    config_lookup_string(&cfg, "uuid", &str_val);
    memcpy(qrdata, str_val, 36);
    unsigned char uuid_bin[16];
    uuid_parse(str_val, uuid_bin);
    for(i = 0; i < 16; i++) {
      output[RES_UUID + i] = uuid_bin[i];
    }
    fprintf(out_txt, "UUID: %s\n", str_val);

    /* Example: Set PSK ----------------------------------------------------------- */
    config_lookup_string(&cfg, "psk", &str_val);
    memcpy(qrdata + 37, str_val, 16);
    for(i = 0; i < 16; i++) {
      output[RES_PSK + i] = str_val[i];
    }
    fprintf(out_txt, "PSK: %.*s\n", 16, str_val);
    memcpy(output + RES_ANSCHARS, anschars, LEN_ANSCHARS);

    /* Example: ECC base point and order for secp256r1 ---------------------------- */
    uint32_t *base_x = (uint32_t *)(output + RES_ECC_BASE_X);
    base_x[0] = 0xd898c296;
    base_x[1] = 0xf4a13945;
    base_x[2] = 0x2deb33a0;
    base_x[3] = 0x77037d81;
    base_x[4] = 0x63a440f2;
    base_x[5] = 0xf8bce6e5;
    base_x[6] = 0xe12c4247;
    base_x[7] = 0x6b17d1f2;

    uint32_t *base_y = (uint32_t *)(output + RES_ECC_BASE_Y);
    base_y[0] = 0x37bf51f5;
    base_y[1] = 0xcbb64068;
    base_y[2] = 0x6b315ece;
    base_y[3] = 0x2bce3357;
    base_y[4] = 0x7c0f9e16;
    base_y[5] = 0x8ee7eb4a;
    base_y[6] = 0xfe1a7f9b;
    base_y[7] = 0x4fe342e2;

    uint32_t *order = (uint32_t *)(output + RES_ECC_ORDER);
    order[0] = 0xFC632551;
    order[1] = 0xF3B9CAC2;
    order[2] = 0xA7179E84;
    order[3] = 0xBCE6FAAD;
    order[4] = 0xFFFFFFFF;
    order[5] = 0xFFFFFFFF;
    order[6] = 0x00000000;
    order[7] = 0xFFFFFFFF;

    /* Example: Set name ---------------------------------------------------------- */
    config_lookup_string(&cfg, "name", &str_val);
    snprintf(output + RES_NAME, LEN_NAME, "%s", str_val);
    fprintf(out_txt, "Name: %s\n", str_val);

    /* Example: Set model---------------------------------------------------------- */
    config_lookup_string(&cfg, "model", &str_val);
    snprintf(output + RES_MODEL, LEN_MODEL, "%s", str_val);
    fprintf(out_txt, "Model: %s\n", str_val);

    /* Example: Set time ---------------------------------------------------------- */
    time_t my_time = time(NULL);
    memcpy(output + RES_FLASHTIME, (void *)&my_time, LEN_FLASHTIME);
    struct tm *timeinfo = localtime(&my_time);
    fwrite(buf, 1, strftime((char *)buf, 64, "Created on %d.%m.%Y um %H:%M:%S", timeinfo), out_txt);

    /* Output result -------------------------------------------------------------- */
    for(i = 4; i < 0x1F000; i++) {
      fputc(output[i], out_bin);
    }

    /* Generate QR-Code ----------------------------------------------------------- */
    QRcode *code = QRcode_encodeString8bit(qrdata, 3, QR_ECLEVEL_L);
    writeImg(out_pbm, code->data, code->width);

    fclose(in_bin);
    fclose(out_bin);
    fclose(out_txt);
    fclose(out_pbm);

    fprintf(stdout, "DONE\n");
  }

  exit(EXIT_SUCCESS);
}
/* ---------------------------------------------------------------------------- */

FILE *
openFile(const char *name, const char *appendix, const char *mode)
{
  char filename[64];
  sprintf(filename, "%s%s", name, appendix);
  FILE *file = fopen(filename, mode);
  if(file == NULL) {
    perror("Wasn't able to open file.");
    exit(EXIT_FAILURE);
  }
  return file;
}
void
writeStandardConfig()
{
  unsigned int i;

  config_t cfg;
  config_init(&cfg);
  config_setting_t *setting;

  config_setting_t *root = config_root_setting(&cfg);

  setting = config_setting_add(root, "input", CONFIG_TYPE_STRING);
  config_setting_set_string(setting, "dff_econotag");

  setting = config_setting_add(root, "output", CONFIG_TYPE_STRING);
  config_setting_set_string(setting, "dff_e_econotag");

  uint8_t eui[8] = { 0x02, 0x00, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78 };
  config_setting_t *array = config_setting_add(root, "eui", CONFIG_TYPE_ARRAY);
  for(i = 0; i < 8; ++i) {
    setting = config_setting_add(array, NULL, CONFIG_TYPE_INT);
    config_setting_set_format(setting, CONFIG_FORMAT_HEX);
    config_setting_set_int(setting, eui[i]);
  }

  unsigned char uuid_bin[16];
  uuid_generate(uuid_bin);
  char uuid[37];
  uuid_unparse(uuid_bin, uuid);
  setting = config_setting_add(root, "uuid", CONFIG_TYPE_STRING);
  config_setting_set_string(setting, uuid);

  char psk[17];
  psk[16] = '\0';
  FILE *fd = fopen("/dev/urandom", "r");
  if(fd == NULL) {
    perror("Wasn't able to open /dev/urandom: ");
    return;
  }
  for(i = 0; i < 16; i++) {
    int c;
    while((c = fgetc(fd)) == EOF) ;
    psk[i] = anschars[c % 64];
  }
  if(fclose(fd) == -1) {
    perror("Wasn't able to close /dev/urandom: ");
  }
  setting = config_setting_add(root, "psk", CONFIG_TYPE_STRING);
  config_setting_set_string(setting, psk);

  setting = config_setting_add(root, "name", CONFIG_TYPE_STRING);
  config_setting_set_string(setting, "Blaster Standard Device");

  setting = config_setting_add(root, "model", CONFIG_TYPE_STRING);
  config_setting_set_string(setting, "Model 1234 for testing purposes only");

  config_write_file(&cfg, "config.cfg");
}
void
writeImg(FILE *file, unsigned char *data, int width)
{
  unsigned int buf[width];

  fprintf(file, "P4\n# %s\n%3u %3u\n", "QR-Code", width * 32, width * 32);

  int x, y;
  for(y = 0; y < width; y++) {
    for(x = 0; x < width; x++) {
      if(data[(y * width) + x] & 0x01) {
        buf[x] = 0xFFFFFFFF;
      } else {
        buf[x] = 0x00000000;
      }
    }
    for(x = 0; x < 32; x++) {
      fwrite(buf, 4, width, file);
    }
  }
}
