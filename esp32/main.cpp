#include <WiFi.h>
#include <Arduino_MQTT_Client.h>
#include <ThingsBoard.h>
#include <ArduinoOTA.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"

// LCD
#define I2C_ADDR    0x27
#define LCD_COLUMNS 20
#define LCD_LINES   4
LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLUMNS, LCD_LINES);

// DHT SENSOR
#define DHTTYPE DHT22 
#define DHT_PIN 15
DHT dht(DHT_PIN, DHTTYPE);

// LED Pins
constexpr int LED_PIN1 = 25;
constexpr int LED_PIN2 = 33;
constexpr int LED_PIN3 = 32;
constexpr int LED_PIN4 = 26;
constexpr int LED_PIN5 = 27;

// WiFi and ThingsBoard Configuration
constexpr char WIFI_SSID[] = "Wokwi-GUEST";
constexpr char WIFI_PASSWORD[] = "";
constexpr char TOKEN[] = "8AIEOQRywKJ8NLTDdgcq";
constexpr char THINGSBOARD_SERVER[] = "thingsboard.cloud";
constexpr uint16_t THINGSBOARD_PORT = 1883U;
constexpr uint32_t MAX_MESSAGE_SIZE = 1024U;
constexpr uint32_t SERIAL_DEBUG_BAUD = 115200U;

// Variables
WiFiClient wifiClient;
Arduino_MQTT_Client mqttClient(wifiClient);
ThingsBoard tb(mqttClient, MAX_MESSAGE_SIZE);
volatile bool attributesChanged = false;
volatile bool deviceState1 = false, deviceState2 = false, deviceState3 = false, deviceState4 = false, deviceState5 = false;
volatile int deviceValue5 = 0;
uint32_t previousStateChange;
constexpr int16_t telemetrySendInterval = 2000U;
uint32_t previousDataSend;

constexpr std::array<const char *, 6U> SHARED_ATTRIBUTES_LIST = {
  "deviceState1", "deviceState2", "deviceState3", "deviceState4", "deviceState5"
};

constexpr std::array<const char *, 1U> CLIENT_ATTRIBUTES_LIST = {
  "deviceValue5"
};

