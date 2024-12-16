#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>

// Pin Definitions
#define TDS_PIN 34
#define TURBIDITY_PIN 35
#define WATER_LEVEL_PIN 32
#define TEMP_SENSOR_PIN 4

// OLED Display Config
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Temperature Sensor Config
OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature waterTempSensor(&oneWire);

// WiFi Config
#define WIFI_SSID "how"
#define WIFI_PASS "12345678"

// Global Variables
float tdsValue = 0;
float turbidityValue = 0;
float waterLevelValue = 0;
float waterTemperature = 0;

// Web Server on port 80
WiFiServer server(80);

void setup() {
  Serial.begin(115200);

  // Initialize WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Start Web Server
  server.begin();

  // Initialize Sensors
  waterTempSensor.begin();

  // Initialize OLED Display
  if (!display.begin(SSD1306_PAGEADDR, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  display.clearDisplay();
  display.display();

  Serial.println("Water Quality Monitoring System Initialized");
}

void loop() {
  // Read TDS Sensor
  tdsValue = analogRead(TDS_PIN) * (3.3 / 4095.0) * 1000;

  // Read Turbidity Sensor
  turbidityValue = analogRead(TURBIDITY_PIN) * (3.3 / 4095.0);

  // Read Water Level Sensor and Calculate Percentage
  float waterLevelRaw = analogRead(WATER_LEVEL_PIN) * (3.3 / 4095.0);
  waterLevelValue = (waterLevelRaw - 0.5) / (2 - 0.5) * 100;
  waterLevelValue = constrain(waterLevelValue, 0, 100);

  // Read Water Temperature Sensor
  waterTempSensor.requestTemperatures();
  waterTemperature = waterTempSensor.getTempCByIndex(0);

  // Handle Web Client Requests
  WiFiClient client = server.available();
  if (client) {
    String request = client.readStringUntil('\r');
    client.flush();

    // Send Enhanced HTML Response
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println("<!DOCTYPE html>");
    client.println("<html>");
    client.println("<head>");
    client.println("<title>Water Quality Monitoring</title>");
    client.println("<style>");
    client.println("body { font-family: Arial, sans-serif; background: #f8f9fa; color: #212529; margin: 0; padding: 0; }");
    client.println("header { background-color: #0078d7; color: white; padding: 15px 0; text-align: center; }");
    client.println("header h1 { margin: 0; font-size: 1.8rem; }");
    client.println(".container { max-width: 800px; margin: 20px auto; padding: 0 15px; }");
    client.println(".card { background: white; border-radius: 10px; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1); margin: 15px 0; padding: 20px; transition: transform 0.3s; }");
    client.println(".card:hover { transform: scale(1.03); }");
    client.println(".card h2 { font-size: 1.5rem; color: #0078d7; margin-bottom: 10px; }");
    client.println(".progress { width: 100%; background-color: #e9ecef; border-radius: 5px; overflow: hidden; height: 20px; }");
    client.println(".progress-bar { height: 100%; background-color: #0078d7; transition: width 0.3s; }");
    client.println(".info { font-size: 1rem; color: #495057; margin-top: 10px; }");
    client.println("</style>");
    client.println("</head>");
    client.println("<body>");
    client.println("<header>");
    client.println("<h1>Water Quality Monitoring Dashboard</h1>");
    client.println("</header>");
    client.println("<div class='container'>");

    // TDS Card
    client.println("<div class='card'>");
    client.println("<h2>TDS (Total Dissolved Solids)</h2>");
    client.printf("<div class='progress'><div class='progress-bar' style='width: %.2f%%'></div></div>", tdsValue / 1000 * 100);
    client.printf("<p class='info'>TDS Value: %.2f ppm</p>", tdsValue);
    client.println("</div>");

    // Turbidity Card
    client.println("<div class='card'>");
    client.println("<h2>Turbidity</h2>");
    client.printf("<div class='progress'><div class='progress-bar' style='width: %.2f%%'></div></div>", turbidityValue / 3.3 * 100);
    client.printf("<p class='info'>Turbidity Value: %.2f NTU</p>", turbidityValue);
    client.println("</div>");

    // Water Level Card
    client.println("<div class='card'>");
    client.println("<h2>Water Level</h2>");
    client.printf("<div class='progress'><div class='progress-bar' style='width: %.2f%%'></div></div>", waterLevelValue);
    client.printf("<p class='info'>Water Level: %.2f%%</p>", waterLevelValue);
    client.println("</div>");

    // Water Temperature Card
    client.println("<div class='card'>");
    client.println("<h2>Water Temperature</h2>");
    client.printf("<div class='progress'><div class='progress-bar' style='width: %.2f%%'></div></div>", waterTemperature / 50 * 100); // Assuming 50°C as max
    client.printf("<p class='info'>Temperature: %.2f °C</p>", waterTemperature);
    client.println("</div>");

    client.println("</div>");
    client.println("</body>");
    client.println("</html>");
    delay(1);
  }

  // Display Data on OLED
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println(" AquaPulse");

  display.setTextSize(1);
  display.setCursor(0, 16);
  display.println("   IP:");
  display.setCursor(35, 16);
  display.println(WiFi.localIP());
  display.setCursor(0, 28);
  display.printf("TDS:        %.2fppm", tdsValue);

  display.setCursor(0, 36);
  display.printf("Turb:        %.2f NTU", turbidityValue);

  display.setCursor(0, 46);
  display.printf("Water Level: %.2f%%", waterLevelValue);

  display.setCursor(0, 54);
  display.printf("Water Temp: %.2f C", waterTemperature);

  display.display();
  delay(2000);
}
