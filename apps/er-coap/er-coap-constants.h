/*
 * Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *      Collection of constants specified in the CoAP standard.
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 */

#ifndef ER_COAP_CONSTANTS_H_
#define ER_COAP_CONSTANTS_H_

#define COAP_DEFAULT_PORT                    5683

#define COAP_DEFAULT_MAX_AGE                 60
#define COAP_RESPONSE_TIMEOUT                3
#define COAP_RESPONSE_RANDOM_FACTOR          1.5
#define COAP_MAX_RETRANSMIT                  4

#define COAP_HEADER_LEN                      4  /* | version:0x03 type:0x0C tkl:0xF0 | code | mid:0x00FF | mid:0xFF00 | */
#define COAP_TOKEN_LEN                       8  /* The maximum number of bytes for the Token */
#define COAP_ETAG_LEN                        8  /* The maximum number of bytes for the ETag */

#define COAP_HEADER_VERSION_MASK             0xC0
#define COAP_HEADER_VERSION_POSITION         6
#define COAP_HEADER_TYPE_MASK                0x30
#define COAP_HEADER_TYPE_POSITION            4
#define COAP_HEADER_TOKEN_LEN_MASK           0x0F
#define COAP_HEADER_TOKEN_LEN_POSITION       0

#define COAP_HEADER_OPTION_DELTA_MASK        0xF0
#define COAP_HEADER_OPTION_SHORT_LENGTH_MASK 0x0F

/* CoAP message types */
typedef enum {
  COAP_TYPE_CON,                /* confirmables */
  COAP_TYPE_NON,                /* non-confirmables */
  COAP_TYPE_ACK,                /* acknowledgements */
  COAP_TYPE_RST                 /* reset */
} coap_message_type_t;

/* CoAP request method codes */
typedef enum {
  COAP_GET = 1,
  COAP_POST,
  COAP_PUT,
  COAP_DELETE
} coap_method_t;

/* CoAP response codes */
typedef enum {
  NO_ERROR = 0,

  CREATED_2_01 = 65,            /* CREATED */
  DELETED_2_02 = 66,            /* DELETED */
  VALID_2_03 = 67,              /* NOT_MODIFIED */
  CHANGED_2_04 = 68,            /* CHANGED */
  CONTENT_2_05 = 69,            /* OK */
  CONTINUE_2_31 = 95,           /* CONTINUE */

  BAD_REQUEST_4_00 = 128,       /* BAD_REQUEST */
  UNAUTHORIZED_4_01 = 129,      /* UNAUTHORIZED */
  BAD_OPTION_4_02 = 130,        /* BAD_OPTION */
  FORBIDDEN_4_03 = 131,         /* FORBIDDEN */
  NOT_FOUND_4_04 = 132,         /* NOT_FOUND */
  METHOD_NOT_ALLOWED_4_05 = 133,        /* METHOD_NOT_ALLOWED */
  NOT_ACCEPTABLE_4_06 = 134,    /* NOT_ACCEPTABLE */
  PRECONDITION_FAILED_4_12 = 140,       /* BAD_REQUEST */
  REQUEST_ENTITY_TOO_LARGE_4_13 = 141,  /* REQUEST_ENTITY_TOO_LARGE */
  UNSUPPORTED_MEDIA_TYPE_4_15 = 143,    /* UNSUPPORTED_MEDIA_TYPE */

  INTERNAL_SERVER_ERROR_5_00 = 160,     /* INTERNAL_SERVER_ERROR */
  NOT_IMPLEMENTED_5_01 = 161,   /* NOT_IMPLEMENTED */
  BAD_GATEWAY_5_02 = 162,       /* BAD_GATEWAY */
  SERVICE_UNAVAILABLE_5_03 = 163,       /* SERVICE_UNAVAILABLE */
  GATEWAY_TIMEOUT_5_04 = 164,   /* GATEWAY_TIMEOUT */
  PROXYING_NOT_SUPPORTED_5_05 = 165,    /* PROXYING_NOT_SUPPORTED */

  /* Erbium errors */
  MEMORY_ALLOCATION_ERROR = 192,
  PACKET_SERIALIZATION_ERROR,

  /* Erbium hooks */
  MANUAL_RESPONSE,
  PING_RESPONSE
} coap_status_t;

/* CoAP header option numbers */
typedef enum {
  COAP_OPTION_IF_MATCH = 1,     /* 0-8 B */
  COAP_OPTION_URI_HOST = 3,     /* 1-255 B */
  COAP_OPTION_ETAG = 4,         /* 1-8 B */
  COAP_OPTION_IF_NONE_MATCH = 5,        /* 0 B */
  COAP_OPTION_OBSERVE = 6,      /* 0-3 B */
  COAP_OPTION_URI_PORT = 7,     /* 0-2 B */
  COAP_OPTION_LOCATION_PATH = 8,        /* 0-255 B */
  COAP_OPTION_URI_PATH = 11,    /* 0-255 B */
  COAP_OPTION_CONTENT_FORMAT = 12,      /* 0-2 B */
  COAP_OPTION_MAX_AGE = 14,     /* 0-4 B */
  COAP_OPTION_URI_QUERY = 15,   /* 0-255 B */
  COAP_OPTION_ACCEPT = 17,      /* 0-2 B */
  COAP_OPTION_LOCATION_QUERY = 20,      /* 0-255 B */
  COAP_OPTION_BLOCK2 = 23,      /* 1-3 B */
  COAP_OPTION_BLOCK1 = 27,      /* 1-3 B */
  COAP_OPTION_SIZE2 = 28,       /* 0-4 B */
  COAP_OPTION_PROXY_URI = 35,   /* 1-1034 B */
  COAP_OPTION_PROXY_SCHEME = 39,        /* 1-255 B */
  COAP_OPTION_SIZE1 = 60,       /* 0-4 B */
} coap_option_t;

