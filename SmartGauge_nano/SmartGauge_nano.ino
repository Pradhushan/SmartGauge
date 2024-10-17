#include <SoftwareSerial.h>
SoftwareSerial sim800l(3, 2);  // TX, RX of SIM800L

bool isConnected = false;  // Flag for connection status

unsigned long hour = 3;
unsigned long restartInterval = hour * 60 * 60 * 1000;  // Restart interval in milliseconds
unsigned long lastRestartTime = 0;                // Time of the last restart

//Ultrasonic sensor data
#define trigPin 4               // Trig pin of ultrasonic sensor
#define echoPin 5               // Echo pin of ultrasonic sensor
const int samplePeriod = 100;   // timing of 1 sample value
const int dataPeriod = 20000;   // data to collect within last 20 seconds
const int acceptableMin = 2;    // Minimum reading (cm)
const int acceptableMax = 250;  // Maximum reading (cm)

unsigned long lastSampleTime = 0;
unsigned long currentTime;
const int maxValues = dataPeriod / samplePeriod;  // Maximum number of values to store for 10 seconds
int values[maxValues];
int valuesCount = 0;
int currentIndex = 0;
const int NO_VALID_DATA = -1;  // Special value for no valid data



void setup() {
  pinMode(trigPin, OUTPUT);  // Trigger pin will output pulses (sound wave signals)
  pinMode(echoPin, INPUT);   // Echo pin will receive those pulses (sound wave signals)
  Serial.begin(9600);
  sim800l.begin(9600);

  sim800l.println("AT");  // Send AT command to check communication
  delay(1000);

  // Set SIM800L to text mode for SMS handling
  sim800l.println("AT+CMGF=1");
  delay(1000);

  // Enable auto-forward of received SMS to Arduino serial
  sim800l.println("AT+CNMI=2,2,0,0,0");
  delay(1000);

  Serial.println("Setup complete");
}

/***********************************/



/******************************************/


void loop() {
     // Check if it's time to restart
  if (millis() - lastRestartTime >= restartInterval){
    restart();
  } 


  // Check if the SIM800L module is connected to the network
  if (!isConnected) {
    // Try to connect to the network
    if (connectToNetwork()) {
      isConnected = true;
      Serial.println("Connected to network");
      configureSMSMode();  // Configure SMS mode after connecting to the network
    } else {
      Serial.println("Failed to connect to network");
      delay(5000);  // Wait for 5 seconds before retrying
      return;
    }
  }

  // Ultrasound measurements

  int approxDist = measure();
  return approxDist;

  int gap = 10;
  int heighttank = 240;
//  int radius = 595;
  int heightwater = heighttank - (approxDist - gap);
  int feet = heightwater/30.48;
  float percentage = 100 * heightwater / heighttank;    //percentage filled
  delay(50);

  // Check if SMS is being received
  if (sim800l.available()) {
    String sms = sim800l.readString();
    Serial.println(sms);

    // Check for Specific character within SMS
    if (sms.indexOf("?") != -1) {
      // Extract the sender's phone number
      String senderNumber = parseSenderNumber(sms);
      Serial.print("Sender's Number: ");
      Serial.println(senderNumber);

      // Check if the sender's number is not empty
      if (senderNumber.length() > 0) {
        // Send a reply SMS to the sender's number
        sim800l.println("AT+CMGS=\"" + senderNumber + "\"");
        delay(1000);
        sim800l.print(feet);
        sim800l.println(" ft");
        sim800l.print("(");
        sim800l.print(percentage);
        sim800l.print(" %");
        sim800l.println(")");
        delay(1000);
        sim800l.write(26);
        delay(1000);
      }
    }
  }

  // Check every 5 secs whether the module is connected
  if (millis() % 5000 == 0) {
    if (!isConnected) {  // if not connected
      Serial.println("Reconnecting to network");
      restart();  // Restart the Arduino
    }
  }
}
