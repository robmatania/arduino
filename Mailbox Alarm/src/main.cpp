#include <Arduino.h>
#include <WiFiClientSecure.h>

#define PRINTF_BUF 50
#define StartWordCount 32


const char* host = "api.pushbullet.com";
const int httpsPort = 443;
const char* accessToken = "o.DCVgtxouN2DoW0D3jtOjEYSvZzX6KaI1";

const char* ssid     = "Cloyes_Home";
const char* password = "5A69A4FF96D69736CEADF539A6";

void pushNotification(const char *MessageBody, const char *MessageTitle);

void setup() {
  Serial.begin(9600);
  delay(100);

// Connect to WiFi network

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


  pushNotification("You have MAIL", " ");
}

void loop() {
//int s = WiFi.status();
//Serial.println(s);
delay(1000);

} // Loop

//============================================================================================
void pushNotification( const char *MessageBody, const char *MessageTitle) {

String xx = "toto" + String(MessageBody);

// Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }

  String url = "/v2/pushes";
  //String messagebody = "{\"email\": \"matania.robert@gmail.com\",\"type\": \"note\", \"title\": \""+ String(MessageTitle)+"\", \"body\": \""+String(MessageBody)+"\"}\r\n";
  String messagebody = "{\"type\": \"note\", \"title\": \""+ String(MessageTitle)+"\", \"body\": \""+String(MessageBody)+"\"}\r\n";

  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" +
                 "Access-Token: " + accessToken + "\r\n" +
                 "Content-Type: application/json\r\n" +
                 "Content-Length: " +
                 String(messagebody.length()) + "\r\n\r\n");
                 

  client.print(messagebody);

  //print the response

  while (client.available() == 0);

  while (client.available()) {
    String line = client.readStringUntil('\n');
    Serial.println(line);
  }

  client.stop();
}