/* CoAP Content-Types */
typedef enum { // https://www.iana.org/assignments/core-parameters/core-parameters.xhtml#content-formats
  /* 0-255  Expert Review */
  TEXT_PLAIN                   = 0    ,  //  text/plain; charset=utf-8                    /* Ref: [RFC2046][RFC3676][RFC5147] */
  TEXT_XML                     = 1    ,  //                                               /* Ref: ??? */
  TEXT_CSV                     = 2    ,  //                                               /* Ref: ??? */
  TEXT_HTML                    = 3    ,  //                                               /* Ref: ??? */
  APPLICATION_COSE_ENCRYPT0    = 16   ,  //  application/cose; cose-type="cose-encrypt0"  /* Ref: [RFC8152] */
  APPLICATION_COSE_MAC0        = 17   ,  //  application/cose; cose-type="cose-mac0"      /* Ref: [RFC8152] */
  APPLICATION_COSE_SIGN1       = 18   ,  //  application/cose; cose-type="cose-sign1"     /* Ref: [RFC8152] */
  IMAGE_GIF                    = 21   ,  //                                               /* Ref: ??? */
  IMAGE_JPEG                   = 22   ,  //                                               /* Ref: ??? */
  IMAGE_PNG                    = 23   ,  //                                               /* Ref: ??? */
  IMAGE_TIFF                   = 24   ,  //                                               /* Ref: ??? */
  AUDIO_RAW                    = 25   ,  //                                               /* Ref: ??? */
  VIDEO_RAW                    = 26   ,  //                                               /* Ref: ??? */
  APPLICATION_LINK_FORMAT      = 40   ,  //  application/link-format                      /* Ref: [RFC6690] */
  APPLICATION_XML              = 41   ,  //  application/xml                              /* Ref: [RFC3023] */
  APPLICATION_OCTET_STREAM     = 42   ,  //  application/octet-stream                     /* Ref: [RFC2045][RFC2046] */
  APPLICATION_RDF_XML          = 43   ,  //                                               /* Ref: ??? */
  APPLICATION_SOAP_XML         = 44   ,  //                                               /* Ref: ??? */
  APPLICATION_ATOM_XML         = 45   ,  //                                               /* Ref: ??? */
  APPLICATION_XMPP_XML         = 46   ,  //                                               /* Ref: ??? */
  APPLICATION_EXI              = 47   ,  //  application/exi                              /* Ref: ["Efficient XML Interchange (EXI) Format 1.0 (Second Edition)" ,February 2014] */
  APPLICATION_FASTINFOSET      = 48   ,  //                                               /* Ref: ??? */
  APPLICATION_SOAP_FASTINFOSET = 49   ,  //                                               /* Ref: ??? */
  APPLICATION_JSON             = 50   ,  //  application/json                             /* Ref: [RFC4627] */
  APPLICATION_X_OBIX_BINARY    = 51   ,  //  note: Conflicts with application/json-patch+json /* Ref: ??? */
  APPLICATION_JSON_PATCH_JSON  = 51   ,  //  application/json-patch+json                  /* Ref: [RFC6902] */
  APPLICATION_MERGE_PATCH_JSON = 52   ,  //  application/merge-patch+json                 /* Ref: [RFC7396] */
  APPLICATION_CBOR             = 60   ,  //  application/cbor                             /* Ref: [RFC7049] */
  APPLICATION_CWT              = 61   ,  //  application/cwt                              /* Ref: [RFC8392] */
  APPLICATION_COSE_ENCRYPT     = 96   ,  //  application/cose; cose-type="cose-encrypt"   /* Ref: [RFC8152] */
  APPLICATION_COSE_MAC         = 97   ,  //  application/cose; cose-type="cose-mac"       /* Ref: [RFC8152] */
  APPLICATION_COSE_SIGN        = 98   ,  //  application/cose; cose-type="cose-sign"      /* Ref: [RFC8152] */
  APPLICATION_COSE_KEY         = 101  ,  //  application/cose-key                         /* Ref: [RFC8152] */
  APPLICATION_COSE_KEY_SET     = 102  ,  //  application/cose-key-set                     /* Ref: [RFC8152] */
  APPLICATION_COAP_GROUP_JSON  = 256  ,  //  application/coap-group+json                  /* Ref: [RFC7390] */
  /* 256-9999  IETF Review or IESG Approval */
  APPLICATION_OMA_TLV_OLD      = 1542 ,  //  Keep old value for backward-compatibility    /* Ref: [OMA-TS-LightweightM2M-V1_0] */
  APPLICATION_OMA_JSON_OLD     = 1543 ,  //  Keep old value for backward-compatibility    /* Ref: [OMA-TS-LightweightM2M-V1_0] */
  /* 10000-64999  First Come First Served */
  APPLICATION_VND_OCF_CBOR     = 10000,  //  application/vnd.ocf+cbor                     /* Ref: [Michael_Koster] */
  APPLICATION_OMA_TLV          = 11542,  //  application/vnd.oma.lwm2m+tlv                /* Ref: [OMA-TS-LightweightM2M-V1_0] */
  APPLICATION_OMA_JSON         = 11543,   //  application/vnd.oma.lwm2m+json               /* Ref: [OMA-TS-LightweightM2M-V1_0] */
  /* 65000-65535  Experimental use (no operational use) */
  CONTENT_MAX_VALUE = 0xFFFF
} coap_content_type_t;
#endif /* ER_COAP_CONSTANTS_H_ */
