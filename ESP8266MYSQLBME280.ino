/*
  MySQL Connector/Arduino Example : connect by wifi
  This example demonstrates how to connect to a MySQL server from an
  Arduino using an Arduino-compatible Wifi shield. Note that "compatible"
  means it must conform to the Ethernet class library or be a derivative
  thereof. See the documentation located in the /docs folder for more
  details.
  INSTRUCTIONS FOR USE
  1) Change the address of the server to the IP address of the MySQL server
  2) Change the user and password to a valid MySQL user and password
  3) Change the SSID and pass to match your WiFi network
  4) Connect a USB cable to your Arduino
  5) Select the correct board and port
  6) Compile and upload the sketch to your Arduino
  7) Once uploaded, open Serial Monitor (use 115200 speed) and observe
  If you do not see messages indicating you have a connection, refer to the
  manual for troubleshooting tips. The most common issues are the server is
  not accessible from the network or the user name and password is incorrect.
  Created by: Dr. Charles A. Bell
*/
#include <ESP8266WiFi.h>           // Use this for WiFi instead of Ethernet.h
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C

unsigned long delayTime = 10000;

IPAddress server_addr(192,168,0,18);  // IP of the MySQL *server* here
char user[] = "pi_insert";              // MySQL user login username
char password[] = "raspberry";        // MySQL user login password

// Sample query
char INSERT_DATA[] = "INSERT INTO measurements.pressure (pressure, temperature, humidity) VALUES (%u, %u, %u)";
char query[128];
unsigned int pressure; // the value is stored as an it to save space in the database. divide by 100 to get actual value
unsigned int temperature; // the value is stored as an it to save space in the database. divide by 100 to get actual value
unsigned int humidity; // the value is stored as an it to save space in the database. divide by 10 to get actual value

// WiFi card example
char ssid[] = "Sloan-Home";   // your SSID
char pass[] = "04052002";     // your SSID Password

WiFiClient client;                 // Use this for WiFi instead of EthernetClient
MySQL_Connection conn(&client);
MySQL_Cursor* cursor;

void setup()
{
  Serial.begin(115200);
    Serial.println(F(" ESP8266MYSQLBME280.ino "));

    bool status;

    // default settings
    // (you can also pass in a Wire library object like &Wire2)
    status = bme.begin(0x76); //0x77 is the default address if nothing is specified
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1);
    }

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

  Serial.print("Connecting to SQL...  ");
  if (conn.connect(server_addr, 3306, user, password))
    Serial.println("OK.");
  else
    Serial.println("FAILED.");
  
  // create MySQL cursor object
  cursor = new MySQL_Cursor(&conn);
}

void loop()
{
  if (conn.connected()){
//    Serial.println(bme.readPressure() / 100.0F); //read pressure and divide by 100.0 floating point
    pressure = bme.readPressure()*0.029529983071445; // to convert from pascals to inches of mercury divide by 0.00029529983071445
    temperature = ((bme.readTemperature()* 9/5) + 32)*100; // convert to farinheight and multiply by 100
    humidity = bme.readHumidity()*10; // multiply the humidity by 10 

    MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
    // Save
    //dtostrf(50.125, 1, 1, temperature);
    sprintf(query, INSERT_DATA, pressure, temperature, humidity);
    Serial.printf("Pressure: %.2finHg\n", pressure/100.0); // must use 100.0 so the result is a float
    Serial.printf("Temperature: %.2f° F\n", temperature/100.0); // insert the degre symbol by typing alt+0176
    Serial.printf("Humidity: %.1f%%\n", humidity/10.0); // add the %% to get the % to print
    Serial.println(query);
    // Execute the query
    cursor->execute(query);
    // Note: since there are no results, we do not need to read any data
    // Deleting the cursor also frees up memory used
  }
    
  delay(delayTime);

}