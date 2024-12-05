
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
#define WIFI_SSID "Siddhartrockx"
#define WIFI_PASS "$iddhart@Wif@0152"

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

  // Read Water Level Sensor
  waterLevelValue = analogRead(WATER_LEVEL_PIN) * (3.3 / 4095.0);

  // Read Water Temperature Sensor
  waterTempSensor.requestTemperatures();
  waterTemperature = waterTempSensor.getTempCByIndex(0);

  // Handle Web Client Requests
  WiFiClient client = server.available();
  if (client) {
    String request = client.readStringUntil('\r');
    client.flush();

    // Send HTML Response
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println("<!DOCTYPE html>");
    client.println("<html>");
    client.println("<head>");
    client.println("<title>Water Quality Monitoring</title>");
    client.println("<style>");
    client.println("body { font-family: Arial, sans-serif; background: #f0f4f8; color: #333; text-align: center; margin: 0; padding: 0; }");
    client.println("h1 { background: #0078d7; color: white; padding: 20px 0; margin: 0; }");
    client.println(".container { margin: 20px auto; max-width: 800px; text-align: left; }");
    client.println(".card { background: white; border-radius: 8px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); margin: 20px 0; padding: 20px; }");
    client.println(".level { display: flex; align-items: center; margin: 10px 0; }");
    client.println(".level-label { width: 20%; font-weight: bold; }");
    client.println(".progress { flex-grow: 1; height: 20px; border-radius: 10px; background: #e0e0e0; overflow: hidden; }");
    client.println(".progress-bar { height: 100%; background: #0078d7; transition: width 0.3s; }");
    client.println("</style>");
    client.println("</head>");
    client.println("<body>");
    client.println("<h1>Water Quality Monitoring Dashboard</h1>");
    client.println("<div class='container'>");

    // TDS Sensor Card
    client.println("<div class='card'>");
    client.printf("<div class='level'><span class='level-label'>TDS:</span><div class='progress'><div class='progress-bar' style='width: %.2f%%'></div></div></div>", tdsValue / 1000 * 100);
    client.printf("<p>Value: %.2f ppm</p>", tdsValue);
    client.println("</div>");

    // Turbidity Sensor Card
    client.println("<div class='card'>");
    client.printf("<div class='level'><span class='level-label'>Turbidity:</span><div class='progress'><div class='progress-bar' style='width: %.2f%%'></div></div></div>", turbidityValue / 3.3 * 100);
    client.printf("<p>Value: %.2f NTU</p>", turbidityValue);
    client.println("</div>");

    // Water Level Sensor Card
    client.println("<div class='card'>");
    client.printf("<div class='level'><span class='level-label'>Water Level:</span><div class='progress'><div class='progress-bar' style='width: %.2f%%'></div></div></div>", waterLevelValue / 3.3 * 100);
    client.printf("<p>Value: %.2f</p>", waterLevelValue);
    client.println("</div>");

    // Water Temperature Sensor Card
    client.println("<div class='card'>");
    client.printf("<div class='level'><span class='level-label'>Temperature:</span><div class='progress'><div class='progress-bar' style='width: %.2f%%'></div></div></div>", waterTemperature / 50 * 100);  // Assuming 50°C as max temp
    client.printf("<p>Value: %.2f °C</p>", waterTemperature);
    client.println("</div>");

    client.println("</div>");
    client.println("</body>");
    client.println("</html>");
    delay(1);
  }

  // Display Data on OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Water Quality System");

  display.setCursor(0, 10);
  display.printf("TDS: %.2f ppm", tdsValue);

  display.setCursor(0, 20);
  display.printf("Turbidity: %.2f NTU", turbidityValue);

  display.setCursor(0, 30);
  display.printf("Water Level: %.2f", waterLevelValue);

  display.setCursor(0, 40);
  display.printf("Water Temp: %.2f C", waterTemperature);

  display.display();

  delay(2000);
}
