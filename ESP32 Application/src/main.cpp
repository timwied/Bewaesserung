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
int PumpTime = 5000;
int PumpStartTime = 0;
int PumpBreakTime = 10000;
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
  String json = String("{\"waterLevel\":") + WaterLevel +
                String(",\"humidity\":") + Moisture +
                String(",\"pumpStartTime\":") + PumpStartTime +
                String(",\"activatePump\":") + ActivatePump +
                String(",\"millis\":") + millis() +
                String("}");
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

  server->begin();
}

void setPins()
{
  pinMode(PinMoisture, INPUT);
  pinMode(PinWater, INPUT);
  pinMode(PinRelay, OUTPUT);
}

void readWaterLevel()
{
  WaterLevel = analogRead(PinWater);
  if (WaterLevel < 200)
    NoWater = true;
  else
    NoWater = false;
}

void readMoisture()
{
  Moisture = analogRead(PinMoisture);
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

  if (Moisture < 2000 && millis() - PumpStartTime > PumpBreakTime)
  {
    PumpStartTime = millis();
    digitalWrite(PinRelay, HIGH);
    ActivatePump = true;
  }
  else if (millis() - PumpStartTime > PumpTime)
  {
    digitalWrite(PinRelay, LOW);
    ActivatePump = false;
  }
}