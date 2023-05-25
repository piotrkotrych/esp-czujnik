#define DHTPIN 4 // DHT22 data pin
#define DHTTYPE DHT22 // DHT22 sensor type

#include <ESP8266WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>

DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "EltwinDUC";
const char* password = "Nhwjqlaqvaqdaq12!";
const unsigned long WIFI_CONNECT_TIMEOUT = 5000; // 5 seconds
const char* postUrl = "https://www.so718.sohost.pl/sensor/api.php";
const char* email = "test@test.pl";
const char* name = "test";
const char* lokacja = "lokacja";
const int dsTime = 5;
float temperature = 0;
float humidity = 0;

const float batteryVoltageMin = 2.8;
const float batteryVoltageMax = 4.2;
const int batteryPin = A0;

// Calibration constants
const float batteryVoltageCalibrationOffset = 0.0;
const float batteryVoltageCalibrationFactor = 1.0;

void setup() {
  Serial.begin(9600);

  dht.begin();
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  
  Serial.print("Connecting to Wi-Fi");
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < WIFI_CONNECT_TIMEOUT) {
    delay(500);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    // Wi-Fi connected
    Serial.println();
    Serial.print("Connected to Wi-Fi. IP address: ");
    Serial.println(WiFi.localIP());

    temperature = dht.readTemperature();
    humidity = dht.readHumidity();

    // Read battery voltage and percentage
    float batteryVoltage = readBatteryVoltage();
    float batteryPercentage = calculateBatteryPercentage(batteryVoltage);

    // Apply calibration values
    batteryVoltage += batteryVoltageCalibrationOffset;
    batteryVoltage *= batteryVoltageCalibrationFactor;

    //send data to server using sendPostRequest and createJsonString functions
    String jsonString = createJsonString(batteryVoltage, batteryPercentage);
    int httpResponseCode = sendPostRequest(postUrl, jsonString.c_str());
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    ESP.deepSleep(60000000 * dsTime); // 1 minute
  } else {
    // Connection failed within timeout
    Serial.println();
    Serial.println("Failed to connect to Wi-Fi within the specified timeout.");
    ESP.deepSleep(60000000 * dsTime); // 1 minute
  }
}

void loop() {
  // Your code here
}

String createJsonString(float batteryVoltage, float batteryPercentage) {
  StaticJsonDocument<200> jsonDocument;
  
  jsonDocument["chipid"] = ESP.getChipId();
  jsonDocument["email"] = email;
  jsonDocument["name"] = name;
  jsonDocument["lokacja"] = lokacja;
  jsonDocument["temperature"] = temperature;
  jsonDocument["humidity"] = humidity;
  jsonDocument["batteryVoltage"] = batteryVoltage;
  jsonDocument["batteryPercentage"] = batteryPercentage;
  
  String jsonString;
  serializeJson(jsonDocument, jsonString);
  
  return jsonString;
}

// Function to send the POST request
int sendPostRequest(const char* url, const char* payload) {
  WiFiClientSecure client;

  client.setInsecure();

  HTTPClient http;
  http.begin(client, url);

  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(payload);
  Serial.println(httpResponseCode);

  return httpResponseCode;

}

// Function to read battery voltage
float readBatteryVoltage() {
  int rawValue = analogRead(batteryPin);
  float voltage = (rawValue / 1023.0) * 3.3; // Assuming ESP8266 operates at 3.3V

  return voltage;
}

// Function to calculate battery percentage
float calculateBatteryPercentage(float voltage) {
  float percentage = ((voltage - batteryVoltageMin) / (batteryVoltageMax - batteryVoltageMin)) * 100;
  percentage = constrain(percentage, 0, 100); // Ensure the percentage is within the valid range

  return percentage;
}
