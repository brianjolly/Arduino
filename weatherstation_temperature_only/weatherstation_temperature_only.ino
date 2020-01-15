#include "DHTesp.h"
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

WiFiUDP ntpUDP;
// NTPClient timeClient(ntpUDP);
NTPClient timeClient(ntpUDP, "0.arch.pool.ntp.org", 28800, 6000);

DHTesp dht;

String unixTime;

const char* ssid     = "********";
const char* password = "********";
const char* host = "";

void setup() {
  Serial.begin(115200);

  dht.setup(D6, DHTesp::DHT11);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);

  // Wait until WiFi is connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(28800); // seconds
  timeClient.setUpdateInterval(60000); // milliseconds
}

void loop() {
  delay(5000);

  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }

  unixTime = timeClient.getEpochTime();

  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();

  // float vccVolt = ((float)ESP.getVcc())/1024;
  int vcc = ESP.getVcc();
  Serial.print("VCC: ");
  Serial.println(vcc);

  Serial.printf("Hum: %.1f%%, Temp: %.1f°F\n", humidity, dht.toFahrenheit(temperature));

  String weather = String(dht.toFahrenheit(temperature)) + "°F\n" + String(humidity) + "%h";
  String weatherJSON = "{ \"temp_fahrenheit\":\"" + String(dht.toFahrenheit(temperature)) + "\", " +
  "\"temp_celsius\": \"" + temperature + "\", " +
  "\"humidity\": \"" + String(humidity) + "\", " +
  "\"sample_time\": \"" + String(unixTime) + "\", " +
  "\"vcc\": \"" + String(vcc) + "\", " +
  "\"location\": \"bedroom 1\"" + 
  " }";

  Serial.print("connecting to ");
  Serial.println(host);

  const char* fingerprint = "D9 35 14 26 D9 FC EA F3 20 9D 10 68 B7 0F 9E 60 FE EE 6F 16";

  // https uses port 443
  //WiFiClientSecure client;
  WiFiClient client;
  const int httpPort = 9000;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // Add here the certificate check
  /*
  if (client.verify(fingerprint, host)) {
    Serial.println("certificate matches");
  } else {
    Serial.println("certificate doesn't match");
    return;
  }
  */

  // We now create a URI for the request
  //String url = "/weather?a=1&b=2";
  String url = "/weather/_doc/";
  
  Serial.print("Requesting: ");
  Serial.println(url);

//               "Content-Type: application/json" + "\r\n" +

  String req = String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Content-Type: application/json" + "\r\n" +
               "Content-Length: " + weatherJSON.length() + "\r\n" +
               "Authorization: Basic ZWxhc3RpYzpidXN0ZXI3Mgo=\r\n" +
               "Connection: close\r\n" +
               "\r\n" +
               weatherJSON + "\r\n\r\n";

  Serial.println(req);
  
  // This will send the request to the server
  client.print(req);
               
              // "Connection: close\r\n\r\n");
/*
  client.println("POST /weather HTTP/1.1");
  client.println("Host: jsonplaceholder.typicode.com");
  client.println("Cache-Control: no-cache");
  client.println("Content-Type: application/x-www-form-urlencoded");
  client.print("Content-Length: ");
  client.println(PostData.length());
  client.println();
  client.println(PostData);
*/

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  Serial.println("Response:");
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
}
