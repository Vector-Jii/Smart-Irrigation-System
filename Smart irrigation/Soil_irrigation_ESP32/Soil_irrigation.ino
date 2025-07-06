#include <WiFi.h>
#include <WebServer.h>
#include <NewPing.h>
#include <DHT.h>

// Pin Definitions
#define TRIGGER_PIN 23      // Change this as per your ESP32 setup
#define ECHO_PIN 22         // Change this as per your ESP32 setup
#define FLOW_SENSOR 21      // Change this as per your ESP32 setup
#define SOIL_MOISTURE 34    // Use an ADC pin on ESP32
#define RAIN_SENSOR 32      // Change this as per your ESP32 setup
#define PUMP_RELAY 2       // Change this as per your ESP32 setup
#define DHT_PIN 25          // Pin connected to the DHT22 sensor
#define DHT_TYPE DHT22      // DHT22 sensor type

// Constants
#define MAX_DISTANCE 200
#define TANK_HEIGHT 50
#define DRY_SOIL 700
#define WET_SOIL 300
#define RAIN_THRESHOLD 500

// WiFi Credentials
const char* ssid = "LAPTOP-U3UKOTIH 5306";
const char* password = "S276<5d1";

// Global Variables
volatile int flowPulseCount = 0;
float flowRate = 0.0;
unsigned int totalMilliliters = 0;
unsigned long oldTime = 0;
int soilValue = 0;
bool isRaining = false;
unsigned int waterLevel = 0;
bool needsWater = false;

WebServer server(80);
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
DHT dht(DHT_PIN, DHT_TYPE);  // Initialize DHT sensor

void IRAM_ATTR pulseCounter() {
  flowPulseCount++;
}

void setup() {
  Serial.begin(115200);

  // Initialize pins
  pinMode(FLOW_SENSOR, INPUT_PULLUP);
  pinMode(RAIN_SENSOR, INPUT);
  pinMode(PUMP_RELAY, OUTPUT);
  digitalWrite(PUMP_RELAY, LOW);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP address: ");
  Serial.println(WiFi.localIP());

  // Attach interrupt for flow sensor
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR), pulseCounter, FALLING);

  // Initialize DHT sensor
  dht.begin();

  // Set up web server
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Read sensors
  soilValue = analogRead(SOIL_MOISTURE);
  isRaining = digitalRead(RAIN_SENSOR) == LOW;

  // Calculate water level
  unsigned int waterDistance = sonar.ping_cm();
  waterLevel = TANK_HEIGHT - waterDistance;

  // Calculate flow rate
  if (millis() - oldTime >= 1000) {
    detachInterrupt(digitalPinToInterrupt(FLOW_SENSOR));
    flowRate = ((1000.0 / (millis() - oldTime)) * flowPulseCount) / 7.5;
    oldTime = millis();
    float milliliters = (flowRate / 60) * 1000;
    totalMilliliters += milliliters;
    flowPulseCount = 0;
    attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR), pulseCounter, FALLING);
  }

  // Control pump
  needsWater = soilValue > DRY_SOIL && !isRaining && waterLevel > 10;
  digitalWrite(PUMP_RELAY, needsWater ? HIGH : LOW);

  // Handle web requests
  server.handleClient();

  // Serial output for debugging
  Serial.print("Soil: ");
  Serial.print(soilValue);
  Serial.print(" | Rain: ");
  Serial.print(isRaining ? "YES" : "NO");
  Serial.print(" | Tank: ");
  Serial.print(waterLevel);
  Serial.print("cm | Flow: ");
  Serial.print(flowRate);
  Serial.print(" L/min | Pump: ");
  Serial.println(needsWater ? "ON" : "OFF");

  delay(1000);
}

