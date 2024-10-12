#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// Replace with your Wi-Fi credentials
const char* ssid = "Railkuni_2.4G";
const char* password = "RK0947503645";

// Replace with the URL of your PHP script
const char* serverUrl = "http://192.168.1.109/sensordataupdate.php";


// Create a WiFiClient object
WiFiClient client;

void setup() {
  Serial.begin(115200);        // Start Serial communication with Mega
  WiFi.begin(ssid, password);  // Connect to WiFi

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi.");
}

void loop() {
  // Check if the Mega sent a fingerprint ID via Serial
  if (Serial.available()) {
    String fingerprintID = Serial.readStringUntil('\n');  // Read until newline character

    // Send the fingerprint ID to the server for verification
    sendFingerprintID(fingerprintID.toInt());
  }
  delay(100);  // Add a small delay
}

void sendFingerprintID(int fingerprintID) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(serverUrl) + "?fingerprintID=" + String(fingerprintID);

    // Send request
    http.begin(client, url);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String response = http.getString();

      // Only send "Authorized" or "Unauthorized" to the Mega
      if (response == "authorized") {
        Serial.println("Authorized");  // Send only the result to Mega
      } else {
        Serial.println("Unauthorized");  // Send only the result to Mega
      }
      delay(100);  // Ensure Mega receives the message
    } else {
      Serial.println("Error sending request to server");
    }

    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}