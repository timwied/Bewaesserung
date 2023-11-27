#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>

void setup() {
  Serial.begin(9600);

  // begin WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin("RvWBK-BYOD", "IchGeheGerneZurSchule");

  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("WiFi Failed!\n");
    return;
  }

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  ////////////// WEBSERVER ////////////////
  AsyncWebServer *server = new AsyncWebServer(80);

  // definieren von handler von get requests
  server->on("/data", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", "<h1>CustomTest</h1>");
  });

  server->on(
      "/data", HTTP_POST,
      [](AsyncWebServerRequest *request) {
        request->send(400, "text/html", "<h1>Body missing</h1>");
      },
      NULL,
      [](AsyncWebServerRequest *request, uint8_t *data, size_t len,
         size_t index, size_t total) {
        if (len != total) {
          request->send(400, "text/html", "<h1>Paginagion kickt</h1>");
          return;
        }
        // processing of stuff
        request->send(200, "text/html", "<h1>Processed</h1>");
      });

  // Json post handler
  AsyncCallbackJsonWebHandler *handler = new AsyncCallbackJsonWebHandler(
      "/data2", [](AsyncWebServerRequest *request, JsonVariant &json) {
        if (json["username"].is<String>()) {
          String x = json["username"];
          request->send(200, "text/html", "du heißt " + x);
        } 
        else {
          // also respond with json
          DynamicJsonDocument doc(1024);
          
          doc["sensor"] = "gps";
          doc["time"] = 1351824120;
          doc["data"][0] = 48.756080;
          doc["data"][1] = 2.302038;

          String x;
          serializeJson(doc, x);
          request->send(400, "application/json", x);
        }
      });

  server->addHandler(handler);
  server->begin();
}

void loop() {
}