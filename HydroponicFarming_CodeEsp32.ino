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

// Define GPIO pins for controlling the L298N motor driver
#define IN1_PIN 23  // Control motor direction 1 (change to GPIO 17)
#define IN2_PIN 19  // Control motor direction 2 (change to GPIO 19)
#define LIGHT_THRESHOLD 100  // Lux threshold for turning on the pump
#define DS18B20_PIN 4  // Pin for DS18B20 (GPIO4)

BH1750 lightSensor;  // Create an instance of the BH1750 sensor
OneWire oneWire(DS18B20_PIN);
DallasTemperature tempSensor(&oneWire);
void setup() {
  // Start serial communication
  Serial.begin(115200);

  // Initialize I2C communication
  Wire.begin();

  // Initialize the BH1750 sensor
  if (!lightSensor.begin()) {
    Serial.println("BH1750 not detected!");
    while (1);  // Infinite loop if sensor is not found
  }
  Serial.println("BH1750 initialized.");
  tempSensor.begin();
  Serial.println("DS18B20 Initialized!");

  // Initialize L298N motor driver pins
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);

  // Ensure the pump is OFF initially
  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, LOW);
}

void loop() {
  // Read the light level (lux) from the BH1750
  float lux = lightSensor.readLightLevel();
  // Serial.print("Light Level: ");
  // Serial.print(lux);
  // Serial.println(" lux");
  tempSensor.requestTemperatures();
  float temperature = tempSensor.getTempCByIndex(0);  // Get temperature in Celsius

  // // Turn the pump on or off based on the light level
  // if (lux < LIGHT_THRESHOLD) {
  //   // Turn on the pump (forward direction)
  //   Serial.println("Turning on the pump...");
  //   digitalWrite(IN1_PIN, HIGH);  // Motor forward
  //   digitalWrite(IN2_PIN, LOW);   // Motor forward
  // } else {
  //   // Turn off the pump
  //   Serial.println("Turning off the pump...");
  //   digitalWrite(IN1_PIN, LOW);   // Stop motor
  //   digitalWrite(IN2_PIN, LOW);   // Stop motor
  // }
  if(Serial.available()>0) {
    String command = Serial.readStringUntil('\n');
    if(command == "ON") {
      Serial.println("Turning on the pump...");
      digitalWrite(IN1_PIN, HIGH);  // Motor forward
      digitalWrite(IN2_PIN, LOW);   // Motor forward
      Serial.print("Temperature (°C): ");
      Serial.println(temperature);
      Serial.print("Light Level (lux): ");
      Serial.println(lux);
    } else if (command = "OFF") {
      Serial.println("Turning off the pump...");
      digitalWrite(IN1_PIN, LOW);   // Stop motor
      digitalWrite(IN2_PIN, LOW);   // Stop motor
      // Serial.println("Relay OFF");
    } else {
      Serial.println("Undefined command!");
    }
  }

  // Delay before the next reading
  delay(1000);  // Update every second
}
