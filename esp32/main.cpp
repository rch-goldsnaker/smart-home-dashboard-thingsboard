#include <WiFi.h>
#include <Arduino_MQTT_Client.h>
#include <ThingsBoard.h>
#include <ArduinoOTA.h>
#include <LiquidCrystal_I2C.h>

#define I2C_ADDR    0x27
#define LCD_COLUMNS 20
#define LCD_LINES   4
LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLUMNS, LCD_LINES);

// Configuración de WiFi y ThingsBoard
constexpr char WIFI_SSID[] = "Wokwi-GUEST";
constexpr char WIFI_PASSWORD[] = "";
constexpr char TOKEN[] = "7hVoTzIu2CfPV3hmM7m9";
constexpr char THINGSBOARD_SERVER[] = "thingsboard.cloud";
constexpr uint16_t THINGSBOARD_PORT = 1883U;
constexpr uint32_t MAX_MESSAGE_SIZE = 1024U;
constexpr uint32_t SERIAL_DEBUG_BAUD = 115200U;

// Definiciones y variables
constexpr int LED_PIN1 = 25;
constexpr int LED_PIN2 = 33;
constexpr int LED_PIN3 = 32;
constexpr int LED_PIN4 = 26;
constexpr int LED_PIN5 = 27;

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

// Inicialización de la conexión WiFi
void InitWiFi() {
  Serial.println("Connecting to AP ...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}

// Reintentar conexión WiFi
bool reconnect() {
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }
  InitWiFi();
  return true;
}

// Procesar cambios de switch
RPC_Response processSwitchChange(const RPC_Data &data) {
  Serial.println(F("Received the set switch method"));
  deviceValue5 = data["switch"];
  Serial.print(F("value change: "));
  Serial.println(deviceValue5);
  attributesChanged = true;

  StaticJsonDocument<JSON_OBJECT_SIZE(1)> doc;
  doc["response"] = 1;
  return RPC_Response(doc);
}

const std::array<RPC_Callback, 1U> callbacks = {
  RPC_Callback{ "set_switch", processSwitchChange }
};

// Procesar atributos compartidos
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

// Procesar atributos del cliente
void processClientAttributes(const Shared_Attribute_Data &data) {
  for (auto it = data.begin(); it != data.end(); ++it) {
    if (strcmp(it->key().c_str(), "deviceValue5") == 0) {
      deviceValue5 = it->value().as<uint16_t>();
    }
  }
}

const Shared_Attribute_Callback attributes_callback(&processSharedAttributes, SHARED_ATTRIBUTES_LIST.cbegin(), SHARED_ATTRIBUTES_LIST.cend());
const Attribute_Request_Callback attribute_shared_request_callback(&processSharedAttributes, SHARED_ATTRIBUTES_LIST.cbegin(), SHARED_ATTRIBUTES_LIST.cend());
const Attribute_Request_Callback attribute_client_request_callback(&processClientAttributes, CLIENT_ATTRIBUTES_LIST.cbegin(), CLIENT_ATTRIBUTES_LIST.cend());

void setup() {
  // Init
  lcd.init();
  lcd.backlight();
  // Print something
  lcd.setCursor(0, 0);
  lcd.print("Air Conditioner");

  Serial.begin(SERIAL_DEBUG_BAUD);
  pinMode(LED_PIN1, OUTPUT);
  pinMode(LED_PIN2, OUTPUT);
  pinMode(LED_PIN3, OUTPUT);
  pinMode(LED_PIN4, OUTPUT);
  pinMode(LED_PIN5, OUTPUT);
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
    Serial.print("Saving attribute");
    Serial.println(deviceValue5);
    tb.sendAttributeData("deviceValue5", deviceValue5);
  }

  lcd.setCursor(0, 1);
  lcd.print(deviceValue5);

  tb.loop();
}
