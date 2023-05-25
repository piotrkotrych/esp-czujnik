#define LED_BUILTIN 2 // Default led
#define DHTPIN 4 // DHT22 data pin
#define DHTTYPE DHT22 // DHT22 sensor type

#include <FS.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <stdlib.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <Ticker.h>
Ticker ticker;

DHT dht(DHTPIN, DHTTYPE);

bool shouldSaveConfig = false;

//client setup for backend server
char postUrl[100] = "https://www.so718.sohost.pl/sensor/api.php";
char email[40] = "test@test.pl";
char name[40] = "test";
char lokacja[40] = "lokacja";
char dsTime[2] = "1";

//reset button pin D1
const int BUTTON_PIN = 5;

//battery percentage setup
const float batteryPercent = 0.0;
const float batteryVoltage = 0.0;
const float batteryVoltageMin = 2.8;
const float batteryVoltageMax = 4.2;
const int batteryPin = A0;

void setup() {
  WiFi.mode(WIFI_STA);
	// Setup serial as 8E1 to communicate with sound meter
	Serial.begin(9600);

	// reset button pin mode 10k ohm pullup
	pinMode(BUTTON_PIN, INPUT_PULLUP);

  dht.begin();

	// set led pin as output and blink 0.5s interval until connected
	pinMode(LED_BUILTIN, OUTPUT);
	ticker.attach(0.5, led_tick);
	
	load_settings();

	//get chipid as string id
	String id = String(ESP.getChipId());
	
	// WiFiManager
	// Local intialization. Once its business is done, there is no need to keep it around
	WiFiManager wifiManager;
  String apName = "Temp Sensor " + id;
	wifiManager.setAPCallback(configModeCallback);
	wifiManager.setSaveConfigCallback(saveConfigCallback);

  if (digitalRead(BUTTON_PIN) == LOW) {
    Serial.println("Button pressed during restart. Erasing settings...");
    erase_settings();
    wifiManager.resetSettings();
    ESP.restart();
  } else {
    

		// custom parameters
		WiFiManagerParameter custom_name("name", "Sensor name", name, 40, " required");
		wifiManager.addParameter(&custom_name);
		WiFiManagerParameter custom_email("email", "User email", email, 40, " required");
		wifiManager.addParameter(&custom_email);
		WiFiManagerParameter custom_location("lokacja", "Sensor location", lokacja, 40, " required");
		wifiManager.addParameter(&custom_location);
		WiFiManagerParameter custom_dsTime("dsTime", "Deep sleep time", dsTime, 2, " required maxlength=2");
		wifiManager.addParameter(&custom_dsTime);
		WiFiManagerParameter custom_postUrl("postUrl", "Server adress", postUrl, 100, " required");
		wifiManager.addParameter(&custom_postUrl);
		
    wifiManager.setConnectTimeout(20);
    wifiManager.setTimeout(180);
			
		// try to connect or fallback to ESP+ChipID AP config mode.
		if ( ! wifiManager.autoConnect(apName.c_str())) {
			// reset and try again, or maybe put it to deep sleep
      Serial.println("Failed to connect or hit timeout");
      delay(1000);
			ESP.deepSleep(60000000);
		}
		
		// read paramters
		strcpy(name, custom_name.getValue());
		strcpy(email, custom_email.getValue());
		strcpy(lokacja, custom_location.getValue());
		strcpy(dsTime, custom_dsTime.getValue());
		strcpy(postUrl, custom_postUrl.getValue());
		
		// if we went through config, we should save our changes.
		if (shouldSaveConfig) save_settings();
		
		// no more blinking
		ticker.detach();
	}
}

void loop() {
	String id = String(ESP.getChipId());
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  // float temperature = 20;
  // float humidity = 40;
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" *C | Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  if(postUrl)

	if (isnan(temperature) || isnan(humidity)) {
		Serial.println("Failed to read from DHT sensor!");
		ticker.attach(0.2, led_tick);
		delay(2000);
		ticker.detach();
		ESP.deepSleep(10000000);
	}else{
		// Create a JSON object
		StaticJsonBuffer<200> jsonBuffer;
		JsonObject& root = jsonBuffer.createObject();

		// Add the variables to the JSON object
		root["chipid"] = id;
		root["email"] = email;
		root["name"] = name;
		root["lokacja"] = lokacja;
		root["temperature"] = temperature;
		root["humidity"] = humidity;

		// Serialize the JSON object to a string
		String jsonString;
		root.printTo(jsonString);

    Serial.println(jsonString);
    Serial.println(postUrl);

		// Send the POST request
		if(sendPostRequest(postUrl, jsonString.c_str()) == 200){
      Serial.println("ok");
    }else{
      Serial.println("not ok");
    }
	}

	int num = atoi(dsTime);

  // ESP.deepSleep(60000000 * num);
  ESP.deepSleep(60000000 * num);
}


// WiFiManager requiring config save callback
void saveConfigCallback () {
	shouldSaveConfig = true;
}

// WiFiManager entering configuration mode callback
void configModeCallback(WiFiManager *myWiFiManager) {
	// entered config mode, make led toggle faster
	ticker.attach(0.2, led_tick);
}

// Saves custom parameters to /config.json on SPIFFS
void save_settings() {
	DynamicJsonBuffer jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();
	json["postUrl"] = postUrl;
	json["email"] = email;
	json["name"] = name;
	json["lokacja"] = lokacja;
	json["dsTime"] = dsTime;

	File configFile = SPIFFS.open("/config.json", "w");
	json.printTo(configFile);
	configFile.close();
}

// Loads custom parameters from /config.json on SPIFFS
void load_settings() {
  Serial.println("laduje konfiguracje");
	if (SPIFFS.begin() && SPIFFS.exists("/config.json")) {
		File configFile = SPIFFS.open("/config.json", "r");
		Serial.println("laduje konfiguracje, plik znaleziony");
		if (configFile) {
			size_t size = configFile.size();
			// Allocate a buffer to store contents of the file.
			std::unique_ptr<char[]> buf(new char[size]);

			configFile.readBytes(buf.get(), size);
			DynamicJsonBuffer jsonBuffer;
			JsonObject& json = jsonBuffer.parseObject(buf.get());
			
			if (json.success()) {
				strcpy(postUrl, json["postUrl"]);
        Serial.print("Zaladowany ");
        Serial.println(postUrl);
				strcpy(email, json["email"]);
        Serial.print("Zaladowany ");
        Serial.println(email);
				strcpy(name, json["name"]);
				strcpy(lokacja, json["lokacja"]);
				strcpy(dsTime, json["dsTime"]);
			} else {
				Serial.println("failed to load config.json");
			}
		}else{
			Serial.println("failed to load config.json");
		}
	}else{
		Serial.println("file config.json does not exist");
  }
}

// Remove the file
void erase_settings() {
  if (SPIFFS.begin() && SPIFFS.exists("/config.json")) {
    SPIFFS.remove("/config.json");
  }
}

// Called by ticker for LED toggle
void led_tick() {
	// toggle LEF state
	int state = digitalRead(LED_BUILTIN);
	digitalWrite(LED_BUILTIN, !state);
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