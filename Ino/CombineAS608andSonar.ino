#include <Adafruit_Fingerprint.h>
#include <Servo.h>

Servo myServo;  // Create a servo object
const int trigPin = 9;
const int echoPin = 8;

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial1);  // Fingerprint sensor on Serial1 (Pins 18, 19)

long duration;
int distance;
bool authorized = false; // Flag to check if the user is authorized locally (by fingerprint sensor)
bool stopWork = false;   // Flag to check if stop has been triggered
bool serverAuthorized = false; // New variable to track server authorization from the server
bool previousAuthorizationStatus = false;  // Initial previous status is unauthorized
int servoPosition;  // Variable to store the current position of the servo



void setup() {
  myServo.attach(11);  // Attach the servo on pin 11
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Serial.begin(115200);  // Communication with the Serial Monitor
  Serial1.begin(57600);  // Communication with AS608 fingerprint sensor
  Serial2.begin(115200); // Communication with ESP8266 on Serial2 (Pins 16, 17)

  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Fingerprint sensor detected!");
  } else {
    Serial.println("Fingerprint sensor not detected or password mismatch :(");
    while (1) { 
      delay(1000); 
    }
  }
}

void loop() {
  // Handle Serial inputs for commands (start, stop, enroll)
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
    else if (userInput == "f") {
      listFingerprintIDs();  // List all enrolled fingerprint IDs
    }
  }

  // Check if the fingerprint is scanned and the system is not stopped
  if (!authorized && !stopWork) {
    Serial.println("Please scan your fingerprint...");
    if (getFingerprintID() == FINGERPRINT_OK) {
      Serial.println("Fingerprint authorized locally!");

      // Send the fingerprint ID to ESP8266 for server authorization
      Serial2.println(finger.fingerID);  // Send to ESP8266

      // Wait for the ESP8266 to respond with authorization result
      delay(500);  // Give time for response

      String response = "";  // Buffer to store incoming data
      while (Serial2.available()) {
        char incomingChar = Serial2.read();
        response += incomingChar;
        if (incomingChar == '\n') break;  // Stop reading when newline is detected
        delay(10);  // Small delay to ensure the message is fully received
      }

      response.trim();  // Trim any extra spaces or newline characters
      Serial.print("Response from ESP8266: ");
      Serial.println(response);

      if (response == "Authorized") {
        serverAuthorized = true;
        Serial.println("Server Authorized! Running sonar...");
        authorized = true;  // Mark as fully authorized

        // Send "Authorized" status to Processing
        Serial.println("AUTH:Authorized");

      } else if (response == "Unauthorized") {
        serverAuthorized = false;
        Serial.println("Server denied access!");
        authorized = false;

        // Send "Unauthorized" status to Processing
        Serial.println("AUTH:Unauthorized");

      } else {
        Serial.println("Incomplete or incorrect response from ESP8266.");
      }
    } else {
      Serial.println("Fingerprint not recognized.");
      delay(2000);
    }
  }

  // Only send the authorization status if it has changed
  if (authorized != previousAuthorizationStatus) {
    if (authorized) {
      Serial.println("authorized");
    } else {
      Serial.println("unauthorized");
    }
    previousAuthorizationStatus = authorized;  // Update the previous status
  }

  // Now, if the system is authorized and not stopped, run the sonar system
  if (authorized && !stopWork) {
    for (int angle = 0; angle <= 180; angle += 1) {
      myServo.write(angle);  // Move the servo to the current angle
      delay(30);  // Give the servo time to move to the position

      calculateDistance();  // Call your existing function to calculate the distance

      // Send the angle and distance to Processing in the format "angle:distance"
      Serial.print(angle);
      Serial.print(":");
      Serial.println(distance);

      // Small delay between sweeps
      delay(10);
    }

    // Sweep back from 180 to 0 degrees
    for (int angle = 180; angle >= 0; angle -= 1) {
      myServo.write(angle);  // Move the servo back to the previous position
      delay(30);  // Give the servo time to move

      calculateDistance();  // Recalculate the distance for the current angle

      // Send the angle and distance to Processing in the format "angle:distance"
      Serial.print(angle);
      Serial.print(":");
      Serial.println(distance);

      // Small delay between sweeps
      delay(10);
    }
  }
}




