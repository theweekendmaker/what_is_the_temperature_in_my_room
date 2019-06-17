/*
   Copyright (c) 2015, Majenko Technologies
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

 * * Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer.

 * * Redistributions in binary form must reproduce the above copyright notice, this
     list of conditions and the following disclaimer in the documentation and/or
     other materials provided with the distribution.

 * * Neither the name of Majenko Technologies nor the names of its
     contributors may be used to endorse or promote products derived from
     this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "DHT.h"

#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
float temp_data_array[24];

#ifndef STASSID
#define STASSID "<wifi ssid i.e. name>"
#define STAPSK  "<wifi password>"
#endif

const char *ssid = STASSID;
const char *password = STAPSK;

ESP8266WebServer server(80);

unsigned long period = 10000; // interval between succesive reading of temp
unsigned long prev_time = millis();

const int led = 13;

void handleRoot() {
  digitalWrite(led, 1);
  char temp[2000];
  
  snprintf(temp, 2000,

"<!DOCTYPE html>\
<html>\
<head>\
  <script type=\"text/javascript\" src=\"https://www.gstatic.com/charts/loader.js\"></script>\
  <div id=\"chart_div\">\
  </div>\
</head>\
<body>\
  <script>\
google.charts.load('current', {packages: ['corechart', 'line']});\
google.charts.setOnLoadCallback(drawBackgroundColor);\
function drawBackgroundColor() {\
      var data = new google.visualization.DataTable();\
      data.addColumn('number', 'X');\
      data.addColumn('number', 'Temp');\
      data.addRows([\
        [0, %3f],   [1, %3f],  [2, %3f],  [3, %3f],  [4, %3f], [5, %3f],\
        [6, %3f],   [7, %3f],  [8, %3f],  [9, %3f],  [10, %3f], [11, %3f],\
        [12, %3f],   [13, %3f],  [14, %3f],  [15, %3f],  [16, %3f], [17, %3f],\
        [18, %3f],   [19, %3f],  [20, %3f],  [21, %3f],  [22, %3f], [23, %3f]\
      ]);\
      var options = {\
        hAxis: {\
          title: 'Time'\
        },\
        vAxis: {\
          title: 'Celsius',\
          ticks: [25,30,35]\
        },\
        backgroundColor: '#f1f8e9'\
      };\
      var chart = new google.visualization.LineChart(document.getElementById('chart_div'));\
      chart.draw(data, options);\
    }\
    </script>\
</body>\
</html>",
           temp_data_array[0], temp_data_array[1], temp_data_array[2], temp_data_array[3], temp_data_array[4], temp_data_array[5],
           temp_data_array[6], temp_data_array[7], temp_data_array[8], temp_data_array[9], temp_data_array[10], temp_data_array[11],
           temp_data_array[12], temp_data_array[13], temp_data_array[14], temp_data_array[15], temp_data_array[16], temp_data_array[17],
           temp_data_array[18], temp_data_array[19], temp_data_array[20], temp_data_array[21], temp_data_array[22], temp_data_array[23]
          );
  server.send(200, "text/html", temp);
  digitalWrite(led, 0);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup(void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  
  Serial.begin(115200);
  dht.begin();
  for (int i = 0; i <= 23; i++) {
    temp_data_array[i] = 25.0;
  }
  delay(3000);
  float t = dht.readTemperature();
  temp_data_array[0] = t;
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  unsigned long curr_time = millis();
  if ((curr_time - prev_time) >= period) {
    float t = dht.readTemperature();
    for (int i = 22; i >= 0; i--) {
      temp_data_array[i+1] = temp_data_array[i];
    }
    temp_data_array[0] = t;
    prev_time = curr_time;
  }
  server.handleClient();
  MDNS.update();
}
