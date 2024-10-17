#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include "OTA_config.h"
#include "html.h" // Include the HTML template file

// Pin definitions
#define trigPin D5  // Trig pin of ultrasonic sensor
#define echoPin D6  // Echo pin of ultrasonic sensor

// SoftwareSerial object to communicate with SIM800L
SoftwareSerial sim800l(D2, D1); // TX, RX

// Configuration and state variables
bool isConnected = false;
unsigned long restartInterval = 5 * 60 * 1000; // 5 minutes
unsigned long lastRestartTime = 0;
const int samplePeriod = 100; // 100 milliseconds
const int dataPeriod = 20000; // 20 seconds
const int acceptableMin = 10; // Minimum distance in cm
const int acceptableMax = 250; // Maximum distance in cm
unsigned long lastSampleTime = 0;
unsigned long lastValidDataTime = 0;
unsigned long currentTime;
const int maxValues = dataPeriod / samplePeriod;
int values[maxValues];
int valuesCount = 0;
int currentIndex = 0;
const int NO_VALID_DATA = -1; // Special value for no valid data
unsigned long lastConnectionCheck = 0;
const unsigned long connectionCheckInterval = 5000; // 5 seconds

int heighttank = 240;
int gap = 7;

void setup() {
  Serial.begin(9600);
  sim800l.begin(9600);
  setupOTA();
  
  pinMode(trigPin, OUTPUT); // Trigger pin will output pulses
  pinMode(echoPin, INPUT);  // Echo pin will receive pulses

  initializeSIM800L();
  Serial.println("Setup complete");
  lastValidDataTime = millis(); // Initialize last valid data time
}

void loop() {
  currentTime = millis();
  handleOTA();
  handleRestart();
  checkGSMConnection();
  sampleSensor();
  processReceivedSMS();
}

void initializeSIM800L() {
  sim800l.println("AT"); // Check communication
  delay(1000);
  sim800l.println("AT+CMGF=1"); // Set to text mode
  delay(1000);
  sim800l.println("AT+CNMI=2,2,0,0,0"); // Auto-forward received SMS
  delay(1000);
}

void handleRestart() {
  if (currentTime - lastRestartTime >= restartInterval) {
    lastRestartTime = millis();
    ESP.restart(); // Soft reset the ESP8266
  }
}

void checkGSMConnection() {
  if (currentTime - lastConnectionCheck >= connectionCheckInterval) {
    lastConnectionCheck = currentTime;
    if (!isConnected) {
      isConnected = connectToNetwork();
      if (isConnected) {
        Serial.println("Connected to network");
        configureSMSMode();
      } else {
        Serial.println("Failed to connect to network");
        delay(5000); // Wait before retrying
      }
    }
  }
}

bool connectToNetwork() {
  for (int attempt = 0; attempt < 3; attempt++) {
    sim800l.println("AT+CREG?");
    delay(1000);
    if (sim800l.available()) {
      String response = sim800l.readString();
      if (response.indexOf("+CREG: 0,1") != -1 || response.indexOf("+CREG: 0,5") != -1) {
        return true;
      }
    }
    delay(1000);
  }
  return false;
}

void configureSMSMode() {
  sim800l.println("AT+CMGF=1");
  delay(1000);
  sim800l.println("AT+CNMI=2,2,0,0,0");
  delay(1000);
}

void sampleSensor() {
  if (currentTime - lastSampleTime >= samplePeriod) {
    int distance = getDistance();
    if (distance >= acceptableMin && distance <= acceptableMax) {
      values[currentIndex] = distance;
      currentIndex = (currentIndex + 1) % maxValues;
      if (valuesCount < maxValues) valuesCount++;
      lastValidDataTime = currentTime;
    }
    lastSampleTime = currentTime;
  }
}

int getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  return duration * 0.034 / 2;
}

int calculateMode() {
  if (valuesCount == 0) return NO_VALID_DATA;
  int mode = values[0], maxCount = 0;
  for (int i = 0; i < valuesCount; i++) {
    int count = 0;
    for (int j = 0; j < valuesCount; j++) {
      if (values[j] == values[i]) count++;
    }
    if (count > maxCount) {
      maxCount = count;
      mode = values[i];
    }
  }
  return maxCount > 1 ? mode : NO_VALID_DATA;
}

int getWaterLevel() {
  int distance = calculateMode();
  if (currentTime - lastValidDataTime >= dataPeriod) {
    distance = NO_VALID_DATA;
  }
  if (distance == NO_VALID_DATA) {
    Serial.println("No valid data");
    return NO_VALID_DATA;
  } else {
    Serial.println(distance);
    // Calculate water level in feet and percentage
    float waterLevel = heighttank - (distance - gap);
    float waterLevelFeet = waterLevel / 30.48;
    int waterLevelPercent = (waterLevel * 100) / heighttank;
    return waterLevel; // Return waterLevel instead of waterLevelFeet
  }
}


void processReceivedSMS() {
  if (sim800l.available()) {
    String sms = sim800l.readString();
    Serial.println(sms);
    if (sms.indexOf("?") != -1 || sms.indexOf("water") != -1 || sms.indexOf("Water") != -1 || sms.indexOf("level") != -1 || sms.indexOf("Level") != -1) {
      String senderNumber = parseSenderNumber(sms);
      if (senderNumber.length() > 0) {
        sendReply(senderNumber, getWaterLevel());
      }
    }
  }
}

String parseSenderNumber(String sms) {
  int startPos = sms.indexOf("\"", sms.indexOf("+"));
  int endPos = sms.indexOf("\"", startPos + 1);
  if (startPos != -1 && endPos != -1) {
    return sms.substring(startPos + 1, endPos);
  }
  return "";
}

void sendReply(String number, int waterLevel) {
  sim800l.println("AT+CMGS=\"" + number + "\"");
  delay(1000);
  if (waterLevel == NO_VALID_DATA) {
    sim800l.print("Dear user,\n");
    sim800l.print("  Please note that the water level is not measurable at the moment either because the Sensor has failed or the device has been misaligned/removed. Please take necessary actions to experience an uninterrupted service.\n\n");
    sim800l.print("SmartGauge,\n");
    sim800l.print("Jagro (pvt) ltd.");
  } else {
    float waterLevelFeet = waterLevel / 30.48;
    int waterLevelPercent = (waterLevel * 100) / heighttank;
    
    sim800l.print("Dear user,\n");
    sim800l.print("  The current water level is ");
    sim800l.print(waterLevelFeet, 2);
    sim800l.print(" feet (");
    sim800l.print(waterLevelPercent);
    sim800l.print("%).\n\n");
    sim800l.print("SmartGauge,\n");
    sim800l.print("Jagro (pvt) ltd.");
  }
  delay(1000);
  sim800l.write(26); // ASCII code for Ctrl+Z to send the SMS
  delay(1000);
}

