#include <Arduino.h>
#include <WiFiClientSecure.h>

/*
  Example from WiFi > WiFiScan
  Complete details at https://RandomNerdTutorials.com/esp32-useful-wi-fi-functions-arduino/
*/


const char* host = "api.pushbullet.com";
const int httpsPort = 443;
const char* accessToken = "o.DCVgtxouN2DoW0D3jtOjEYSvZzX6KaI1";

const char* ssid     = "Cloyes_Home";
const char* password = "5A69A4FF96D69736CEADF539A6";

void setup() {
  Serial.begin(9600);
  delay(100);

/**************
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.println("Setup done");
****************/
// We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

	Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  Serial.print("Connected to host: ");
  Serial.println(host);
}

void loop() {

/**************
  Serial.println("scan start");
  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
      Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
      delay(10);
    }
  }
  Serial.println("");

  // Wait a bit before scanning again
  delay(5000);

***************/
}