void handleRoot() {
  // Read temperature and humidity from DHT22
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Prepare the HTML page with the sensor data
  String html = R"=====( 
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Smart Irrigation System</title>
  <meta http-equiv="refresh" content="5">
  <style>
    body {
      font-family: 'Arial', sans-serif;
      background-color: #f4f4f4;
      margin: 0;
      padding: 0;
      color: #333;
    }
    header {
      background-color: #4CAF50;
      color: white;
      text-align: center;
      padding: 20px;
      font-size: 2em;
      font-weight: bold;
    }
    .container {
      display: flex;
      flex-wrap: wrap;
      justify-content: space-evenly;
      padding: 20px;
    }
    .sensor {
      background: #fff;
      padding: 20px;
      margin: 15px;
      border-radius: 8px;
      box-shadow: 0 2px 8px rgba(0, 0, 0, 0.1);
      width: 300px;
      text-align: center;
      transition: transform 0.2s;
    }
    .sensor:hover {
      transform: scale(1.05);
    }
    .gauge {
      height: 25px;
      background: #ddd;
      border-radius: 10px;
      margin: 10px 0;
      overflow: hidden;
    }
    .gauge-fill {
      height: 100%;
      background: #4CAF50;
    }
    .pump-status {
      color: white;
      padding: 10px 20px;
      border-radius: 5px;
      margin-top: 10px;
    }
    .pump-on { background: #4CAF50; }
    .pump-off { background: #F44336; }
    .rain-yes { color: #2196F3; font-weight: bold; }
    .rain-no { color: #FF9800; }
    .sensor h2 {
      font-size: 1.2em;
      margin: 10px 0;
    }
    .sensor p {
      font-size: 1em;
      margin: 5px 0;
    }
    .footer {
      background-color: #333;
      color: white;
      text-align: center;
      padding: 10px;
      position: fixed;
      width: 100%;
      bottom: 0;
    }
    @media (max-width: 768px) {
      .container {
        flex-direction: column;
        align-items: center;
      }
      .sensor {
        width: 90%;
        margin: 10px 0;
      }
    }
  </style>
</head>
<body>
  <header>
    Smart Irrigation System Dashboard
  </header>

  <div class="container">
    <div class="sensor">
      <h2>Soil Moisture</h2>
      <div class="gauge">
        <div class="gauge-fill" style="width: %SOIL_PERCENT%%"></div>
      </div>
      <p>Value: %SOIL% (Dry > %DRY_SOIL%, Wet < %WET_SOIL%)</p>
    </div>

    <div class="sensor">
      <h2>Water Tank</h2>
      <div class="gauge">
        <div class="gauge-fill" style="width: %TANK_PERCENT%%"></div>
      </div>
      <p>%TANK_LEVEL% cm / %TANK_HEIGHT% cm</p>
    </div>

    <div class="sensor">
      <h2>Temperature: %TEMP% °C</h2>
      <h2>Humidity: %HUMIDITY% %</h2>
    </div>

    <div class="sensor">
      <h2>Water Flow</h2>
      <p>Flow Rate: %FLOW_RATE% L/min</p>
      <p>Total Used: %TOTAL_WATER% mL</p>
    </div>

    <div class="sensor">
      <h2>Rain: <span class="%RAIN_CLASS%">%RAIN%</span></h2>
    </div>

    <div class="sensor">
      <h2>🚿 Pump Status</h2>
      <div class="pump-status %PUMP_CLASS%">%PUMP_STATUS%</div>
    </div>
  </div>

  <div class="footer">
    Auto-refreshing | IP: %IP% | %DATE%
  </div>

  <script>
    function updateTime() {
      const now = new Date();
      document.getElementById('datetime').innerText = now.toLocaleString();
    }
    setInterval(updateTime, 1000);
    updateTime();
  </script>
</body>
</html>
)=====";

  // Replace placeholders with actual values
  html.replace("%SOIL%", String(soilValue));
  html.replace("%SOIL_PERCENT%", String(map(soilValue, DRY_SOIL, WET_SOIL, 0, 100)));
  html.replace("%TANK_LEVEL%", String(waterLevel));
  html.replace("%TANK_PERCENT%", String(map(waterLevel, 0, TANK_HEIGHT, 0, 100)));
  html.replace("%FLOW_RATE%", String(flowRate, 2));
  html.replace("%TOTAL_WATER%", String(totalMilliliters));
  html.replace("%RAIN%", isRaining ? "Detected" : "Not Detected");
  html.replace("%RAIN_CLASS%", isRaining ? "rain-yes" : "rain-no");
  html.replace("%PUMP_STATUS%", needsWater ? "ACTIVE" : "INACTIVE");
  html.replace("%PUMP_CLASS%", needsWater ? "pump-on" : "pump-off");
  html.replace("%TEMP%", String(temperature, 1));
  html.replace("%HUMIDITY%", String(humidity, 1));
  html.replace("%IP%", WiFi.localIP().toString());
  html.replace("%DATE%", "<span id='datetime'></span>");

  server.send(200, "text/html", html);
}

void handleData() {
  String json = "{";
  json += "\"soil\":" + String(soilValue) + ",";
  json += "\"tank_level\":" + String(waterLevel) + ",";
  json += "\"flow_rate\":" + String(flowRate, 2) + ",";
  json += "\"total_water\":" + String(totalMilliliters) + ",";
  json += "\"rain\":" + String(isRaining ? 1 : 0) + ",";
  json += "\"pump\":" + String(needsWater ? 1 : 0);
  json += "}";

  server.send(200, "application/json", json);
}
