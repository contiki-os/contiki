#include "Particle.h"
#include "sdfat/SdFat.h" /*the sdfat folder is in the base folder with the .ino file*/
#include <ctype.h>
#include "application.h"

SYSTEM_MODE(MANUAL); /*do not autoconnect to particle cloud*/
SYSTEM_THREAD(ENABLED);
STARTUP(cellular_credentials_set("internet", "", "", NULL));

// Primary SPI with DMA
// SCK => A3, MISO => A4, MOSI => A5, SS => A2 (default)
SdFat sd;
const uint8_t chipSelect = SS;


String data="", errormsg="", systime="";
File myFile;
int rep_count=0,delcount=0, ind=0,filesize=0;
double SOC=100.0;
bool newdata=false, client_connected=false, cellular_is_on=false, fuelGaugeIsAsleep=false;
int n=0; /*increments sleep periods during low power conditions*/

/*sd card settings*/
String  temp="", apn="", user="",pass="";
int reps=0;


/*TCP configuration*/
TCPClient client;
String server = "wimea.mak.ac.ug"; //default server
int port=10005; // default port
byte buff [1024];  /*see transmission metrics regarding choice of 1024*/

FuelGauge fuel;
CellularSignal sig;

Thread* superVthread;
unsigned long lastTime = 0, now=0;

os_thread_return_t sysreset(void* param){
	for(;;){
		if(cellular_is_on){
			now = millis();
			if((now-lastTime > 100000) && client_connected==false)
			{
				System.reset();
			}
		}
		delay(1000);
	}
}


void setup() {
	Serial1.begin(38400);
	Cellular.on();
	Cellular.off();

	if (!sd.begin(chipSelect, SPI_HALF_SPEED)) {
		errormsg += "SD Begin Operation Failed. ";
		RGB.control(true);
		for(int j=0; j<5;j++){ //5 RED blinks
			RGB.color(255, 0, 0);
			delay(50);
			RGB.color(0, 0, 0);
			delay(50);
		}
		RGB.control(false);
	}

	SOC=fuel.getSoC();
	superVthread = new Thread("superVthread", sysreset, NULL);

if(myFile.open("config.txt", O_READ))
{
	int d;
	while ((d = myFile.read()) >= 0)
	{
		temp.concat((char)d);
		if(d=='\n')
		{
			if(temp.indexOf("APN")>-1)
			{
			  apn = temp.substring(4).replace('\n','\0');
			}
			if(temp.indexOf("USER")>-1)
			{
			  user = temp.substring(9).replace('\n','\0');
			}
			if(temp.indexOf("PASS")>-1)
			{
			  pass = temp.substring(5).replace('\n','\0');
			}
			if(temp.indexOf("SERVER")>-1)
			{
			  server = temp.substring(7).replace("\n","");
				server.remove(server.length()-1); /*remove the character at the last index if its a null terminator*/
			}
			if(temp.indexOf("UPLOAD")>-1)
			{
			  reps = atoi(temp.substring(12));
			}
			if(temp.indexOf("PORT")>-1)
			{
			  port = atoi(temp.substring(5));
			}
		    temp="";
		}
	}
	myFile.close();

}
else
errormsg+="Failed to open config file.";
}

void loop() {

	if(fuelGaugeIsAsleep){/*Fuel gauge must wake up to measure V_BAT and SOC*/
		fuel.wakeup();
		fuelGaugeIsAsleep=false;
	}

	if (!myFile.open("sensors.dat", O_RDWR | O_CREAT | O_AT_END)) {
		errormsg += "Opening SD File for write failed. ";
		RGB.control(true);
		for(int j=0; j<5;j++){ //5 fast BLUE blinks
			RGB.color(0, 0, 255);
			delay(50);
			RGB.color(0, 0, 0);
			delay(50);
		}
		RGB.control(false);
	}
char c;
	while(1){
		if(Serial1.available())
		{
			newdata=true;
			c = Serial1.read();
			data +=c;
			if(c=='\n')
			{
				if((ind = data.indexOf("RTC_T")) > -1)
				{
					myFile.print(data);
					systime=data.substring(ind, ind+26);
					++rep_count;
					if(rep_count % 50 == 0)
					{
						myFile.println(systime + " TXT=electron V_BAT="+String(fuel.getVCell()) + " SOC="+(SOC=fuel.getSoC()));
					}
					data="";
					newdata=false;
				} else data="";
			}
		}
		if(newdata==false) //wait 200ms for any other report
		{
			delay(2);
			++delcount;
			if(delcount > 100)
			{
				delcount=0;
				break;
			}
		}
	}

	if(rep_count >= reps && SOC >= 15.0)
	{
		rep_count=0;
		n=0; //reset any previous value of n
		data="";
		Cellular.on();
		lastTime=millis();  //get the time we turned on the module
		cellular_is_on = true;
		Cellular.connect();
		while(!Cellular.ready()); //wait until condition is true
		if (client.connect(server, port))
		{
			client_connected=true;
		}
		sig = Cellular.RSSI();
		myFile.close();
		myFile.open("sensors.dat", O_READ);
		filesize = myFile.size();
		client.println(systime + "TXT=electron RSSI=" + sig.rssi + " QUAL="+sig.qual + " FILE_SIZE="+filesize);

		int d, i=0;
		while ((d = myFile.read()) >= 0) {
			buff[i]=d;
			i++;
			if(i==1024){
				client.write(buff, i);
				i=0;
				memset(buff,0,sizeof(buff));
			}
		}
		client.write(buff, i);//write remainder
		memset(buff,0,sizeof(buff));
		client.println("DISCONNECT");
		Cellular.off();
		cellular_is_on=false;
		if(client_connected) //rough assumption. all data was sent
		{
			sd.remove("sensors.dat");
			System.reset();
		}
	}

	myFile.close();
	if(SOC < 5.0){
		++n;
		fuelGaugeIsAsleep=true;
		System.sleep(SLEEP_MODE_SOFTPOWEROFF, 3600*n); // check again after 3600*n seconds
	} else
	{
		System.sleep(D1,RISING,3000); //normal operation. wake up on D1
	}
	//nothing below here is executed
}
