/*  
  MySQL Connector/Arduino Example : connect by wifi
  This example demonstrates how to connect to a MySQL server from an
  ESP8266 equipped with a BME280 connected via I2C. In this case I am using a
  D1 mini Pro.
  INSTRUCTIONS FOR USE
  1) Change the address of the server to the IP address of the MySQL server
  2) Change the user and password to a valid MySQL user and password
  3) Change the SSID and pass to match your WiFi network
  4) Connect a USB cable to your ESP8266
  5) Select the correct board and port
  6) Compile and upload the sketch to your ESP8266
  7) Once uploaded, open Serial Monitor (use 115200 speed) and observe
  If you do not see messages indicating you have a connection, refer to the
  manual for troubleshooting tips. The most common issues are the server is
  not accessible from the network or the user name and password is incorrect.
  Created by: Dr. Charles A. Bell
  Modified by: Derek
*/
#include <ESP8266WiFi.h>           // Use this for WiFi instead of Ethernet.h
//#include <WiFiInfo.h>              //TODO: this stores information and passwords for connetctions 

#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C

// constants won't change:
const unsigned short ledInterval    = 1000;  // interval at which to blink (milliseconds)
const unsigned long readDelayTime   = 600000;// interval between readings (milliseconds) 600,000 equals 10 minutes
const unsigned short ledBlinkTime   = 100;   // time for the led to be on (milliseconds)
const unsigned short blinkDelayTime = 1000;  // interval between blinks (milliseconds)

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned short previousMillis = 0;   // store last time LED was updated
bool ledState                 = LOW; // ledState used to set the LED

IPAddress server_addr(192,168,0,85);  // IP of the MySQL *server* here
char user[]     = "pi_insert";        // MySQL user login username
char password[] = "raspberry";        // MySQL user login password

// Sample query
char INSERT_DATA[] = "INSERT INTO measurements.pressure (pressure, temperature, humidity) VALUES (%u, %u, %u)";
char query[128];
unsigned int pressure;    // the value is stored as an it to save space in the database. divide by 100 to get actual value
unsigned int temperature; // the value is stored as an it to save space in the database. divide by 100 to get actual value
unsigned int humidity;    // the value is stored as an it to save space in the database. divide by 10  to get actual value

// WiFi card example
char ssid[] = "Sloan-Home"; // your SSID
char pass[] = "04052002";   // your SSID Password

WiFiClient client;          // Use this for WiFi instead of EthernetClient
MySQL_Connection conn(&client);
MySQL_Cursor* cursor;

void connectWiFi()
{
  // Begin WiFi section
  Serial.printf("\nConnecting to %s", ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // print out info about the connection:
  Serial.println("\nConnected to network");
  Serial.print("My IP address is: ");
  Serial.println(WiFi.localIP());
}

void connectSQL()
{
  Serial.print("Connecting to SQL...  ");
  if (conn.connect(server_addr, 3306, user, password))
    Serial.println("OK.");
  else
    Serial.println("FAILED.");
}

void setup()
{
  Serial.begin(115200);
  Serial.println(F(" ESP8266MYSQLBME280.ino "));

  bool status;

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
 
  // default settings
  // (you can also pass in a Wire library object like &Wire2)
  status = bme.begin(0x76); //0x77 is the default address if nothing is specified
  if (!status) {
      Serial.println("Could not find a valid BME280 sensor, check wiring!");
      while (1);
  }

  connectWiFi();

  connectSQL();
}

void loop()
{
  if (conn.connected())
  {
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED on by connecting it to ground

    pressure = bme.readPressure()*0.029529983071445;       // to convert from pascals to inches of mercury divide by 0.00029529983071445
    temperature = ((bme.readTemperature()* 9/5) + 32)*100; // convert to farinheight and multiply by 100
    humidity = bme.readHumidity()*10;                      // multiply the humidity by 10 
 
    // create MySQL cursor object
    cursor = new MySQL_Cursor(&conn);
    // Save
    //dtostrf(50.125, 1, 1, temperature);
    sprintf(query, INSERT_DATA, pressure, temperature, humidity);
    Serial.printf("Pressure: %.2finHg\n", pressure/100.0);      // must use 100.0 so the result is a float
    Serial.printf("Temperature: %.2f° F\n", temperature/100.0); // insert the degre symbol by typing alt+0176
    Serial.printf("Humidity: %.1f%%\n", humidity/10.0);         // add the %% to get the % to print
    Serial.printf("Free heap: %u\n\n", ESP.getFreeHeap());
    //Serial.println(query);
    cursor->execute(query);  // Execute the query
    // Note: since there are no results, we do not need to read any data
    delete cursor; //! Deleting the cursor also frees up memory used unit will run out of memory and
                   //! crash processor if the memory gets used on every loop
    digitalWrite(LED_BUILTIN, HIGH);    // turn the LED off by disconnecting it from ground
  }
  else
  {
  Serial.println("SQL connection dropped");
  Serial.println("Attempting to reconnect");
  connectWiFi();
  connectSQL();
  }  
  delay(readDelayTime);
}