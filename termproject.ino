#include <Wire.h>
#include <TridentTD_LineNotify.h>
#include "MAX30105.h"  //Get it here: http://librarymanager/All#SparkFun_MAX30105
#include "heartRate.h"

#define SSID        "Galaxy A71EA85"
#define PASSWORD    "nevgneov"
#define LINE_TOKEN  "GbGv8EmE6A2X5EwMTcDywVDafc8FucC76dGdvQ4xFf8"

MAX30105 particleSensor;

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;

void setup()
{
  Serial.begin(9600);
  Serial.println("Initializing...");

  // Initialize sensor
  if (particleSensor.begin(Wire, I2C_SPEED_FAST) == false) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  
  particleSensor.setup(0); //Configure sensor. Turn off LEDs
  particleSensor.enableDIETEMPRDY(); //Enable the temp ready interrupt. This is required.
  
  Serial.begin(115200); Serial.println();
  Serial.println(LINE.getVersion());
  Serial.println("Initializing...");

  WiFi.begin(SSID, PASSWORD);
  Serial.printf("WiFi connecting to %s\n",  SSID);
  while(WiFi.status() != WL_CONNECTED) { Serial.print("."); delay(400); }
  Serial.printf("\nWiFi connected\nIP : ");
  Serial.println(WiFi.localIP());  
  
  LINE.setToken(LINE_TOKEN);
  LINE.notifySticker("สวัสดี IMI(^ ^)",2,514);

  // Initialize sensor
  particleSensor.begin(Wire, I2C_SPEED_STANDARD);
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
}

void loop()
{
  String val = "";
  float temperature = particleSensor.readTemperature();

  Serial.print("temperature=");
  Serial.print(temperature, 4);
  Serial.println();
  
  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true)
  {
    long delta = millis() - lastBeat;
    lastBeat = millis();
    beatsPerMinute = 60 / (delta / 1000.0);
    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable
    }
  }
  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);

  if (irValue < 50000)
    Serial.print(" No finger?");
    
  val = val + temperature;
  val = val + "C";
  //Serial.println(val);
  if(irValue>=10000){
    String text = "วางนิ้วแล้ว \nBPM = ";
    text.concat(beatsPerMinute);
    text.concat("\n");
    text.concat("Temperature : ");
    text.concat(val);
    LINE.notify(text);
    delay(60000);
  }
}
