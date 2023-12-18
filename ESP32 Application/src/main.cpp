#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <SPIFFS.h>

int PinMoisture = 34;
int PinWater = 35;
int PinRelay = 5;

bool ActivatePump = false;
bool NoWater = false;

int Moisture = 0;
int WaterLevel = 0;

void HandleRoot(AsyncWebServerRequest* request)
{
  Serial.printf("Entering HandleRoot\n");
  request->send(SPIFFS, "/index.html");
}

void HandleGetData(AsyncWebServerRequest* request)
{
  String json = String("{\"waterLevel\":") + WaterLevel + String(",\"humidity\":") + Moisture + String("}");
  request->send(200, "application/json", json);
}

void startServer()
{
  // begin WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin("RvWBK-BYOD", "IchGeheGerneZurSchule");

  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.printf("WiFi Failed!\n");
    return;
  }

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  ////////////// WEBSERVER ////////////////
  AsyncWebServer *server = new AsyncWebServer(80);

  // definieren von handler von get requests
  server->on("/", HTTP_GET, HandleRoot);
  server->on("/getData", HTTP_GET, HandleGetData);

  server->on(
      "/data", HTTP_POST,
      [](AsyncWebServerRequest *request)
      {
        request->send(400, "text/html", "<h1>Body missing</h1>");
      },
      NULL,
      [](AsyncWebServerRequest *request, uint8_t *data, size_t len,
         size_t index, size_t total)
      {
        if (len != total)
        {
          request->send(400, "text/html", "<h1>Paginagion kickt</h1>");
          return;
        }
        // processing of stuff
        request->send(200, "text/html", "<h1>Processed</h1>");
      });

  // Json post handler
  AsyncCallbackJsonWebHandler *handler = new AsyncCallbackJsonWebHandler(
      "/data2", [](AsyncWebServerRequest *request, JsonVariant &json)
      {
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
        } });

  server->addHandler(handler);
  server->begin();
}

void setPins()
{
  pinMode(PinMoisture, INPUT);
  pinMode(PinWater, INPUT);
  pinMode(PinRelay, OUTPUT);
}

void pumpWater(int milliSeconds)
{
  Serial.printf("Pumping water for %d milliseconds\n", milliSeconds);
  digitalWrite(PinRelay, HIGH);
  delay(milliSeconds);
  digitalWrite(PinRelay, LOW);
}

void readWaterLevel()
{
  Serial.print("Entering readWaterLevel()\n");
  WaterLevel = analogRead(PinWater);
  Serial.printf("Waterlevel reading: %d\n", WaterLevel);
  if (WaterLevel < 200)
    NoWater = true;
  else
    NoWater = false;
  Serial.printf("NoWater=%s\n", NoWater ? "True" : "False");
}

void readMoisture()
{
  Serial.print("Entering readMoisture()\n");
  Moisture = analogRead(PinMoisture);
  Serial.printf("Moisture reading: %d\n", Moisture);
  if (Moisture < 2000)
    pumpWater(1000);
}

//--------------------------------------------------------------------
//--------------------------------------------------------------------

void setup()
{
  Serial.begin(9600);

  setPins();

  SPIFFS.begin();
  startServer();
}

void loop()
{
  readWaterLevel();
  readMoisture();
  // userInput()
  delay(2000);
}