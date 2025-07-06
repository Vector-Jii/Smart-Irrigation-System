#include <Servo.h>

// Define the pins
const int soilSensorPin = A0;   // Analog pin for soil moisture sensor
const int pumpPin = 8;          // Digital pin for water pump (Relay control)
const int rainSensorPin = A1;   // Analog pin for rain sensor
const int servoPin = 9;         // PWM-capable pin for servo

// Moisture threshold values
const int dryThreshold = 700;   // Soil is considered dry when moisture level is higher than this
const int wetThreshold = 300;   // Soil is considered wet when moisture level is lower than this

// Watering duration in milliseconds
const int wateringDuration = 2000; // Adjust watering duration (2 seconds in this case)

// Rain detection threshold
const int rainThreshold = 500;  // Adjust based on your rain sensor's output

// Create servo object
Servo rainServo;

// Flag to track whether the pump is already on or not
bool pumpActive = false;

void setup() {
  pinMode(soilSensorPin, INPUT);  // Set the soil sensor pin as input
  pinMode(rainSensorPin, INPUT);  // Set the rain sensor pin as input
  pinMode(pumpPin, OUTPUT);       // Set the pump pin as output

  rainServo.attach(servoPin);     // Attach the servo to the pin

  Serial.begin(9600);             // Start serial communication for debugging
}

void loop() {
  // Read soil moisture and average over multiple readings to get a stable value
  int soilMoisture = 0;
  for (int i = 0; i < 5; i++) {
    soilMoisture += analogRead(soilSensorPin);
    delay(50);  // Small delay between readings
  }
  soilMoisture /= 5; // Average the readings
  Serial.print("Soil Moisture: ");
  Serial.println(soilMoisture);

  // Read rain sensor and average over multiple readings
  int rainValue = 0;
  for (int i = 0; i < 5; i++) {
    rainValue += analogRead(rainSensorPin);
    delay(50);  // Small delay between readings
  }
  rainValue /= 5; // Average the readings
  Serial.print("Rain Sensor: ");
  Serial.println(rainValue);

  // Control the water pump based on soil moisture
  if (soilMoisture > dryThreshold && !pumpActive) {
    // If soil is dry and the pump is not already active
    digitalWrite(pumpPin, HIGH);  // Turn pump on
    pumpActive = true;  // Set pumpActive to true to prevent repeated switching
    Serial.println("Watering soil...");
    delay(wateringDuration);       // Wait for the watering duration
    digitalWrite(pumpPin, LOW);   // Turn pump off
    pumpActive = false;  // Reset pumpActive after watering
    Serial.println("Soil watered.");
  }

  // Control the servo based on rain detection
  if (rainValue < rainThreshold) {
    // If rain is detected, move servo to 180 degrees
    if (rainServo.read() != 180) {
      rainServo.write(180);
      Serial.println("Rain detected! Moving servo to 180 degrees.");
      delay(1000);  // Delay to prevent constant servo movement
    }
  } else {
    // If no rain, move servo back to 0 degrees (initial position)
    if (rainServo.read() != 0) {
      rainServo.write(0);
      Serial.println("No rain. Moving servo back to 0 degrees.");
      delay(1000);  // Delay to prevent constant servo movement
    }
  }

  delay(1000); // Delay before the next loop iteration to avoid excessive sensor readings
}
