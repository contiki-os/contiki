# Zolertia MQTT node

This application is only available for `Zoul` boards like the `RE-Mote`

## Features

The MQTT client features the required functions to connect, subscribe and publish data to a MQTT broker, easily allowing cloud-specific applications and sensors/actuators to be implemented on top, fastening the implementation of real-world applications.

The MQTT application uses [Cetic 6lbr](http://cetic.github.io/6lbr/) as the Border Router, handling the configuration and traffic of the IPv4/IPv6 annd 6LoWPAN networks.

The MQTT client features:

* Supports IPv4 and IPv6 cloud platforms
* Compatible with [Inmarsat BGAN M2M satellite modems](http://www.inmarsat.com/service/bgan-m2m)
* DHCP enabled as default
* Support out-of-the-box for the following cloud platforms: [IBM Bluemix](https://quickstart.internetofthings.ibmcloud.com/#/), [The Things.io](https://thethings.io/), [Relayr.io](https://relayr.io/)
* Provisioning over browser (configuring user and password over Chrome, Firefox, ...)
* Save provisioning information to flash memory
* Implements structure to validate and publish sensor data to a Broker, allowing seamless configuration of sensor parameters (minimum and maximum allowed values, resolution)
* Implements structure to check for values over/below given thresholds to publish alarms to a Broker
* Implements structure to receive commands handlers, to implement custom commands at the application level
* Basic set of status information included in publications: Device ID, RSSI (strenght signal indicator), parent address
* Basic set of commands included as default: toggle LED, reboot, enable and disable sensors, change periodic publish interval
* Integrates several sensors and actuators, independently from the chosen Cloud platform

### What do I need?

To build the application you will need:

* At least one **[[Zolertia RE-Mote | RE-Mote]]**
* A **Border Router** (same as Edge Router), check this page for instruction on [[how to setup a Border Router using an ENC28J60 ethernet module | 6lbr#zolertia-ethernet-iot-gateway]]

In the next section is shown [[how to create your own application | MQTT-client#build-your-own]], and [[how to use the ready-to-deploy examples available|MQTT-client#download-and-program-ready-to-use-examples]].

### How does it works?

![](http://i.imgur.com/wMGQ2Hp.jpg)

The MQTT client just requires to things: a MQTT broker or cloud platform to publish and subscribe to, and sensors/actuators, that's it.

The implementation ensures the Cloud platform implementation is independent from the MQTT client, having only to use the provided API and implement the proper _callback_ handlers.  The sensor and actuator implementations at the same time are not dependant on the Cloud platforms, but implement a standard structure to ensure many different sensors and actuators can be implemented together into business logic applications.

This modelling approach ensures developers and makers build their own applications in minutes, just either by using an existing Cloud platform implementation and just implementing their own sensors, or even implementing a new platform on their own!

#### Build your own!

The magic happens in the `Makefile`:

````
# Platform options: relayr, thingsio, bluemix
MQTT_PLATFORM ?= bluemix

# Sensor options: fridge, agriculture, remote, irrigation
MQTT_SENSORS  ?= remote
````

Or over the command line:

`make mqtt-client MQTT_PLATFORM=bluemix MQTT_SENSORS_remote`

Depending on the platform selected this part of the tutorial will continue [[on the next sections | MQTT-client#supported-iot-cloud-platforms]].

And of course [the firmware to compile and program](https://github.com/alignan/contiki/tree/zolertia-workshop) the devices.

````
git clone https://github.com/alignan/contiki
cd contiki
git checkout zolertia-workshop
cd examples/zolertia/zoul/mqtt-client
````

#### Download and program ready to use examples

There are already **binaries available** to download and save you the trouble of compiling:

The Border Router:

* [6lbr border router for the eth-gw](https://github.com/alignan/contiki/raw/zolertia-workshop/examples/zolertia/zoul/mqtt-client/Binaries/router/cetic_6lbr_router_eth_gw.bin).
* [6lbr border router for the RE-Mote and ENC28J60](https://github.com/alignan/contiki/raw/zolertia-workshop/examples/zolertia/zoul/mqtt-client/Binaries/router/cetic_6lbr_router_remote.bin).

And [compiled examples for the Use Cases](https://github.com/alignan/contiki/tree/zolertia-workshop/examples/zolertia/zoul/mqtt-client/Binaries/nodes).  The use cases are properly detailed in the [[Use Cases implementation|MQTT-client#example-use-cases-implemented]] section, with a detailed step by step guide for the supported platforms.

For instructions on how to compile and flash a binary please see the guides below:

* [[Toolchain and tools | Toolchain-and-tools]]
* [[Flash the platforms | Program-the-Zolertia-platforms]]

Mind the Border Router should be flashed starting the address `0x00200000` and the MQTT nodes start at the `0x00202000` address.  If flashing using the BSL then it will look like:

`python cc2538-bsl -e -v -w -a 0x00200000 cetic_6lbr_router_eth_gw.bin`

Or:

`python cc2538-bsl -e -v -w -a 0x00202000 mqtt-client-bluemix-remote.bin`


#### The MQTT client

The client is broken down into two components:

* The MQTT client in `mqtt-client.c`, interfacing with the Cloud platforms
* The MQTT sensors in `mqtt-sensors.c`, providing sensors and actuators a basic set of functionalities, like checking for valid values, trigger alarms, registering sensors, etc

The MQTT client holds the MQTT user configuration in the following _extern_ structure available to the platforms:

````
/**
 * \brief Data structure declaration for the MQTT client configuration, saved
 * in flash memory
 */
typedef struct mqtt_client_config {
  uint16_t magic_word;
  char auth_user[DEFAULT_AUTH_USER_LEN];
  char auth_token[DEFAULT_AUTH_TOKEN_LEN];
  clock_time_t pub_interval;
  uint16_t pub_interval_check;
  char client_id[DEFAULT_IP_ADDR_STR_LEN];
  uint16_t crc;
} mqtt_client_config_t;

extern mqtt_client_config_t conf; 
````

Whenever the `auth_user` and `auth_token` values are updated, the changes are stored in flash memory along with the current `pub_interval`, `pub_interval_check`, and the configured `client_id` (created by each platform). The `magic_word` and `crc` are variables used to validate the content retrieved from flash.

The following functions are available for the Cloud platforms:

````
/* MQTT client available functions */
void subscribe(char * topic);
void publish(uint8_t *app_buffer, char *pub_topic, uint16_t len);
int mqtt_write_config_to_flash(char *name, uint8_t *buf, uint16_t len);
int mqtt_read_config_from_flash(char *name, uint8_t *buf, uint16_t len);
/*---------------------------------------------------------------------------*/
/* MQTT client process events */
extern process_event_t mqtt_client_event_connected;
extern process_event_t mqtt_client_event_disconnected;
/*---------------------------------------------------------------------------*/
/* Macro to register the callback handler when a PUBLISH event is received */
#define MQTT_PUB_REGISTER_HANDLER(ptr) pub_handler = ptr;
extern void (*pub_handler)(const char *topic, uint16_t topic_len,
                           const uint8_t *chunk, uint16_t chunk_len);
````

Basically the platform follow the next approach:

1. Wait for the MQTT client to initialize the platform process once the credentials (if required) and information from flash has been checked, fill in the `conf.client_id` field as it will be used for the connection
2. Wait for the MQTT client to send a `mqtt_client_event_connected`, at this point the device is connected to the Broker
3. `Subscribe` to topics if required
4. Start the sensor process

The Cloud platforms needs to register a `pub_handler` using the `MQTT_PUB_REGISTER_HANDLER`.  Whenever a new publication is received, the MQTT client will invoke this function for the Cloud platform to handle it.

The MQTT client knows how to "talk" to platforms as it uses the following macro to expand:

````
#define SENSORS_NAME_EXPAND(x, y) x##y
#define SENSORS_NAME(x, y) SENSORS_NAME_EXPAND(x, y)
````

Thus `PROCESS_NAME(PLATFORM_NAME(MQTT_PLATFORM,_process));` is expanded at compilation to `PROCESS_NAME(relayr_process)`, if we are using the `relayr` platform.

The `Makefile` uses the following:

````
# Platform options: relayr, thingsio, bluemix
MQTT_PLATFORM ?= bluemix

# Sensor options: fridge, agriculture, remote, irrigation
MQTT_SENSORS  ?= remote
````

The MQTT sensors provides the sensors the following schema:

````
typedef struct sensor_val {
  int16_t value;                            /* Sensor value */
  int16_t over_threshold;                   /* Threshold (over) */
  int16_t below_threshold;                  /* Threshold (below) */
  int16_t min;                              /* Minimum allowed value */
  int16_t max;                              /* Maximum allowed value */
  uint16_t pres;                             /* Number of precision digits */
  char sensor_name[SENSOR_NAME_STRING];     /* Sensor name (string) */
  char alarm_name[SENSOR_NAME_STRING];      /* Alarm name (string) */
  char sensor_config[SENSOR_NAME_STRING];   /* Configuration name (string) */
} sensor_val_t;
````

This characterization allows to validate if a given sensor reading is invalid, or if the MQTT sensors component should trigger an alarm to the Cloud platform process if values over or below the threshold limits are reached.
The `sensor_name` string is a readable name for the sensor used when publishing sensor data to the Broker.  The `alarm_name` consequently is also used when publishing data to the Broker in case the application prefers to use a different variable name and keep alarms separately.  The `sensor_config` string is used in case the application requires to configure a given sensor remotely, exposing this to the Cloud platform implementation when checking for received commands.

The sensors are stored into an array inside the `sensor_values_t`.  The number of sensors is defined by the sensor component at the `Makefile` using the `DEFAULT_SENSORS_NUM` value.

````
typedef struct sensors_values {
  uint8_t num;                              /* Number of sensors */
  sensor_val_t sensor[DEFAULT_SENSORS_NUM]; /* Array of sensor_val_t */
} sensor_values_t;
````

Similar as done for the sensors, the MQTT sensors component also provides the following structure for actuators (switches, electro-valves, LEDs, etc):

````
typedef struct command_val {
  int (* cmd) (int arg);
  char command_name[COMMAND_NAME_STRING];     /* Command name (string) */
} command_val_t;

typedef struct command_values {
  uint8_t num;                                /* Number of commands */
  command_val_t command[DEFAULT_COMMANDS_NUM]; /* Array of command_val_t */
} command_values_t;
````

Basically the sensor/actuator component just register a command handler, and whenever the Cloud platform receives a command matching the `command_name` string it will invoke this command.  Commands are stored into an array inside the `command_values_t` structure.  The number of commands available is defined in the `Makefile` by each actuator process, using the `DEFAULT_COMMANDS_NUM` definition.

Moreover the MQTT sensors module also provides the following helper functions:

````
/* Auxiliary function to fill the sensor_values_t structure */
int mqtt_sensor_register(sensor_values_t *reg, uint8_t i, uint16_t val,
                         char *name, char *alarm, char *config, int16_t min,
                         int16_t max, int16_t thres, int16_t thresl,
                         uint16_t pres);

/* Auxiliary function to check the sensor values and send alarms or publish
 * periodic sensor events */
void mqtt_sensor_check(sensor_values_t *reg, process_event_t alarm,
                       process_event_t data);
````

#### The Cloud platforms

Each platform should define the following:

````
#define DEFAULT_CONF_AUTH_TOKEN       ""
#define DEFAULT_CONF_AUTH_USER        ""
#define MQTT_DEMO_CONF_BROKER_IP_ADDR "::ffff:36ab:7f82"
````

The `DEFAULT_CONF_AUTH_TOKEN` and `DEFAULT_CONF_AUTH_USER` strings can be empty, only used if the user wants to hard-code this information (taking precedence over any credentials configured later over the configuration webservice).  The `MQTT_DEMO_CONF_BROKER_IP_ADDR` can be an IPv6 address or a IPv4 address in the format of NAT64 (preceding by the `::ffff:` prefix).

The MQTT client upon initializing checks for the following:

````
#define DEFAULT_CONF_AUTH_IS_REQUIRED      1
#define DEFAULT_CONF_AUTH_USER_IS_REQUIRED 1
````

If the `auth_user` and `auth_token` are required, then the `DEFAULT_CONF_AUTH_IS_REQUIRED` should be `1`, else it will not be checked.  If no values are hardcoded and no information is found in flash memory, then it will not continue until it is configured over the configuration webservice.

The `DEFAULT_CONF_AUTH_USER_IS_REQUIRED` works in the same way as described above, but only checks for `auth_user`.

A minimal platform implementation is shown below:

````
/*---------------------------------------------------------------------------*/
#define SENSORS_NAME_EXPAND(x, y) x##y
#define SENSORS_NAME(x, y) SENSORS_NAME_EXPAND(x, y)
/*---------------------------------------------------------------------------*/
static void
publish_alarm(sensor_val_t *sensor)
{
  /* Create a string with the data into the expected format (i.e JSON) */
  if(etimer_expired(&alarm_expired)) {
    publish((uint8_t *)app_buffer, data_topic, strlen(app_buffer));
    /* Schedule the timer to prevent flooding the broker with the same event */
    etimer_set(&alarm_expired, (CLOCK_SECOND * DEFAULT_ALARM_TIME));
  }
}
/*---------------------------------------------------------------------------*/
static void
publish_event(sensor_values_t *msg)
{
  /* Create a string with the data into the expected format (i.e JSON) */
  publish((uint8_t *)app_buffer, data_topic, strlen(app_buffer));
}
/*---------------------------------------------------------------------------*/
/* This function handler receives publications to which we are subscribed */
static void
relayr_pub_handler(const char *topic, uint16_t topic_len, const uint8_t *chunk,
            uint16_t chunk_len)
{
  /* Parse and check for received commands */
}
/*---------------------------------------------------------------------------*/
static void
init_platform(void)
{
  /* Register the publish callback handler */
  MQTT_PUB_REGISTER_HANDLER(relayr_pub_handler);

  /* Create client id */
  /* Create topics */
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(relayr_process, ev, data)
{
  PROCESS_BEGIN();

  /* Initialize platform-specific */
  init_platform();

  printf("\nRelayr process started\n");
  printf("  Client ID:    %s\n", conf.client_id);
  printf("  Data topic:   %s\n", data_topic);
  printf("  Cmd topic:    %s\n\n", cmd_topic);

  while(1) {
    PROCESS_YIELD();
    if(ev == mqtt_client_event_connected) {

      /* Subscribe to topics (MQTT driver only supports 1 topic at the moment */
      subscribe(cmd_topic);

      /* Enable the sensor */
      process_start(&SENSORS_NAME(MQTT_SENSORS, _sensors_process), NULL);
    }

    if(ev == mqtt_client_event_disconnected) {
      /* We are not connected, disable the sensors */
      process_stop(&SENSORS_NAME(MQTT_SENSORS, _sensors_process), NULL);
    }

    /* Check for periodic publish events */
    if(ev == SENSORS_NAME(MQTT_SENSORS,_sensors_data_event)) {
      seq_nr_value++;

      /* The `pub_interval_check` is an external struct defined in mqtt-client */
      if(!(seq_nr_value % conf.pub_interval_check)) {
        sensor_values_t *msgPtr = (sensor_values_t *) data;
        publish_event(msgPtr);
      }
    }

    /* Check for alarms */
    if(ev == SENSORS_NAME(MQTT_SENSORS,_sensors_alarm_event)) {
      sensor_val_t *sensorPtr = (sensor_val_t *) data;
      publish_alarm(sensorPtr);
    }
  }
  PROCESS_END();
}
````

Notice how we are also using a Macro to expand the sensor/actuator process name:

````
#define SENSORS_NAME_EXPAND(x, y) x##y
#define SENSORS_NAME(x, y) SENSORS_NAME_EXPAND(x, y)
````

As we saw before the `MQTT_SENSORS` is defined in the `Makefile` and reflects the name of the enabled use case module.

#### The sensors and actuators

The modules containing the sensors and actuators need to implement the following:

````
extern process_event_t irrigation_sensors_data_event;
extern process_event_t irrigation_sensors_alarm_event;
/*---------------------------------------------------------------------------*/
extern sensor_values_t irrigation_sensors;
/*---------------------------------------------------------------------------*/
extern command_values_t irrigation_commands;
````

In this example the `irrigation.c` module has created a `sensor_values_t` and `command_values_t` structures, in which it will define the sensors and actuators supported.  Moreover it will create two events named `irrigation_sensors_data_event` and `irrigation_sensors_alarm_event`, used to inform the Cloud platform when data is available, or whenever an alarm occurs.  Notice the `SENSORS_NAME` macro explained in the section before will use the name of the module to create during compilation names, for example `SENSORS_NAME(MQTT_SENSORS,_sensors_data_event)` in the Cloud platform module expands into the `irrigation_sensors_data_event` we just implemented in our module.

A basic sensor and actuator module is shown below:

````
/*---------------------------------------------------------------------------*/
static int
activate_electrovalve(int arg)
{
  /* Activate electrovalve */
}
/*---------------------------------------------------------------------------*/
static void
poll_sensors(void)
{
  /*  Save value into structure */
  irrigation_sensors.sensor[IRRIGATION_SENSOR_SOIL].value = soil_hum.value(1);
  /* Use this function to check for proper values and alarms occurrences */
  mqtt_sensor_check(&irrigation_sensors, irrigation_sensors_alarm_event,
                    irrigation_sensors_data_event);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(irrigation_sensors_process, ev, data)
{
  /* This is where our process start */
  PROCESS_BEGIN();

  /* Load sensor defaults */
  irrigation_sensors.num = 0;
  mqtt_sensor_register(&irrigation_sensors, IRRIGATION_SENSOR_SOIL,
                       DEFAULT_SOIL_MOIST_MAX, DEFAULT_PUBLISH_EVENT_SOIL,
                       DEFAULT_PUBLISH_ALARM_SOIL, DEFAULT_SUBSCRIBE_CFG_SOILTHR,
                       DEFAULT_SOIL_MOIST_MIN, DEFAULT_SOIL_MOIST_MAX,
                       DEFAULT_SOIL_THRESH, DEFAULT_SOIL_THRESL, 0);

  /* Load commands default */
  irrigation_commands.num = 1;
  memcpy(irrigation_commands.command[IRRIGATION_COMMAND_VALVE].command_name,
         DEFAULT_COMMAND_EVENT_VALVE, strlen(DEFAULT_COMMAND_EVENT_VALVE));
  irrigation_commands.command[IRRIGATION_COMMAND_VALVE].cmd = activate_electrovalve;

  /* Get an event ID for our events */
  irrigation_sensors_data_event = process_alloc_event();
  irrigation_sensors_alarm_event = process_alloc_event();

  /* Start the periodic process */
  etimer_set(&et, DEFAULT_SAMPLING_INTERVAL);

  /* Configure ADC channel for soil moisture measurements */
  SENSORS_ACTIVATE(soil_hum);
  /* Configure GPIO for Relay activation */
  grove_relay_configure();
  
  while(1) {
    PROCESS_YIELD();
    if(ev == PROCESS_EVENT_TIMER && data == &et) {
      poll_sensors();
      etimer_reset(&et);
    }
  }
  PROCESS_END();
}
````

### Setting up the application

This section shows to deploy the application.

For example purposes we are going to use the [Wideye Sabre Ranger BGAN M2M terminal](http://www.groundcontrol.com/BGAN_Wideye_Ranger_M2M.htm), but any BGAN terminal would do.

![](http://www.groundcontrol.com/images/Wideye_Ranger_M2M_Product_01.png)

First power-up the BGAN terminal and open the configuration webserver, for the Wideye Ranger is normally at IP address `192.168.1.35`, suing `admin` as user and `wideye` as password.  Check out the [User Guide](http://www.groundcontrol.com/bgan/Wideye_Sabre_Ranger_M2M_Users_Guide.pdf) and [Installation guide](http://www.groundcontrol.com/bgan/Wideye_Sabre_Ranger_M2M_Installation_Guide.pdf) for details.

After power-up the model will display the signal level and closest satellite details and orientation.  We should orientate the model towards the satellite position using a compass (any mobile application would do!), trying to achieve a signal level over 45dBHz.

![](http://i.imgur.com/jqU6vnCl.png)

After registering to the Network then we should enable the enable the PDP context and enable Internet service, note we need to Activate the `Default Profile`, for which our `APN` will be `bgan.inmarsat.com` and if not provided an Username and Password for the SIM card, just leave blank.

![](http://i.imgur.com/UY14Niw.jpg?1)

We activate the profile and verify we have been assigned an IP address:

![](http://i.imgur.com/krCTUgh.jpg?1)

Now we connect the satellite modem to the inRout Border Router over the Ethernet and power up.

The Border Router when it boots will display the following information if connected over the USB:

````
Contiki-develop-20160421-4-gde85971
Zolertia eth-gw
Rime configured with address 00:12:4b:00:06:16:0f:42
 Net: sicslowpan
 MAC: CSMA
 RDC: nullrdc
NOTICE: 6LBR: Starting 6LBR version 1.4.x (Contiki-develop-20160421-4-gde85971)
INFO: NVM: Reading 6LBR NVM
INFO: NVM: NVM Magic : 2009
INFO: NVM: NVM Version : 5
NOTICE: 6LBR: Log level: 30 (services: ffffffff)
INFO: ETH: ENC28J60 init
INFO: ETH: Eth MAC address : 06:00:06:16:0f:42
INFO: ENC: resetting chip
INFO: ETH: ENC-28J60 Process started
INFO: LLSEC: Using 'nullsec' llsec driver
INFO: 6LBR: Security layer initialized
INFO: 6LBR: Tentative local IPv6 address fe80::212:4b00:616:f42
INFO: 6LBR: Tentative global IPv6 address (WSN) aaaa::212:4b00:616:f42
INFO: 6LBR: Tentative global IPv6 address (ETH) bbbb::100
INFO: 6LBR: RA Daemon enabled
INFO: 6LBR: Checking addresses duplication
INFO: NVM: Flashing 6LBR NVM
ERROR: NVM: verify NVM failed, retry
INFO: NVM: Flashing 6LBR NVM
INFO: 6LBR: Configured as DODAG Root aaaa::212:4b00:616:f42
INFO: 6LBR: Starting IP64
Starting DHCPv4
INFO: 6LBR: Starting as RPL ROUTER
INFO: NODECFG: Node Config init
INFO: HTTP: Starting webserver on port 80
INFO: UDPS: UDP server started
INFO: DNS: DNS proxy started
INFO: 6LBR: CETIC 6LBR Started
INFO: 6LBR: Set IPv4 address : 192.168.137.98
````

Now we have been assigned an IP private address.

We need the `6LBR: Set IPv4 address : 192.168.137.98` information to access the Border Router webservice and explore the options provided by `6lbr`.  First we check the connectivity to the Border Router by making a `ping` to its address:

![](http://i.imgur.com/1qOQBtr.png)

Notice the Border Router also advertise a local IPv6 address (`bbbb::/64` as default), being the default Border Router address the `bbbb:100`.  We can use this address to also talk to the Border Router using IPv6.

Open a browser and using the Border Router IPv4 address you will see the following page:

![](http://i.imgur.com/FxKd4yg.png)

This is the 6lbr webservice showing the Border Router status and configuration options.  We can manage and configure both the IPv4/IPv6 and 6LoWPAN networks, and also manage the wireless nodes in our 6LoWPAN network.

![](http://i.imgur.com/MP48leg.png)

In the `Sensors` tab the devices in our 6LoWPAN network are shown.  The Border Router uses static address mapping, allowing to address each device in the network from our IPv4 network (IPv6 individual addressing is out of the box).  If the Border Router has been assigned over `DHCP` the address `192.168.1.80`, then it will start mapping nodes in the network as `192.168.1.80:20000`.

This is important to remember, as mentioned before the MQTT client exposes a provisioning webserver.  Following the example above, the configuration webserver should be accessible at `192.168.1.80:20000/config.html`.  Opening a different URL will return a `404 not found` error message.

![](http://i.imgur.com/PTprQ0L.png)

We are officially connected to Inmarsat global satellite network! the next steps will detail the ready to use Use Case examples, and how to deploy your own.

## Example Use Cases implemented

The following are just the first use cases implemented, providing real-world applications and business logic:

### RE-Mote demonstrator

Exposes the hardware functionalities and allows to quick-dive into your very own RE-Mote platform.  This example does not require any external sensors, so it is recommended as the first one to test.

![](http://i.imgur.com/ZEt8h3kl.jpg)

* Publish periodically the device ID, parent address, RSSI
* Publish periodically the internal core temperature and battery level
* Publish periodically the on-board ADC (analogue to digital converter) values (ADC1 and ADC3 ports), you can connect any sensor and see its value.
* Remotely toggle the LEDs using commands
* Remotely change the periodic publish interval

### Cold chain monitoring

Remotely monitor the temperature and humidity of refrigerated units used to store and transport sensitive material, like medicines, organs and food supplies.

![](http://i.imgur.com/MdSc0hEl.jpg)
> Image taken from [The Daily Star](http://www.thedailystar.net/cold-chain-can-save-food-supply-chain-41858)

* Publish periodically the device ID, parent address, RSSI
* Publish periodically the temperature and humidity values
* Publish periodically the configured temperature and humidity threshold, used to check for values over the configured threshold and send an alarm (a publication to a separate alarm topic).
* Remotely toggle the LEDs using commands
* Remotely change the periodic publish interval
* Enable and disable the sensors remotely

This application requires the [Temperature and Humidity sensor: SHT21/SHT25](http://zolertia.io/product/sensors/temp-humidity-sensor-sht21) sensor.

### Basic agriculture weather monitoring

Monitor the weather conditions of your crops, greenhouses and gardens remotely.  Smartening your agriculture process will optimize for the best quality of agricultural products.  Use with the Irrigation use case to also control the soil moisture level of your crops.

![](http://i.imgur.com/ruiXohJ.png)

* Publish periodically the device ID, parent address, RSSI
* Publish periodically the temperature, humidity, ambient light and atmospheric pressure values
* Publish periodically the configured temperature, humidity, light and atmospheric pressure thresholds, used to check for values over the configured threshold and send an alarm (a publication to a separate alarm topic)
* Remotely toggle the LEDs using commands
* Remotely change the periodic publish interval
* Enable and disable the sensors remotely

This application requires the following sensors:

* [Temperature and Humidity sensor: SHT21/SHT25](http://zolertia.io/product/sensors/temp-humidity-sensor-sht21)
* [Ambient light sensor: TSL2563](http://zolertia.io/product/sensors/tsl-light-sensor-0)
* [Atmospheric pressure sensor: BMP180](http://zolertia.io/product/barometer-sensor)

### Irrigation control

The Irrigation application allows to control and monitor the moisture of your soil and remotely watering your plants.  Use together with the Basic Agriculture application for further results.

![](http://i.imgur.com/rkk9vEJl.jpg)
> Image taken from [Donna Vincent Roa](http://donnavincentroa.com/explosive-smart-irrigation/)

* Publish periodically the device ID, parent address, RSSI
* Publish periodically the soil moisture level
* Publish periodically the configured soil moisture threshold, used to check for values over the configured threshold and send an alarm (a publication to a separate alarm topic)
* Remotely activate an electrovalve and water your plans
* Remotely toggle the LEDs using commands
* Remotely change the periodic publish interval
* Enable and disable the sensors remotely

This application requires the following:

* [Sparkfun Soil Moisture sensor](https://www.sparkfun.com/products/13322)
* [Sparkfun solenoid valve](https://www.sparkfun.com/products/10456)


## Supported IoT cloud platforms

The following are the IoT cloud platforms supported out of the box by the **inRout** solution, this list will continue to grow overtime.

### IBM Bluemix

IBM Bluemix quickstart allows to directly send and visualize your data without having to register, a truly plug and play platform service.  To visualize the data we only need to retrieve the `client id`, shown in the next 

````
Rime configured with address 00:12:4b:00:06:15:ab:25
 Net: sicslowpan
 MAC: CSMA
 RDC: nullrdc

Zolertia MQTT client
  Broker IP:    ::ffff:b8ac:7cbd
  Broken port:  1883
Connecting to the WSN network... 

Started MQTT webserver for provisioning
Warning: No hardcoded Auth User
Warning: No hardcoded Auth Token
Client: not found in flash

IBM bluemix process started
  Client ID:    d:quickstart:Zolertia:00124b15ab25
  Data topic:   iot-2/evt/data/fmt/json
````

![](http://i.imgur.com/K3rtRtE.png)

### Relayr.io

````
Contiki-3.x-2283-gcebe6c5
Zolertia RE-Mote platform
Rime configured with address 00:12:4b:00:06:15:ab:25
 Net: sicslowpan
 MAC: CSMA
 RDC: nullrdc

Zolertia MQTT client
  Broker IP:    ::ffff:36ab:7f82
  Broken port:  1883
Connecting to the WSN network... 

Started MQTT webserver for provisioning
Warning: No hardcoded Auth User
Warning: No hardcoded Auth Token
Client: not found in flash
No client information found!
Awaiting provisioning over the httpd webserver
````

![](http://i.imgur.com/PVa4On5.png)

![](http://i.imgur.com/Ma96LtK.png)

![](http://i.imgur.com/iikexu5.png)

![](http://i.imgur.com/JBbqFUs.png)

![](http://i.imgur.com/xHlv7vQ.png)

![](http://i.imgur.com/t9CzEgS.png)

![](http://i.imgur.com/BxcJ9pm.png)


````
No client information found!
Awaiting provisioning over the httpd webserver
Client: New Auth User config --> fee8c814-6c31-4469-9bd8-54e3a6ff10cb
Client: saved in flash (MW 0xABCD, CRC16 59049, len 124)
Auth User --------------> fee8c814-6c31-4469-9bd8-54e3a6ff10cb
Auth Token -------------> 
Pub Interval -----------> 45
*** New configuration over httpd
Client: New Auth Token config --> d51S7bd1Eh5i
Client: saved in flash (MW 0xABCD, CRC16 28316, len 124)
Auth User --------------> fee8c814-6c31-4469-9bd8-54e3a6ff10cb
Auth Token -------------> d51S7bd1Eh5i
Pub Interval -----------> 45
*** New configuration over httpd
Configuration found, continuing...

Relayr process started
  Client ID:    00124b15ab25
  Data topic:   /v1/fee8c814-6c31-4469-9bd8-54e3a6ff10cb/data
  Cmd topic:    /v1/fee8c814-6c31-4469-9bd8-54e3a6ff10cb/cmd
````



````
Client: Publish [{"meaning":"ID","value":"Zolertia RE-Mote"},{"meaning":"core_temp","value":"35.71"},{"meaning":"battery","value":"3.29"},{"meaning":"ADC1","value":"2.32"},{"meaning":"ADC3","value":"1.50"},{"meaning":"uptime","value":"1146"},{"meaning":"parent","value":"fe80::212:4b00:616:f42"},{"meaning":"rssi","value":"-48"}] to /v1/fee8c814-6c31-4469-9bd8-54e3a6ff10cb/data
````

````
RE-Mote: ...and button released!
Relayr: Alarm! button_pres --> 12
Client: Publish [{"meaning":"button_pres","value":12.00}] to /v1/fee8c814-6c31-4469-9bd8-54e3a6ff10cb/data
````

````
MQTT - Got PUBLISH, called once per manageable chunk of message.
MQTT - Handling publish on topic '/v1/fee8c814-6c31-4469-9bd8-54e3a6ff10cb/cmd'
MQTT - This chunk is 35 bytes
Client: Application received a publish on topic '/v1/fee8c814-6c31-4469-9bd8-54e3a6ff10cb/cmd'. Payload size is 35 bytes. Content:

Relayr: Pub Handler, topic='/v1/fee8c814-6c31-4469-9bd8-54e3a6ff10cb/cmd' (len=44), chunk='{"name":"leds_toggle","value":true}', chunk_len=35
Relayr: Command received --> toggle LED
````

### TheThings.io

````
Contiki-3.x-2283-gcebe6c5
Zolertia RE-Mote platform
Rime configured with address 00:12:4b:00:06:15:ab:25
 Net: sicslowpan
 MAC: CSMA
 RDC: nullrdc

Zolertia MQTT client
  Broker IP:    ::ffff:36ab:7f82
  Broken port:  1883
Connecting to the WSN network... 

Started MQTT webserver for provisioning
Warning: No hardcoded Auth User
Warning: No hardcoded Auth Token
Client: not found in flash
No client information found!
Awaiting provisioning over the httpd webserver
````

![](http://i.imgur.com/KMVCtHk.png)

![](http://i.imgur.com/8ZKf82E.png)

![](http://i.imgur.com/bId9aMx.png)

![](http://i.imgur.com/2CRu9QR.png)

![](http://i.imgur.com/NE6lvFF.png)

![](http://i.imgur.com/rE6mFzz.png)

````
Client: New Auth User config --> oR_GWgW1A5clNAEy7sFHNejXTl0z1XdnD89KXk9D-RI
Client: saved in flash (MW 0xABCD, CRC16 65531, len 120)
Auth User --------------> oR_GWgW1A5clNAEy7sFHNejXTl0z1XdnD89KXk9D-RI
Auth Token -------------> 
Pub Interval -----------> 45
*** New configuration over httpd
Configuration found, continuing...

The Things.io process started
  Client ID:    00124b15ab25
  Data topic:   v2/things/oR_GWgW1A5clNAEy7sFHNejXTl0z1XdnD89KXk9D-RI
  Cmd topic:    v2/things/oR_GWgW1A5clNAEy7sFHNejXTl0z1XdnD89KXk9D-RI/cmd
````

![](http://i.imgur.com/OH1pX0H.png)

![](http://i.imgur.com/i0V4Y1C.png)

![](http://i.imgur.com/sjEyUzr.png)
