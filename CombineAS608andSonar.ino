#include <Adafruit_Fingerprint.h>
#include <Servo.h>

Servo myServo;  // Create a servo object
const int trigPin = 9;
const int echoPin = 8;

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial1);

long duration;
int distance;
bool authorized = false; // Flag to check if the user is authorized
bool stopWork = false;   // Flag to check if stop has been triggered

void setup() {
  myServo.attach(11);  // Attach the servo on pin 11
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Serial.begin(250000);
  Serial1.begin(57600); // Baud rate for AS608
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Fingerprint sensor detected!");
  } else {
    Serial.println("Fingerprint sensor not detected or password mismatch :(");
    while (1) { delay(1000); }
  }
}

void loop() {
  if (Serial.available()) {
    String userInput = Serial.readStringUntil('\n');
    userInput.trim();  // Remove any whitespace characters

    if (userInput == "e") {
      enrollFingerprint(); // Start enrolling a new fingerprint
    }
    else if (userInput == "stop") {
      stopWork = true;   // Set the stop flag to true
      Serial.println("System stopped. Type 'start' to resume.");
    }
    else if (userInput == "start") {
      stopWork = false;  // Reset the stop flag
      Serial.println("System resumed.");
    }
    else if (userInput == "c") {
      clearFingerprintData();  // Clear all fingerprint data
    }
    else if (userInput == "F") {
      listFingerprintIDs();  // List all enrolled fingerprint IDs
    }
  }

  if (!authorized && !stopWork) {
    Serial.println("Please scan your fingerprint...");
    if (getFingerprintID() == FINGERPRINT_OK) {
      Serial.println("Fingerprint authorized!");
      authorized = true;
    } else {
      Serial.println("Fingerprint not recognized.");
      delay(2000);
    }
  }

  if (authorized && !stopWork) {
    Serial.println("Starting sonar detection...");
    // Servo sweep code remains the same
    for (int pos = 10; pos <= 160; pos += 1) {
      if (stopWork) break; // Check if stop was requested
      myServo.write(pos);
      delay(15);
      calculateDistance();

      if (distance >= 10 && distance <= 300) {
        Serial.print(pos);
        Serial.print(",");
        Serial.print(distance);
        Serial.println(".");
      }
    }

    for (int pos = 160; pos >= 10; pos -= 1) {
      if (stopWork) break; // Check if stop was requested
      myServo.write(pos);
      delay(15);
      calculateDistance();

      if (distance >= 10 && distance <= 300) {
        Serial.print(pos);
        Serial.print(",");
        Serial.print(distance);
        Serial.println(".");
      }
    }
  }
}

// Function to calculate distance using the HC-SR04 sensor
void calculateDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;
}

// Function to get fingerprint ID
int getFingerprintID() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return p;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return p;

  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.print("Found fingerprint ID #");
    Serial.print(finger.fingerID);
    Serial.print(" with confidence ");
    Serial.println(finger.confidence);
    return FINGERPRINT_OK;
  } else {
    return p;
  }
}

// Function to enroll a new fingerprint
void enrollFingerprint() {
  int id = 1;

  // Find the next available ID by checking each ID slot
  for (id = 1; id < 128; id++) {
    if (finger.loadModel(id) != FINGERPRINT_OK) {
      break;  // Use this ID if it's not already in use
    }
  }

  if (id == 128) {
    Serial.println("No available fingerprint slots.");
    return;  // Exit if all slots are full
  }

  Serial.print("Preparing to enroll fingerprint with ID "); 
  Serial.println(id);
  Serial.println("Place your finger on the sensor to start enrollment...");

  // Wait for the first finger press
  int p = waitForFingerPress();
  if (p != FINGERPRINT_OK) return;

  // Convert fingerprint to feature set
  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) {
    Serial.println("Failed to convert image to template.");
    return;
  }

  Serial.println("First image captured. Remove your finger.");
  delay(2000);  // Wait for the user to remove the finger

  // Wait for the second finger press
  Serial.println("Place the same finger again.");
  p = waitForFingerPress();
  if (p != FINGERPRINT_OK) return;

  // Convert fingerprint to feature set again
  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) {
    Serial.println("Failed to convert second image to template.");
    return;
  }

  // Create the model
  p = finger.createModel();
  if (p != FINGERPRINT_OK) {
    Serial.println("Failed to create fingerprint model.");
    return;
  }

  // Store the model in the database under the determined ID
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Fingerprint successfully enrolled!");
  } else {
    Serial.println("Failed to store fingerprint.");
  }
}

// Function to clear all fingerprint data
void clearFingerprintData() {
  if (finger.emptyDatabase() == FINGERPRINT_OK) {
    Serial.println("All fingerprint data cleared!");
  } else {
    Serial.println("Failed to clear fingerprint data.");
  }
}

// Function to list all enrolled fingerprint IDs
void listFingerprintIDs() {
  uint8_t p;
  bool hasFingerprints = false;

  Serial.println("Checking all fingerprint slots (ID 1-127)...");
  for (int id = 1; id < 128; id++) {
    p = finger.loadModel(id);
    if (p == FINGERPRINT_OK) {
      Serial.print("Fingerprint ID: F"); 
      Serial.println(id);  // List each valid fingerprint ID
      hasFingerprints = true;
    }
  }

  if (!hasFingerprints) {
    Serial.println("No fingerprints enrolled.");
  }
}

// Helper function to wait for a finger press
int waitForFingerPress() {
  int p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    if (p == FINGERPRINT_NOFINGER) {
      Serial.print(".");
      delay(500); // Small delay to prevent flooding the serial output
    } else if (p == FINGERPRINT_OK) {
      Serial.println("Finger detected.");
    } else {
      Serial.println("Unknown error. Try again.");
    }
  }
  return p;
}