void calculateDistance() {
  // Clear the trigPin by setting it LOW
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  // Trigger the ultrasonic pulse by setting trigPin HIGH for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Measure the duration of the echo pulse (in microseconds)
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance in centimeters
  distance = (duration * 0.034) / 2;
  
  Serial.println(distance);  // Send only the distance value
}


// Function to run sonar system
void runSonar() {
  Serial.println("Starting continuous sonar detection...");

  while (!stopWork) {  // Keep sweeping as long as stopWork is false
    // Sweep from 10 to 160 degrees
    for (int pos = 10; pos <= 160; pos += 1) {
      if (stopWork) break;  // Check if stop was requested
      myServo.write(pos);
      delay(15);

      calculateDistance();  // For now, skip checking distance for simplicity
      Serial.print("Servo at position: ");
      Serial.println(pos);
    }

    // Sweep back from 160 to 10 degrees
    for (int pos = 160; pos >= 10; pos -= 1) {
      if (stopWork) break;  // Check if stop was requested
      myServo.write(pos);
      delay(15);

      calculateDistance();  // For now, skip checking distance for simplicity
      Serial.print("Servo at position: ");
      Serial.println(pos);
    }
  }

  Serial.println("Sonar stopped by stop command.");
}


// Function to get fingerprint ID
int getFingerprintID() {
  uint8_t p = finger.getImage();  // Get the fingerprint image
  if (p != FINGERPRINT_OK) return p;  // Return if there's an error getting the image

  p = finger.image2Tz();  // Convert the image to a fingerprint template
  if (p != FINGERPRINT_OK) return p;  // Return if there's an error converting

  p = finger.fingerFastSearch();  // Search for the fingerprint template in the database
  if (p == FINGERPRINT_OK) {
    Serial.print("Found fingerprint ID #");
    Serial.print(finger.fingerID);  // Print the fingerprint ID
    Serial.print(" with confidence ");
    Serial.println(finger.confidence);  // Print the confidence level of the match
    return FINGERPRINT_OK;
  } else {
    return p;  // Return error if no match is found
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

  // Loop through fingerprint slots and print the ID if a fingerprint is enrolled
  for (int id = 1; id < 128; id++) {
    p = finger.loadModel(id);
    if (p == FINGERPRINT_OK) {
      Serial.print("Fingerprint ID: ");
      Serial.println(id);  // Print each valid fingerprint ID
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
      delay(500);
    } else if (p == FINGERPRINT_OK) {
      Serial.println("Finger detected.");
    } else {
      Serial.println("Unknown error. Try again.");
    }
  }
  return p;
}

// Function to enroll a new fingerprint
void enrollFingerprint() {
  int id = 1;

  for (id = 1; id < 128; id++) {
    if (finger.loadModel(id) != FINGERPRINT_OK) {
      break;  // Use this ID if it's not already in use
    }
  }

  if (id == 128) {
    Serial.println("No available fingerprint slots.");
    return;
  }

  Serial.print("Preparing to enroll fingerprint with ID ");
  Serial.println(id);
  Serial.println("Place your finger on the sensor to start enrollment...");

  int p = waitForFingerPress();
  if (p != FINGERPRINT_OK) return;

  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) {
    Serial.println("Failed to convert image to template.");
    return;
  }

  Serial.println("First image captured. Remove your finger.");
  delay(2000);

  Serial.println("Place the same finger again.");
  p = waitForFingerPress();
  if (p != FINGERPRINT_OK) return;

  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) {
    Serial.println("Failed to convert second image to template.");
    return;
  }

  p = finger.createModel();
  if (p != FINGERPRINT_OK) {
    Serial.println("Failed to create fingerprint model.");
    return;
  }

  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Fingerprint successfully enrolled!");
  } else {
    Serial.println("Failed to store fingerprint.");
  }
}