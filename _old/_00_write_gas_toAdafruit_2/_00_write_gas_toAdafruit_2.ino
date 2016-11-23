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



/* ======================= IO ADAFRUIT ================================= */

#define IO_USERNAME    "IO_USERNAME"
#define IO_KEY         "IO_KEY"

#define WIFI_SSID       "WIFI_SSID"
#define WIFI_PASS       "WIFI_PASS"

// Set up your Client
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

// Set up your feed
AdafruitIO_Feed *gasReading = io.feed("gasReading");   
                   
/* ======================= VARIABLES ================================= */

const int gasPin = A0;             // gas pin
int THRESHOLD = 80;                // base threshold

boolean collectRan = false;         // did we collect all the data?
int collectCount = 0;               // the count

const int numReadings = 5;          // numbers to pull from
int gasVals[numReadings];           // array init
// run Adafruit io. 
// SO! Fun stuff...all the functions you would normally put after loop you gotta put above
// because soomething in this bungs it right up


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

  // start the serial connection
  Serial.begin(115200);
  pinMode(gasPin, INPUT);


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

}

void loop() 
{
  // run Adafruit io. 
  // SO! Fun stuff...all the functions you would normally put after loop you gotta put above
  // because soomething in this bungs it right up
  io.run();
  

  if(!collectRan){
    for(int i=0; i<numReadings; i++){
      // increase the count
      collectCount++;
      // place the analog readings in the gasVals array
      gasVals[i] = analogRead(gasPin);
      Serial.print("toArray-> ");
      Serial.println(gasVals[i]);
      
      // 1 minute delay. 
      // This is fine. I'm only reading one sensor. 
      // If it was multi-tasking i'd have to use a non-blocking method.

      delay(60000);  
    }
    if(collectCount == 5){
      collectRan = true;
    }
  } else if(collectRan){
    Serial.println("collecting data ran");
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
  
}