#include "contiki.h"
#include "common.h"
#include "lib/random.h"
#include "sys/node-id.h"
#include "dev/radio.h"
#include PLATFORM_RADIO_HEADER

#ifdef CONTIKI_TARGET_SKY
#include "dev/sht11/sht11-sensor.h"
#elif CONTIKI_TARGET_Z1
#include "dev/tmp102.h"
#elif CONTIKI_TARGET_AVR_RSS2
#include "ds18b20.h" // for RS2
#endif

extern uint8_t currentState;

PROCESS(samplingProcess, "Sampling process");

PROCESS_THREAD(samplingProcess, ev, data)
{
    //  static struct etimer et;
    //  static uint16_t r;
    
    PROCESS_BEGIN();
    
#if CONTIKI_TARGET_SKY
    SENSORS_ACTIVATE(sht11_sensor);
#elif CONTIKI_TARGET_Z1
    tmp102_init();
#else
#endif
    
    //  etimer_set(&et, CLOCK_SECOND);
    
    while (1) {
        //    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
        PROCESS_WAIT_EVENT();
        if (ev == PROCESS_EVENT_POLL) {
            
            // r = (random_rand() >> 2) % (4 * CLOCK_SECOND);
            
            // etimer_set(&et, r);
            // PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
            
#if CONTIKI_TARGET_SKY
            uint16_t temp = sht11_sensor.value(SHT11_SENSOR_TEMP);
            uint16_t humidity = sht11_sensor.value(SHT11_SENSOR_HUMIDITY);
            
            printf("T=%u H=%u\n", temp, humidity);
#elif CONTIKI_TARGET_Z1
            uint16_t temp = tmp102_read_temp_raw();
            printf("T=%u A=%u\n", temp, node_id);
#else
            int16_t temp;
            if (ds18b20_get_temp(&temp)) {
                printf("T=%u A=%u\n", temp, node_id);
            } else {
                puts("reading temperature failed");
            }
#endif
            
        }
        
        //    etimer_set(&et, 5*CLOCK_SECOND - r);
    }
    
    PROCESS_END();
}

extern uint8_t  ds18b20_get_temp(int16_t *temperature);