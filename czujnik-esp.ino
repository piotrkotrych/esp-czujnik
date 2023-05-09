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
WiFiClient espClient;

#include <Ticker.h>
Ticker ticker;

DHT dht(DHTPIN, DHTTYPE);

bool shouldSaveConfig = false;
char mqtt_server[40] = "mqtt";
char mqtt_port[6] = "1883";
char mqtt_topic[40] = "sensors/esp-sensor";
char poll_interval[6] = "5";
char sensor_name[40] = "sensorA";

void setup() {
	// Setup serial as 8E1 to communicate with sound meter
	Serial.begin(9600);

  dht.begin();

	// set led pin as output and blink 0.5s interval until connected
	pinMode(LED_BUILTIN, OUTPUT);
	ticker.attach(0.5, led_tick);
	
	load_settings();
	
	// WiFiManager
	// Local intialization. Once its business is done, there is no need to keep it around
	WiFiManager wifiManager;
	wifiManager.setAPCallback(configModeCallback);
	wifiManager.setSaveConfigCallback(saveConfigCallback);
	
	// custom parameters
	WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
	wifiManager.addParameter(&custom_mqtt_server);
	WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
	wifiManager.addParameter(&custom_mqtt_port);
	WiFiManagerParameter custom_mqtt_topic("topic", "mqtt topic", mqtt_topic, 40);
	wifiManager.addParameter(&custom_mqtt_topic);
	WiFiManagerParameter custom_poll_interval("interval", "poll interval", poll_interval, 6);
	wifiManager.addParameter(&custom_poll_interval);
	WiFiManagerParameter custom_sensor_name("sensor name", "sensor name", sensor_name, 40);
	wifiManager.addParameter(&custom_sensor_name);
		
	// try to connect or fallback to ESP+ChipID AP config mode.
	if ( ! wifiManager.autoConnect()) {
		// reset and try again, or maybe put it to deep sleep
		ESP.reset();
		delay(1000);
	}
	
	// read paramters
	strcpy(mqtt_server, custom_mqtt_server.getValue());
	strcpy(mqtt_port, custom_mqtt_port.getValue());
	strcpy(mqtt_topic, custom_mqtt_topic.getValue());
	strcpy(poll_interval, custom_poll_interval.getValue());
	strcpy(sensor_name, custom_sensor_name.getValue());
	
	// if we went through config, we should save our changes.
	if (shouldSaveConfig) save_settings();
	
	// no more blinking
	ticker.detach();
	
}

void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" *C | Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
  ESP.deepSleep(300000);
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
	json["mqtt_server"] = mqtt_server;
	json["mqtt_port"] = mqtt_port;
	json["mqtt_topic"] = mqtt_topic;
	json["poll_interval"] = poll_interval;
	json["sensor_name"] = sensor_name;

	File configFile = SPIFFS.open("/config.json", "w");
	json.printTo(configFile);
	configFile.close();
}

// Loads custom parameters from /config.json on SPIFFS
void load_settings() {
	if (SPIFFS.begin() && SPIFFS.exists("/config.json")) {
		File configFile = SPIFFS.open("/config.json", "r");
		
		if (configFile) {
			size_t size = configFile.size();
			// Allocate a buffer to store contents of the file.
			std::unique_ptr<char[]> buf(new char[size]);

			configFile.readBytes(buf.get(), size);
			DynamicJsonBuffer jsonBuffer;
			JsonObject& json = jsonBuffer.parseObject(buf.get());
			
			if (json.success()) {
				strcpy(mqtt_server, json["mqtt_server"]);
				strcpy(mqtt_port, json["mqtt_port"]);
				strcpy(mqtt_topic, json["mqtt_topic"]);
				strcpy(poll_interval, json["poll_interval"]);
				strcpy(sensor_name, json["sensor_name"]);
			}
		}
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