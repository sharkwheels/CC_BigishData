/*********************************************************************** 
  CREATION COMPUTATION - BIG(ISH) DATA 

 * Gas Sensor for PoopArray v1.0
 * Sends the largest value collected to Adafruit IO every 5 minutes.
 * Very likely a giant battery hog and the sensor will overheat, but oh well. 
 * That's the point of a 1.0 right?
******************************************************************************/


/* ======================= INCLUDES ================================= */
// ditched the config.h while trying to troubleshoot something. Just included the wi-fi client.
#include <AdafruitIO_WiFi.h>
#include <elapsedMillis.h>
#include <Esp.h>

/* ======================= IO ADAFRUIT ================================= */

#define IO_USERNAME    "XX"
#define IO_KEY         "XX"

#define WIFI_SSID       "XX"
#define WIFI_PASS       "XX"

// Set up your Client
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

// Set up your feed
AdafruitIO_Feed *gasReading = io.feed("gasReading");   
                   
/* ======================= VARIABLES ================================= */

/// Gas-Pin Logic ////////////////////////////////////////

const int gasPin = A0;             // gas pin
const int ledPin = 0;


const int numReadings = 5;          // numbers to pull from
int THRESHOLD = 80;                // base threshold
int gasVals[numReadings];           // array init


/// Data Collection Logic ////////////////////////////////////////

boolean systemPause = false;
boolean collectRan = false;         // did we collect all the data?
int collectCount = 0;               // the count
int sensorRestCount = 0;


elapsedMillis sendTimer;
unsigned int sendTimerInterval = 60000; 

elapsedMillis calibrateTimer;
unsigned int calibrateInterval = 180000; // 3 minutes


// Time to sleep (in seconds):
const int sleepTimeS = 180;

/* ======================= FUNCTIONS ================================= */

// Return the Average (pulled from arduino.com)

int theAverage (int * array, int len)  
{
  long sum = 0L ;  // sum will be larger than an item, long for safety.
  for (int i = 0 ; i < len ; i++)
    sum += array [i] ;
  return  ((int) sum) / len ;  
}

// Return the max (pulled from arduino.com)

int maxInArray(int* arr, const int size){
 int max = arr[0];
 for (int i=0; i<size; i++){
   if (max<arr[i]){ max = arr[i]; }
 }
 return max;
} 
/* ======================= WRITE GAS ================================= */

void setup() {
  
  pinMode(gasPin, INPUT);
  pinMode(ledPin, OUTPUT);

  // start the serial connection
  Serial.begin(115200);
 // wait for serial monitor to open
  while(! Serial);
  Serial.print("Connecting to Adafruit IO");
  // connect to io.adafruit.com
  io.connect();
  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  // we are connected
  Serial.println();
  Serial.println(io.statusText());
  delay(100);

  // initialize all the readings to 0:
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    gasVals[thisReading] = 0;
  }

  // hang out for a minute to warm up the sensor a bit
  /*Serial.println("delay for 1 minute to let sensor warm up");
  digitalWrite(ledPin, HIGH);
  delay(5000); 
  digitalWrite(ledPin, LOW);*/
  // Rest the send data timer...

  gasReading->save(0); // reset Adafruit IO reading so you know it restarted

  sendTimer = 0;
  Serial.println("setup done");
  Serial.println("waiting one minute for data collection to begin...");
}

void loop() 
{
  // run Adafruit io. 
  // SO! Fun stuff...all the functions you would normally put after loop you gotta put above
  // because soomething in this bungs it right up
  io.run();
  
  if(!systemPause){
    if(!collectRan)
    {
      for(int i=0; i<numReadings; i++)
      {
        if (sendTimer > sendTimerInterval) 
        {
            sendTimer -= sendTimerInterval; //reset the timer
            // increase the count
            collectCount++;
            // place the analog readings in the gasVals array
            gasVals[i] = analogRead(gasPin);
            Serial.print("toArray-> ");
            Serial.println(gasVals[i]);
          } 
      }
      if(collectCount == 5)
      {
        collectRan = true;
      }
    } else if(collectRan)
    {

      sensorRestCount++;
      Serial.println("sensorRestCount: ");
      Serial.println(sensorRestCount);
      
      if(sensorRestCount == 12){
        systemPause = true; 
      }

      Serial.println("Sending Data to IO");

      int gasThing = maxInArray(gasVals,numReadings);          
      Serial.print("sending-> ");
      Serial.println(gasThing);
      gasReading->save(gasThing);

      Serial.println("reset collection");
      collectCount = 0;
      collectRan = false;
        
    } else {
      Serial.println("WTF");
      
    }
  } else if(systemPause) {
    Serial.println("the system is resting");
    // deepSleep time is defined in microseconds. Multiply
    // seconds by 1e6 
    // SLEEP FOR 3 MINUTES..
    ESP.deepSleep(sleepTimeS * 1000000,WAKE_RF_DEFAULT);
    sendTimer = 0;
    systemPause = false;
  } else {
    Serial.println("something is wrong");
  }
  
  
}
