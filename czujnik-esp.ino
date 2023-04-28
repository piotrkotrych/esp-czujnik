#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <EEPROM.h>

// Define the global variables to store the WiFi credentials and email
String ssid;
String password;
String email;

// Define the EEPROM addresses for storing the WiFi credentials and email
int ssidAddress = 0;
int passwordAddress = 32;
int emailAddress = 64;

void setup() {
  // Start the serial communication
  Serial.begin(9600);
  
  // Read the stored values from EEPROM
  readCredentials();
  
  // If the WiFi credentials are not stored, start the access point
  if (ssid == "" || password == "") {
    startAccessPoint();
  } else { // Otherwise, connect to the stored WiFi network
    connectWiFi();
  }
}

void loop() {
  // Your code for normal operation goes here
}

void startAccessPoint() {
  // Create an instance of WiFiManager
  WiFiManager wifiManager;

  // Set the custom parameters to configure the access point
  WiFiManagerParameter customSsid("ssid", "SSID", "", 32);
  WiFiManagerParameter customPassword("password", "Password", "", 32);
  WiFiManagerParameter customEmail("email", "Email", "", 64);
  wifiManager.addParameter(&customSsid);
  wifiManager.addParameter(&customPassword);
  wifiManager.addParameter(&customEmail);

  // Start the access point and the web server
  wifiManager.autoConnect("ESP Access Point");

  // Read the values from the custom parameters
  ssid = customSsid.getValue();
  password = customPassword.getValue();
  email = customEmail.getValue();

  // Save the values to the EEPROM
  saveCredentials();

  // Restart the ESP to connect to the configured WiFi network
  ESP.reset();
}

void connectWiFi() {
  // Connect to the stored WiFi network
  WiFi.begin(ssid.c_str(), password.c_str());
  
  // Wait for the WiFi connection to be established
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  // Print the local IP address
  Serial.print("Local IP address: ");
  Serial.println(WiFi.localIP());
}

void saveCredentials() {
  // Erase the EEPROM contents
  for (int i = 0; i < 512; i++) {
    EEPROM.write(i, 0);
  }

  // Write the WiFi credentials and email to the EEPROM
  for (int i = 0; i < ssid.length(); i++) {
    EEPROM.write(ssidAddress + i, ssid[i]);
  }
  for (int i = 0; i < password.length(); i++) {
    EEPROM.write(passwordAddress + i, password[i]);
  }
  for (int i = 0; i < email.length(); i++) {
    EEPROM.write(emailAddress + i, email[i]);
  }

  // Commit the changes to the EEPROM
  EEPROM.commit();
}

void readCredentials() {
  // Read the WiFi credentials and email from the EEPROM
  for (int i = 0; i < 32; i++) {
    ssid += char(EEPROM.read(ssidAddress + i));
    password += char(EEPROM.read(passwordAddress + i));
  }
  for (int i = 0; i < 64; i++) {
    email += char(EEPROM.read(emailAddress + i));
  }
}