// Function to initialize WiFi
void InitWiFi() {
  Serial.println("Connecting to AP ...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}

// Function to reconnect WiFi
bool reconnect() {
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }
  InitWiFi();
  return true;
}

// Callback function for processing switch changes
RPC_Response processSwitchChange(const RPC_Data &data) {
  Serial.println(F("Received the set switch method"));
  deviceValue5 = data["switch"];
  Serial.print(F("Value change: "));
  Serial.println(deviceValue5);
  attributesChanged = true;

  StaticJsonDocument<JSON_OBJECT_SIZE(1)> doc;
  doc["response"] = 1;
  return RPC_Response(doc);
}

// Array of RPC callbacks
const std::array<RPC_Callback, 1U> callbacks = {
  RPC_Callback{ "set_switch", processSwitchChange }
};

// Callback function for processing shared attributes
void processSharedAttributes(const Shared_Attribute_Data &data) {
  for (auto it = data.begin(); it != data.end(); ++it) {
    const char* key = it->key().c_str();
    bool value = it->value().as<bool>();

    if (strcmp(key, "deviceState1") == 0) {
      deviceState1 = value;
      digitalWrite(LED_PIN1, deviceState1);
      Serial.print("deviceState1 is set to: ");
      Serial.println(deviceState1);
    } else if (strcmp(key, "deviceState2") == 0) {
      deviceState2 = value;
      digitalWrite(LED_PIN2, deviceState2);
      Serial.print("deviceState2 is set to: ");
      Serial.println(deviceState2);
    } else if (strcmp(key, "deviceState3") == 0) {
      deviceState3 = value;
      digitalWrite(LED_PIN3, deviceState3);
      Serial.print("deviceState3 is set to: ");
      Serial.println(deviceState3);
    } else if (strcmp(key, "deviceState4") == 0) {
      deviceState4 = value;
      digitalWrite(LED_PIN4, deviceState4);
      Serial.print("deviceState4 is set to: ");
      Serial.println(deviceState4);
    } else if (strcmp(key, "deviceState5") == 0) {
      deviceState5 = value;
      digitalWrite(LED_PIN5, deviceState5);
      Serial.print("deviceState5 is set to: ");
      Serial.println(deviceState5);
    }
  }
}

// Callback function for processing client attributes
void processClientAttributes(const Shared_Attribute_Data &data) {
  for (auto it = data.begin(); it != data.end(); ++it) {
    if (strcmp(it->key().c_str(), "deviceValue5") == 0) {
      deviceValue5 = it->value().as<uint16_t>();
    }
  }
}

// Define attribute callbacks
const Shared_Attribute_Callback attributes_callback(&processSharedAttributes, SHARED_ATTRIBUTES_LIST.cbegin(), SHARED_ATTRIBUTES_LIST.cend());
const Attribute_Request_Callback attribute_shared_request_callback(&processSharedAttributes, SHARED_ATTRIBUTES_LIST.cbegin(), SHARED_ATTRIBUTES_LIST.cend());
const Attribute_Request_Callback attribute_client_request_callback(&processClientAttributes, CLIENT_ATTRIBUTES_LIST.cbegin(), CLIENT_ATTRIBUTES_LIST.cend());

void setup() {
  // Initialize Serial and LCD
  Serial.begin(SERIAL_DEBUG_BAUD);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Air Conditioner");

  // Initialize DHT sensor
  dht.begin();

  // Initialize LED pins
  pinMode(LED_PIN1, OUTPUT);
  pinMode(LED_PIN2, OUTPUT);
  pinMode(LED_PIN3, OUTPUT);
  pinMode(LED_PIN4, OUTPUT);
  pinMode(LED_PIN5, OUTPUT);

  // Initialize WiFi
  delay(1000);
  InitWiFi();
}

void loop() {
  delay(10);

  if (!reconnect()) {
    return;
  }

  if (!tb.connected()) {
    Serial.print("Connecting to: ");
    Serial.print(THINGSBOARD_SERVER);
    Serial.print(" with token ");
    Serial.println(TOKEN);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN, THINGSBOARD_PORT)) {
      Serial.println("Failed to connect");
      return;
    }
    tb.sendAttributeData("macAddress", WiFi.macAddress().c_str());
    Serial.println("Subscribing for RPC...");
    if (!tb.RPC_Subscribe(callbacks.cbegin(), callbacks.cend())) {
      Serial.println("Failed to subscribe for RPC");
      return;
    }
    if (!tb.Shared_Attributes_Subscribe(attributes_callback)) {
      Serial.println("Failed to subscribe for shared attribute updates");
      return;
    }
    Serial.println("Subscribe done");
    if (!tb.Shared_Attributes_Request(attribute_shared_request_callback)) {
      Serial.println("Failed to request for shared attributes");
      return;
    }
    if (!tb.Client_Attributes_Request(attribute_client_request_callback)) {
      Serial.println("Failed to request for client attributes");
      return;
    }
  }

  if (attributesChanged) {
    attributesChanged = false;
    Serial.print("Saving attribute: ");
    Serial.println(deviceValue5);
    tb.sendAttributeData("deviceValue5", deviceValue5);
  }

  lcd.setCursor(0, 1);
  lcd.print(deviceValue5);

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  char temperatureString[6];
  dtostrf(t, 4, 1, temperatureString);

  tb.sendTelemetryData("tempIndoor", temperatureString);
  tb.sendTelemetryData("humidity", h);
  tb.sendTelemetryData("deviceValue5", deviceValue5);
  
  tb.sendTelemetryData("tempOutdoor", random(10, 20));
  tb.sendTelemetryData("energy", random(10, 20));

  tb.sendAttributeData("rssi", WiFi.RSSI());
  tb.sendAttributeData("channel", WiFi.channel());
  tb.sendAttributeData("bssid", WiFi.BSSIDstr().c_str());
  tb.sendAttributeData("localIp", WiFi.localIP().toString().c_str());
  tb.sendAttributeData("ssid", WiFi.SSID().c_str());

  Serial.print("Temp ");
  Serial.print(t);
  Serial.println("°C");
  Serial.print("Humidity ");
  Serial.print(h);
  Serial.println("%");
  Serial.println("---");
  delay(2000);

  tb.loop();
}
