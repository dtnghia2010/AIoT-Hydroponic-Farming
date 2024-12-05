// #include <Wire.h>
// #include <BH1750.h>
// #include <OneWire.h>
// #include <DallasTemperature.h>

// #define DS18B20_PIN 4  // Pin for DS18B20 (GPIO4)
// #define RELAY_PIN 23   // Pin for Relay (GPIO26)
// #define BH1750_ADDR 0x23  // I2C address for BH1750

// // Create OneWire instance for DS18B20
// OneWire oneWire(DS18B20_PIN);
// DallasTemperature tempSensor(&oneWire);

// // Create BH1750 instance
// BH1750 lightSensor;

// void setup() {
//   // Start serial communication
//   Serial.begin(115200);
  
//   // Initialize I2C for BH1750
//   Wire.begin();
  
//   // Initialize BH1750 sensor
//   if (!lightSensor.begin()) {
//     Serial.println("BH1750 initialization failed!");
//     while (1);  // Halt if sensor initialization fails
//   }
//   Serial.println("BH1750 Initialized!");

//   // Initialize DS18B20 sensor
//   tempSensor.begin();
//   Serial.println("DS18B20 Initialized!");

//   // Initialize Relay pin
//   pinMode(RELAY_PIN, OUTPUT);
//   digitalWrite(RELAY_PIN, LOW);  // Turn relay off initially
// }

// void loop() {
//   // Request temperature from DS18B20
//   tempSensor.requestTemperatures();
//   float temperature = tempSensor.getTempCByIndex(0);  // Get temperature in Celsius
  
//   // Read light intensity from BH1750 (lux)
//   uint16_t lightLevel = lightSensor.readLightLevel();
  
//   // Print the temperature and light level to the Serial Monitor

  
//   // Control relay based on temperature or light level
//   // if (temperature > 30.0 || lightLevel < 100) {
//   //   // If temperature is above 30°C or light level is below 100 lux, turn on the relay
//   //   digitalWrite(RELAY_PIN, HIGH);  // Turn on relay
//   //   Serial.println("Relay ON");
//   // } else {
//   //   // Otherwise, turn off the relay
//   //   digitalWrite(RELAY_PIN, LOW);  // Turn off relay
//   //   Serial.println("Relay OFF");
//   // }
//   if(Serial.available()>0) {
//     String command = Serial.readStringUntil('\n');
//     if(command == "ON") {
//       digitalWrite(RELAY_PIN, HIGH);
//       Serial.println("Relay ON");
//       Serial.print("Temperature (°C): ");
//       Serial.println(temperature);
//       Serial.print("Light Level (lux): ");
//       Serial.println(lightLevel);
//     } else if (command = "OFF") {
//       digitalWrite(RELAY_PIN, LOW);
//       Serial.println("Relay OFF");
//     } else {
//       Serial.println("Undefined command!");
//     }
//   }

//   delay(1000);  // Wait for 1 second before reading again
// }
#include <Wire.h>
#include <BH1750.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <ThingSpeak.h>

// Define GPIO pins for controlling the L298N motor driver
#define IN1_PIN 23  // Control motor direction 1
#define IN2_PIN 19  // Control motor direction 2
#define DS18B20_PIN 4  // Pin for DS18B20 (GPIO4)

// Wi-Fi credentials
const char* ssid = "AIoT Lab VN";        // Replace with your Wi-Fi SSID
const char* password = "AIoTLab01082023"; // Replace with your Wi-Fi password

// ThingSpeak channel information
unsigned long channelID = 2776169; // Replace with your ThingSpeak Channel ID
const char* writeAPIKey = "I6G8EYDBG55RXMD1";   // Replace with your ThingSpeak Write API Key

BH1750 lightSensor;  // Create an instance of the BH1750 sensor
OneWire oneWire(DS18B20_PIN);
DallasTemperature tempSensor(&oneWire);

WiFiClient client;

unsigned long lastPrintTime = 0;  // Variable to track the last print time
const unsigned long printInterval = 600000;  // Interval to push data to ThingSpeak (10 minutes)

// Variables for tracking the pump cycle states
unsigned long lastPumpRunTime = 0;  // To track the time the pump has been running
unsigned long lastPumpRestTime = 0; // To track the time the pump has been resting

const unsigned long runTime = 7 * 60 * 60 * 1000;  // 7 hours in milliseconds
const unsigned long restTime = 1 * 60 * 60 * 1000; // 1 hour in milliseconds

bool pumpRunning = true; // Variable to track pump state

void setup() {
  Serial.begin(115200);
  Wire.begin();  // Initialize I2C communication
  if (!lightSensor.begin()) {
    Serial.println("BH1750 not detected!");
    while (1);  // Infinite loop if sensor is not found
  }
  Serial.println("BH1750 initialized.");
  tempSensor.begin();  // Initialize DS18B20 temperature sensor
  Serial.println("DS18B20 Initialized!");

  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  digitalWrite(IN1_PIN, HIGH);  // Motor forward
  digitalWrite(IN2_PIN, LOW);   // Motor forward

  Serial.print("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to Wi-Fi!");
  ThingSpeak.begin(client);
}

void loop() {
  // Read light and temperature data
  float lux = lightSensor.readLightLevel();
  tempSensor.requestTemperatures();
  float temperature = tempSensor.getTempCByIndex(0);  // Get temperature in Celsius

  if (millis() - lastPrintTime >= printInterval) {
    lastPrintTime = millis();
    Serial.print("Temperature (°C): ");
    Serial.println(temperature);
    Serial.print("Light Level (lux): ");
    Serial.println(lux);
    ThingSpeak.setField(1, temperature);
    ThingSpeak.setField(2, lux);
    ThingSpeak.setField(3, pumpRunning);
    int result = ThingSpeak.writeFields(channelID, writeAPIKey);
    if(result == 200) {
      Serial.println("Data successfully sent to ThingSpeak.");
    } else {
      Serial.print("Error sending data to ThingSpeak. Response: ");
      Serial.println(result);
    }
  }

  // Pump control logic
  if (pumpRunning && millis() - lastPumpRunTime >= runTime) {
    digitalWrite(IN1_PIN, LOW);  // Motor stopped
    digitalWrite(IN2_PIN, LOW);  // Motor stopped
    pumpRunning = false;
    lastPumpRestTime = millis();
    Serial.println("Pump is resting.");
  } else if (!pumpRunning && millis() - lastPumpRestTime >= restTime) {
    digitalWrite(IN1_PIN, HIGH);  // Motor forward
    digitalWrite(IN2_PIN, LOW);   // Motor forward
    pumpRunning = true;
    lastPumpRunTime = millis();
    Serial.println("Pump is running.");
  }

  // Check Wi-Fi status periodically and reconnect if needed
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Disconnected from Wi-Fi, attempting to reconnect...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
    }
    Serial.println("Reconnected to Wi-Fi!");
  }
}
