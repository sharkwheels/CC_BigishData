/* ======================= INCLUDES ================================= */
// ditched the config.h while trying to troubleshoot something. Just included the wi-fi client.
#include <AdafruitIO_WiFi.h>

/* ======================= IO ADAFRUIT ================================= */

#define IO_USERNAME    "you"
#define IO_KEY         "key"

#define WIFI_SSID       "web"
#define WIFI_PASS       "pass"



// Set up your Client
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

// Set up your feed
AdafruitIO_Feed *gasReading = io.feed("gasReading");   
AdafruitIO_Feed *voltage = io.feed("gasReading");                    
/* ======================= VARIABLES ================================= */

const int ledPin =  0;             // the number of the LED pin
const int gasPin = A0;             // gas pin
int sendRate = 300000;              // every five minutes
int lastSend;                      // last send
int ledState = LOW;
int gasVal;
int THRESHOLD = 80;
int lastGasVal;

/* ======================= WRITE GAS ================================= */

void setup() {

  // start the serial connection
  Serial.begin(115200);
  pinMode(ledPin,OUTPUT);
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
  // set inital gas value reading:
  lastGasVal = analogRead(gasPin);
  Serial.println("lastGasVal: ");
  Serial.print(lastGasVal);

}

void loop() 
{
  io.run();

  // poll the sensor every 5 seconds
  
  if(millis()-lastSend>=sendRate)
  {
    lastSend = millis();
    // get the new reading
    gasVal = analogRead(gasPin);
    gasReading->save(gasVal);
    Serial.print("sending -> ");
    Serial.println(gasVal);
    
    // still alive heartbeat blink the LED w/ the polling.
    if (ledState == LOW)
    {
      ledState = HIGH;
    } else{
      ledState = LOW;
    }
    digitalWrite(ledPin, ledState); 
  }
}

/*
if(gasVal != lastGasVal){
      gasReading->save(gasVal);
      Serial.print("sending -> ");
      Serial.println(gasVal);
      
    }
*/

// if this gas value is ABOVE your threshold
      /*if(gasVal > THRESHOLD){
        // Write to AdafruitIO
        gasReading->save(gasVal);
        Serial.print("sending -> ");
        Serial.println(gasVal);
      } */
/*if(gasVal > THRESHOLD){
      if(gasVal != lastGasVal){
        gasReading->save(gasVal);
        Serial.print("sending -> ");
        Serial.println(gasVal);
      }
    } */
