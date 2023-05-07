int setsec = 30; 
int count = 0;
#include <AntoIO.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#define LINE_TOKEN "EX2DQ60RUCB3qMzRYVELHC6FuZqVp7YeiCVbVav5b2b"

const char *ssid = "Edok5555";
const char *pass = "0910395891";
const char *user = "nattalesrunner";
const char *token = "IUFaCcA9WUMJHIr2q51CSELotKUgLcNKdmIZ126e";
const char *thing = "myserver";

/* create AntoIO instance */
AntoIO anto(user, token, thing);


#include "DHT.h"

#define DHTPIN 27

#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);
void setup() {

  Serial.begin(9600);
  Serial.print("\nTrying to connect ");
  Serial.print(ssid);
  Serial.print("\nTrying to connect ");
  Serial.print(ssid);
  Serial.println("...");

  while (!anto.wifi.begin(ssid, pass));
  Serial.println("\nConnected, trying to connect to broker...");

  while (!anto.mqtt.connect(user, token, true));
  Serial.println("\nConnected");
  LINE_Notify("ออนไลน์");
  delay(1000);
  dht.begin();

}

void loop() {
  anto.mqtt.loop();
  // Wait a few seconds between measurements.
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));
  anto.mqtt.pub("temp", t);
  anto.mqtt.pub("humid", h);
  count += 1;
  Serial.println("                                count = " + String(count));
  delay(1000);

  if ( count == setsec )
  {
    LINE_Notify("อุณหภูมิ:" + String(t, 1) + " องศา"  );
    Serial.println("    Temp Line !!!");
    delay(1000);
    LINE_Notify("ความชื้นในอากาศ:" + String(h, 1) + " เปอร์เซ็นต์ "  );
    Serial.println("    Humid Line !!!");
    delay(1000);

    count = 0;
  }


}
void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  Serial.print("incoming: ");
  Serial.print(topic);
  Serial.print(" - ");
  Serial.print(payload);
  Serial.println();

}

bool LINE_Notify(String message) {
  WiFiClientSecure client;

  if (!client.connect("notify-api.line.me", 443)) {
    Serial.println("connection failed");
    return false;
  }

  String payload = "message=" + message;
  String req = "";
  req += "POST /api/notify HTTP/1.1\r\n";
  req += "Host: notify-api.line.me\r\n";
  req += "Authorization: Bearer " + String(LINE_TOKEN) + "\r\n";
  req += "User-Agent: ESP32\r\n";
  req += "Content-Type: application/x-www-form-urlencoded\r\n";
  req += "Content-Length: " + String(payload.length()) + "\r\n";
  req += "\r\n";
  req += payload;
  client.print(req);

  delay(20);
  
  long timeOut = millis() + 1000; // 30000
  Serial.println("                                                   timeOut =  " + String(timeOut));
  while (client.connected() && timeOut > millis()) {
    if (client.available()) {
      String str = client.readString();
      Serial.print(str);
    }
    delay(10);
  }
  return timeOut > millis();
}
