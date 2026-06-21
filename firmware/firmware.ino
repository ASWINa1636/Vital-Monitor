#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>

/* ================= WIFI ================= */
const char* ssid = "wifi_name";
const char* password = "wifi_password";

/* ================= OLED ================= */
Adafruit_SSD1306 display(128, 64, &Wire, -1);

/* ================= SENSOR ================= */
MAX30105 sensor;
bool sensorReady = false;   // <-- flag: only read sensor if init succeeded

/* ================= BUZZER ================= */
#define BUZZER 23

/* ================= VARIABLES ================= */
float bpm = 0;
int spo2 = 0;
int hrv = 0;
bool alert = false;

long lastBeat = 0;

/* ================= FILTER ================= */
#define FILTER_SIZE 5
long irFilter[FILTER_SIZE];
int fIndex = 0;

long filterIR(long input) {
  irFilter[fIndex++] = input;
  if (fIndex >= FILTER_SIZE) fIndex = 0;

  long sum = 0;
  for (int i = 0; i < FILTER_SIZE; i++) sum += irFilter[i];

  return sum / FILTER_SIZE;
}

/* ================= HRV ================= */
#define HRV_BUFFER 10
long rrIntervals[HRV_BUFFER];
int rrIndex = 0;
bool bufferFilled = false;

/* ================= SPO2 ================= */
#define SPO2_BUFFER 50
long redBuffer[SPO2_BUFFER];
long irBuffer[SPO2_BUFFER];
int spo2Index = 0;

/* ================= SERVER ================= */
WebServer server(80);

/* ================= DISPLAY ================= */
void updateDisplay() {
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 0);

  if (!sensorReady) {
    display.println("!! SENSOR FAIL !!");
    display.println("");
    display.println("Web server running");
    display.print("IP: ");
    display.println(WiFi.localIP());
  } else {
    display.println("IoMT Monitor");

    display.setTextSize(2);
    display.setCursor(0, 15);
    display.print("HR:");
    display.println((int)bpm);

    display.setCursor(0, 35);
    display.print("O2:");
    display.print(spo2);
    display.println("%");

    display.setTextSize(1);
    display.setCursor(0, 55);
    display.print("HRV:");
    display.print(hrv);
  }

  display.display();
}

/* ================= SPO2 CALC ================= */
int calculateSpO2() {

  float redMean = 0, irMean = 0;

  for (int i = 0; i < SPO2_BUFFER; i++) {
    redMean += redBuffer[i];
    irMean += irBuffer[i];
  }

  redMean /= SPO2_BUFFER;
  irMean /= SPO2_BUFFER;

  float redAC = 0, irAC = 0;

  for (int i = 0; i < SPO2_BUFFER; i++) {
    redAC += abs(redBuffer[i] - redMean);
    irAC += abs(irBuffer[i] - irMean);
  }

  redAC /= SPO2_BUFFER;
  irAC /= SPO2_BUFFER;

  if (irAC == 0 || irMean == 0) return 0;

  float R = (redAC / redMean) / (irAC / irMean);

  int value = 110 - (25 * R);

  return constrain(value, 80, 100);
}

/* ================= SETUP ================= */
void setup() {
  Serial.begin(115200);

  pinMode(BUZZER, OUTPUT);

  /* ---- WiFi (DYNAMIC IP for hotspot) ---- */
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  /* ---- OLED ---- */
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Connected!");
  display.println("IP Address:");
  display.println(WiFi.localIP());
  display.display();

  delay(3000);

  /* ---- WEB SERVER FIRST (before sensor!) ---- */
  /* Start the web server BEFORE sensor init so it is always reachable */

  server.enableCORS(true);

  server.on("/", HTTP_GET, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", "VitalGuard ESP32 OK");
  });

  server.on("/vitals", HTTP_GET, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");

    StaticJsonDocument<200> doc;
    doc["bpm"] = (int)bpm;
    doc["spo2"] = spo2;
    doc["hrv"] = hrv;
    doc["alert"] = alert;
    doc["sensorReady"] = sensorReady;

    String res;
    serializeJson(doc, res);
    server.send(200, "application/json", res);
  });

  server.onNotFound([]() {
    if (server.method() == HTTP_OPTIONS) {
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
      server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
      server.send(204);
    } else {
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(404, "text/plain", "Not found");
    }
  });

  server.begin();
  Serial.println("HTTP server started on port 80");

  /* ---- Sensor (AFTER server, so server works even if sensor fails) ---- */
  if (sensor.begin(Wire, I2C_SPEED_FAST)) {
    sensor.setup(0x1F, 4, 2, 100, 411, 4096);
    sensorReady = true;
    Serial.println("MAX30102 sensor ready");
  } else {
    sensorReady = false;
    Serial.println("WARNING: MAX30102 sensor NOT found! Server still running.");
  }
}

/* ================= LOOP ================= */
void loop() {

  /* Always handle web requests */
  server.handleClient();

  /* Skip sensor reading if sensor failed to init */
  if (!sensorReady) {
    delay(100);
    updateDisplay();
    return;
  }

  long irRaw = sensor.getIR();
  long red = sensor.getRed();

  long ir = filterIR(irRaw);

  /* Finger detection */
  if (ir < 50000) {
    bpm = 0;
    spo2 = 0;
    hrv = 0;
    updateDisplay();
    return;
  }

  /* Store SPO2 buffers */
  redBuffer[spo2Index] = red;
  irBuffer[spo2Index] = ir;

  spo2Index++;
  if (spo2Index >= SPO2_BUFFER) spo2Index = 0;

  /* Heart Rate */
  if (checkForBeat(ir)) {

    long now = millis();
    long interval = now - lastBeat;
    lastBeat = now;

    float newBpm = 60 / (interval / 1000.0);

    /* Smooth BPM */
    bpm = (0.8 * bpm) + (0.2 * newBpm);

    /* HRV */
    rrIntervals[rrIndex++] = interval;

    if (rrIndex >= HRV_BUFFER) {
      rrIndex = 0;
      bufferFilled = true;
    }

    if (bufferFilled) {
      float mean = 0;
      for (int i = 0; i < HRV_BUFFER; i++) mean += rrIntervals[i];
      mean /= HRV_BUFFER;

      float variance = 0;
      for (int i = 0; i < HRV_BUFFER; i++) {
        variance += pow(rrIntervals[i] - mean, 2);
      }
      variance /= HRV_BUFFER;

      hrv = sqrt(variance);
    }
  }

  /* SpO2 */
  spo2 = calculateSpO2();

  /* Alert */
  alert = (spo2 < 90 || bpm > 120);

  digitalWrite(BUZZER, alert ? HIGH : LOW);

  updateDisplay();
}