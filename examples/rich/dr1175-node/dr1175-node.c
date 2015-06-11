/***************************************************************************
 *
 *
 * This software is owned by NXP B.V. and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on NXP products [NXP Microcontrollers such as JN5148, JN5142, JN5139].
 * You, and any third parties must reproduce the copyright and warranty notice
 * and any other legend of ownership on each copy or partial copy of the
 * software.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.

 * Copyright NXP B.V. 2015. All rights reserved
 *
***************************************************************************/
#include "contiki.h"
#include "net/ip/uip.h"
#include "rich.h"
#include "rest-engine.h"
#include "light-sensor.h"
#include "ht-sensor.h"
#include <stdio.h>
#include <LightingBoard.h>
#include <pca9634.h>

static void event_sensors_dr1175_handler(void);
static void get_sensors_dr1175_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void get_light_sensor_value_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void get_light_sensor_unit_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void get_temperature_value_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void get_temperature_unit_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void get_humidity_value_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void get_humidity_unit_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void put_post_white_led_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void put_post_rgb_led_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void put_post_scale_rgb_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

static char content[REST_MAX_CHUNK_SIZE];
static int content_len = 0;

#define CONTENT_PRINTF(...) { if(content_len < sizeof(content)) content_len += snprintf(content+content_len, sizeof(content)-content_len, __VA_ARGS__); }

#define CLIP(value, level) if (value > level) { \
                              value = level;    \
                           }                
/*---------------------------------------------------------------------------*/
PROCESS(start_app, "START_APP");
AUTOSTART_PROCESSES(&start_app, &sensors_process);

/*---------------------------------------------------------------------------*/

/*********** RICH sensor/ resource *************************************************/

/*******************************************************************/
/* Observable resource and event handler to obtain all sensor data */ 
/*******************************************************************/
EVENT_RESOURCE(resource_sensors_dr1175,               /* name */
               "obs;title=\"All_DR1175_sensors\"",    /* attributes */
               get_sensors_dr1175_handler,            /* GET handler */
               NULL,                                  /* POST handler */
               NULL,                                  /* PUT handler */
               NULL,                                  /* DELETE handler */
               event_sensors_dr1175_handler);         /* event handler */
static void
get_sensors_dr1175_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.APPLICATION_JSON) {
    content_len = 0;
    CONTENT_PRINTF("{\"DR1175\":[");
    CONTENT_PRINTF("{\"Humidity\":\"%d\"},", ht_sensor.value(HT_SENSOR_HUM));
    CONTENT_PRINTF("{\"Light\":\"%d\"},", light_sensor.value(0));
    CONTENT_PRINTF("{\"Temp\":\"%d\"}", ht_sensor.value(HT_SENSOR_TEMP));
    CONTENT_PRINTF("]}");
    REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
    REST.set_response_payload(response, (uint8_t *)content, content_len);
  }
} 

static void
event_sensors_dr1175_handler()
{
  /* Registered observers are notified and will trigger the GET handler to create the response. */
  REST.notify_subscribers(&resource_sensors_dr1175);
}

/*****************************************************/
/* Resource and handler to obtain light sensor value */ 
/*****************************************************/
RESOURCE(resource_light_sensor_value,
         "title=\"light sensor value\"",
         get_light_sensor_value_handler,
         NULL,
         NULL,
         NULL);
static void
get_light_sensor_value_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    content_len = 0;
    CONTENT_PRINTF("%d", light_sensor.value(0));
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    REST.set_response_payload(response, (uint8_t *)content, content_len);
  }
}

/***************************************************/
/* Resource and handler to obtain light unit value */ 
/***************************************************/
RESOURCE(resource_light_sensor_unit, 
         "title=\"light sensor unit\"",
         get_light_sensor_unit_handler,
         NULL,
         NULL,
         NULL);
static void
get_light_sensor_unit_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    content_len = 0;
    CONTENT_PRINTF("Lux");
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    REST.set_response_payload(response, (uint8_t *)content, content_len);
  }
}

/***********************************************************/
/* Resource and handler to obtain temperature sensor value */ 
/***********************************************************/
RESOURCE(resource_temperature_value,
         "title=\"temperature value\"",
         get_temperature_value_handler,
         NULL,
         NULL,
         NULL);
static void
get_temperature_value_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    content_len = 0;
    CONTENT_PRINTF("%d", ht_sensor.value(HT_SENSOR_TEMP));
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    REST.set_response_payload(response, (uint8_t *)content, content_len);
  }
}

/*********************************************************/
/* Resource and handler to obtain temperature unit value */ 
/*********************************************************/
RESOURCE(resource_temperature_unit, 
         "title=\"temperature unit\"",
         get_temperature_unit_handler,
         NULL,
         NULL,
         NULL);
static void
get_temperature_unit_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    content_len = 0;
    CONTENT_PRINTF("degrees C");
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    REST.set_response_payload(response, (uint8_t *)content, content_len);
  }
}

/********************************************************/
/* Resource and handler to obtain humidity sensor value */ 
/********************************************************/
RESOURCE(resource_humidity_value,
         "title=\"humidity value\"",
         get_humidity_value_handler,
         NULL,
         NULL,
         NULL);
