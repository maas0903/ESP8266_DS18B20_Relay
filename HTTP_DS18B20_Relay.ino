/*   ESP8266 with DS18B20 thermometer
     ================================

     Using histeresis to switch a relay on and off between min and max temperatures depending on going up or down

     GPIO2 input of the sensor
     GPIO0 output to relay

     Add credentials.h file to the same directory as the ino file with this content:
     //Put your SSID & Password
     const char* ssid = "yourn ssid";  // Enter SSID here
     const char* password = "your password";  //Enter Password here

     Create and edit config.h in the same folder as the ino file to contain the sensor device id and min and max temperatures. For example,
     //Sensor ids
     uint8_t sensor1[8] = {0x28, 0xFF, 0x75, 0x57, 0x80, 0x16, 0x04, 0xEC};
     float maxTemp = 22;
     float minTemp = 21;
*/

#include <ESP8266WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include "credentials.h"
#include "config.h"

#define ONE_WIRE_BUS 2
#define RELAY_BUS 0

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float tempSensor1;
boolean goingUp = false;
String statusStr = "";
int deviceCount = 0;

DeviceAddress Thermometer;

ESP8266WebServer server(80);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    Serial.print("0x");
    if (deviceAddress[i] < 0x10) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
    if (i < 7) Serial.print(", ");
  }
  Serial.println("");
}

void setup()
{
  Serial.begin(115200);
  pinMode(RELAY_BUS, OUTPUT);
  digitalWrite(RELAY_BUS, HIGH);

  delay(100);

  timeClient.begin();

  sensors.begin();

  deviceCount = sensors.getDeviceCount();
  Serial.print(deviceCount, DEC);
  Serial.println(" devices.");
  Serial.println("");

  Serial.println("Printing addresses...");
  for (int i = 0;  i < deviceCount;  i++)
  {
    Serial.print("Sensor ");
    Serial.print(i + 1);
    Serial.print(" : ");
    sensors.getAddress(Thermometer, i);
    printAddress(Thermometer);
  }

  Serial.println("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());

  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop()
{
  server.handleClient();
  delay(1000);
}

void handle_OnConnect() {
  sensors.requestTemperatures();
  tempSensor1 = sensors.getTempC(sensor1); // Gets the values of the temperature

  //handle temperature histeresis
  if (goingUp)
  {
    if (tempSensor1 >= maxTemp)
    {
      goingUp = false;
      statusStr = "Phase change from going up to going down";
    } else
    {
      //swith warming on
      digitalWrite(RELAY_BUS, LOW);
      statusStr = "going up - " + String(tempSensor1) + " < (Upper) " + String(maxTemp) + " Pad is ON";
    }
  } else
  {
    if (tempSensor1 <= minTemp)
    {
      statusStr = "Phase change from going down to going up";
      goingUp = true;
    } else
    {
      //swith warming off
      digitalWrite(RELAY_BUS, HIGH);
      statusStr = "going down - " + String(tempSensor1) + " > (Lower)" + String(minTemp) + " Pad is OFF";
    }
  }

  timeClient.update();
  String formattedTime = timeClient.getFormattedTime() + " UTC";
  statusStr = formattedTime + " - " + statusStr;
  Serial.println(statusStr);

  server.send(200, "text/html", SendHTML(tempSensor1, statusStr));

}

void handle_NotFound()
{
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float tempSensor1, String statusStr)
{
  String ptr = "<!DOCTYPE html>";
  ptr += "<html>";
  ptr += "<head>";
  ptr += "<title>ESP8266 with DS18B20 Temperature Monitor and Relay Switch</title>";
  ptr += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  ptr += "<link href='https://fonts.googleapis.com/css?family=Open+Sans:300,400,600' rel='stylesheet'>";

  ptr += "<style>";
  ptr += "html { font-family: 'Open Sans', sans-serif; display: block; margin: 0px auto; text-align: center;color: #444444;}";
  ptr += "body{margin-top: 50px;} ";
  ptr += "h1 {margin: 50px auto 30px;} ";
  ptr += ".side-by-side{display: table-cell;vertical-align: middle;position: relative;}";
  ptr += ".text{font-weight: 600;font-size: 19px;width: 200px;}";
  ptr += ".temperature{font-weight: 300;font-size: 50px;padding-right: 15px;}";
  ptr += ".Sensor1 .temperature{color: #3B97D3;}";
  ptr += ".superscript{font-size: 17px;font-weight: 600;position: absolute;right: -5px;top: 15px;}";
  ptr += ".data{padding: 10px;}";
  ptr += ".container{display: table;margin: 0 auto;}";
  ptr += ".icon{width:82px}";
  ptr += "</style>";

  //AJAX to auto refresh the body
  ptr += "<script>\n";
  ptr +=  "setInterval(loadDoc,1000);\n";
  ptr +=  "function loadDoc() {\n";
  ptr +=    "var xhttp = new XMLHttpRequest();\n";
  ptr +=    "xhttp.onreadystatechange = function() {\n";
  ptr +=    "if (this.readyState == 4 && this.status == 200) {\n";
  ptr +=      "document.body.innerHTML =this.responseText}\n";
  ptr +=    "};\n";
  ptr +=    "xhttp.open(\"GET\", \"/\", true);\n";
  ptr +=    "xhttp.send();\n";
  ptr +=  "}\n";
  ptr += "</script>\n";

  ptr += "</head>";

  ptr += "<body>";
  ptr +=   "<h1>ESP8266 with DS18B20 Temperature Monitor and Relay Switch</h1>";
  ptr +=   "<div class='container'>";

  ptr +=     "<div class='data Sensor1'>";
  ptr +=     "<div class='side-by-side text'>Sensor 1</div>";
  ptr +=     "<div class='side-by-side temperature'>";
  ptr +=          (int)tempSensor1;
  ptr +=          "<span class='superscript'>&deg;C</span></div>";
  ptr +=   "</div>";

  ptr +=   "<div class='data Sensor1'>";
  ptr +=      "<div class='side-by-side text'>Status</div>";
  ptr +=      "<div class='side-by-side text'>" + statusStr + "</div>";
  ptr +=   "</div>";
  ptr +=  "</div>";

  ptr += "</body>";
  ptr += "</html>";
  return ptr;
}
