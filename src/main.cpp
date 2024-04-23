#include <WiFi.h>
#include <ThingsBoard.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DFRobot_SHT20.h"

#define WIFI_AP "Siloo"
#define WIFI_PASS "Siloo1399"
#define TB_SERVER "192.168.0.103"
#define TOKEN "YA0o66prdG7QIOobfH8x"
constexpr uint16_t MAX_MESSAGE_SIZE = 128U;
WiFiClient wifiClient;
IPAddress server(192, 168, 0, 103);
ThingsBoard tb(wifiClient, MAX_MESSAGE_SIZE);

// DS18B20
#define ONE_WIRE_BUS 6
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

DFRobot_SHT20 sht20(&Wire, SHT20_I2C_ADDR);

void connectToWiFi()
{
  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED && attempts < 20)
  {
    WiFi.begin(WIFI_AP, WIFI_PASS);
    delay(1000);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("\nFailed to connect to WiFi.");
  }
  else
  {
    Serial.println("\n... Online ... ");
  }
}

int ledState = 0;
RPC_Response setLedState(RPC_Data &data)
{
  ledState = data;
  Serial.print("*** LED STATE SWITCHED TO : ");
  Serial.print(ledState);
  Serial.println(" ***");
  return RPC_Response("setLedSwitchValue", ledState);
}

const std::array<RPC_Callback, 1U> cb = {
    RPC_Callback("setLedSwitchValue", setLedState)};

void connectToThingsBoard()
{
  if (!tb.connected())
  {
    Serial.println("Connecting to ThingsBoard server");

    if (!tb.connect(TB_SERVER, TOKEN))
    {
      Serial.println("Failed to connect to ThingsBoard");
    }
    else
    {
      Serial.println("Connected to ThingsBoard");
      if (!tb.RPC_Subscribe(cb.cbegin(), cb.cend()))
      {
        Serial.println("Faild to sub PRC :()");
        return;
      }
    }
  }
}

void sendDataToThingsBoard(float ds18b20, float temp, int hum)
{
  String jsonData = "{\"ds18b20\":" + String(ds18b20) + ", \"temperature\":" + String(temp) + ", \"humidity\":" + String(hum) + "}";
  tb.sendTelemetryJson(jsonData.c_str());
  Serial.println("Data sent");
}

void setup()
{
  Serial.begin(115200);
  sensors.begin();
  sht20.initSHT20();
  sht20.checkSHT20();
  connectToWiFi();
  pinMode(42, OUTPUT);
  connectToThingsBoard();
}

void loop()
{
  sensors.requestTemperatures();
  connectToWiFi();

  float tempDS18b20 = sensors.getTempCByIndex(0);
  float tempSHT20 = sht20.readTemperature();
  int humSHT20 = sht20.readHumidity();

  Serial.println(tempDS18b20);

  if (!tb.connected())
  {
    connectToThingsBoard();
  }

  // sendDataToThingsBoard(tempDS18b20, tempSHT20, humSHT20);

  // delay(3000);

  if (ledState == 1)
    digitalWrite(42, HIGH);
  else
    digitalWrite(42, LOW);

  tb.loop();
}