static void
get_humidity_value_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    content_len = 0;
    CONTENT_PRINTF("%d", ht_sensor.value(HT_SENSOR_HUM));
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    REST.set_response_payload(response, (uint8_t *)content, content_len);
  }
}

/******************************************************/
/* Resource and handler to obtain humidity unit value */ 
/******************************************************/
RESOURCE(resource_humidity_unit, 
         "title=\"humidity unit\"",
         get_humidity_unit_handler,
         NULL,
         NULL,
         NULL);
static void
get_humidity_unit_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    content_len = 0;
    CONTENT_PRINTF("relative %%");
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    REST.set_response_payload(response, (uint8_t *)content, content_len);
  }
}

/***************************************************/
/* Resource and handler to control White LED level */ 
/***************************************************/
RESOURCE(resource_white_led, 
         "title=\"WhiteLED <[0..255]>\"",
         NULL,
         put_post_white_led_handler,
         put_post_white_led_handler,
         NULL);
static void
put_post_white_led_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  const uint8_t *request_content = NULL;
  int request_content_len;
  int level;
 
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    request_content_len = REST.get_request_payload(request, &request_content);
    level = atoi((const char *)request_content);
    CLIP(level, 255)
    if (level > 255)
    {
      level = 255;
    }
    bWhite_LED_SetLevel(level);
    bWhite_LED_On();
  }
}

/*************************************************/
/* Resource and handler to control RGB LED level */ 
/*************************************************/
RESOURCE(resource_rgb_led, 
         "title=\"RGB LED <[0..255] [0..255] [0..255]>\"",
         NULL,
         put_post_rgb_led_handler,
         put_post_rgb_led_handler,
         NULL);
static void
put_post_rgb_led_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  const uint8_t *request_content = NULL;
  int request_content_len;
  char * pch;
  int RGB[] = {0,0,0};
  int index = 0;
 
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    request_content_len = REST.get_request_payload(request, &request_content);
    pch = strtok((char *)request_content, " ");
    while((pch != NULL) && (index != sizeof(RGB)/sizeof(int)))
    {
      /* Convert token to int */
      RGB[index] = atoi(pch);
      CLIP(RGB[index], 255) 
      index++;
      /* Get next token */
      pch = strtok(NULL, " ");
    }
    bRGB_LED_SetLevel(RGB[0],RGB[1],RGB[2]);
    bRGB_LED_On();
  }
}

/***************************************************/
/* Resource and handler to control RGB LED scaling */ 
/***************************************************/
RESOURCE(resource_scale_rgb, 
         "title=\"Scale RGB <[0..255]>\"",
         NULL,
         put_post_scale_rgb_handler,
         put_post_scale_rgb_handler,
         NULL);
static void
put_post_scale_rgb_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  const uint8_t *request_content = NULL;
  int request_content_len;
  int level;
 
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    request_content_len = REST.get_request_payload(request, &request_content);
    level = atoi((const char *)request_content);
    CLIP(level,255)
    bRGB_LED_SetGroupLevel(level);
    bRGB_LED_On();
  }
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(start_app, ev, data)
{
  PROCESS_BEGIN();

  static int is_coordinator = 0;
  //is_coordinator = node_id == 1;
  
  /* White LED initialisation */
  bWhite_LED_Enable();
  bWhite_LED_SetLevel(0);
  bWhite_LED_On();

  /* Coloured LED initialisation */ 
  bRGB_LED_Enable();
  bRGB_LED_SetGroupLevel(255);
  bRGB_LED_SetLevel(0,0,0);
  bRGB_LED_On();

  /* Make sensor active for measuring */
  SENSORS_ACTIVATE(light_sensor);
  SENSORS_ACTIVATE(ht_sensor);
  
  /* Start RICH stack */
  if(is_coordinator) {
    uip_ipaddr_t prefix;
    uip_ip6addr(&prefix, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
    rich_init(&prefix);
  } else {
    rich_init(NULL);
  }
  printf("Starting RPL node\n");

  rest_init_engine();
  rest_activate_resource(&resource_light_sensor_value,  "DR1175/LightSensor/Value");
  rest_activate_resource(&resource_light_sensor_unit,   "DR1175/LightSensor/Unit");
  rest_activate_resource(&resource_temperature_unit,    "DR1175/Temperature/Unit");
  rest_activate_resource(&resource_temperature_value,   "DR1175/Temperature/Value");
  rest_activate_resource(&resource_humidity_unit,       "DR1175/Humidity/Unit");
  rest_activate_resource(&resource_humidity_value,      "DR1175/Humidity/Value");
  rest_activate_resource(&resource_white_led,           "DR1175/WhiteLED");
  rest_activate_resource(&resource_rgb_led,             "DR1175/ColorLED/RGBValue");
  rest_activate_resource(&resource_scale_rgb,           "DR1175/ColorLED/ScaleRGB");
  rest_activate_resource(&resource_sensors_dr1175,      "DR1175/AllSensors");

  /* If sensor process generates an event, call event_handler of resource.
     This will make this resource observable by the client */ 
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL( (ev == sensors_event) && 
                              ((data == &light_sensor)|| (data == &ht_sensor)) );
    event_sensors_dr1175_handler();
  }

  PROCESS_END();
}
