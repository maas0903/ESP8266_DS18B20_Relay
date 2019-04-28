ESP8266-01 with DS18B20 thermometer
===================================

Using histeresis to switch a relay on and off between min and max temperatures depending on going up or down

Use with the ESP-01S Relay board

GPIO2 input of the sensor. Sensor pulled up to 3V3 through a 4k7 Omm resistor.
GPIO0 output to relay
   
Add credentials.h file to the same directory as the ino file with this content:
//Put your SSID & Password
const char* ssid = "yourn ssid";  // Enter SSID here
const char* password = "your password";  //Enter Password here
   
Edit config.h in the same folder as the ino file to contain the sensor device id and min and max temperatures. For example,
//Sensor ids
uint8_t sensor1[8] = {0x28, 0xFF, 0x75, 0x57, 0x80, 0x16, 0x04, 0xEC};
float maxTemp = 22;
float minTemp = 21;
