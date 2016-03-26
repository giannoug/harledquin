#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <Adafruit_NeoPixel.h>

#define AP_MODE 1

#if AP_MODE

const char* ssid     = "HarLEDquin";
const char* password = "yoloswag";

IPAddress apIP(192, 168, 1, 1);

#else

ESP8266WiFiMulti wifiMulti;
const char* ssid     = "YOUR_SSID_GOES_HERE";
const char* password = "YOUR_PASS_GOES_HERE";

#endif

#define LED_DATA_PIN    12 // 12
#define LED_NUM         8

ESP8266WebServer server(80);
WebSocketsServer webSocket(81);

Adafruit_NeoPixel pixels(LED_NUM, LED_DATA_PIN, NEO_RGB + NEO_KHZ800);

// Initial JSON array
#define INITIAL_LEN     72
char initial[] = "[0,0,0,0,0,0,0,0]";

// Buffer array where data is copied
#define BUFFER_LEN      128
char json[BUFFER_LEN];

/**
   Returns the HTTP content type header, adapted from an ESP8266 example
*/
String getContentType(String filename) {
  if (server.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

/**
   Requested file handler, adapted from an ESP8266 example
*/
bool handleFileRead(String path) {
  Serial.println("[handleFileRead] Requested: " + path);

  if (path.endsWith("/")) {
    path += "index.html";
  }

  String contentType = getContentType(path);

  // If a file with .gz exists, serve that instead
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz)) {
      path += ".gz";
    }

    File file = SPIFFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();

    return true;
  }

  return false;
}

/**
   Websocket event handler
*/
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {

  switch (type) {
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        webSocket.sendTXT(num, json);
      }
      break;
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_TEXT:
      Serial.printf("[%u] Got text: %s\n", num, payload);

      // Parse JSON and light LEDs
      StaticJsonBuffer<500> jsonBuffer;
      Serial.printf("[%u] Erasing json var\n", num);
      
      //memcpy(json, 0, BUFFER_LEN);
      memset(json, 0, sizeof(char)*BUFFER_LEN);
      Serial.printf("[%u] Copying payload to json var\n", num);
      memcpy(json, payload, lenght);

      // Local buffer
      char buff[lenght];
      memcpy(buff, payload, lenght);

      JsonArray& root = jsonBuffer.parseArray(buff);

      if (root.success())
      {
        for (int i = 0; i <= 7; i++) {
          Serial.printf("[%u] Led %d was set to %s\n", num, i, root[i].asString());
          pixels.setPixelColor(i, root[i]);
        }
        pixels.show();

        // Send the payload to everyone connected
        webSocket.broadcastTXT(payload);
      }
      else {
        Serial.printf("[%u] JSON parsing failed\n", num);
      }

      break;
  }

}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  Serial.println("Sup?");

  SPIFFS.begin();
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      String fileSize = String(dir.fileSize());
      Serial.printf("[SETUP] FS file: %s, size (bytes): %s\n", fileName.c_str(), fileSize.c_str());
    }
  }

  memcpy(json, initial, INITIAL_LEN);

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

#if AP_MODE

  Serial.printf("Setting up AP %s\n", ssid);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid, password);

  Serial.println("IP address: ");
  Serial.println(WiFi.softAPIP());

#else

  Serial.print("Connecting to ");
  Serial.println(ssid);

  wifiMulti.addAP(ssid, password);

  while (wifiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

#endif

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  pixels.begin();
  pixels.show();

  server.onNotFound([]() {
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound");
  });

  server.begin();

  Serial.println("[SETUP] Done...\n");
}

void loop() {
  server.handleClient();
  webSocket.loop();
}
