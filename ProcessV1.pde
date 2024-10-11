import processing.serial.*;  // Import the Serial library

Serial myPort; // defines Object Serial
String angle = "";
String distance = "";
String data = "";
int iAngle, iDistance;
int index1 = 0;
int maxDistance = 0; // New variable to store the maximum detected distance

boolean systemStopped = false; // Flag to check if the system is stopped

void setup() {
  size(1366, 768); 
  smooth();
  myPort = new Serial(this, "COM5", 250000); // starts the serial communication
  myPort.bufferUntil('.'); // reads the data from the serial port up to the character '.'

  // UI instructions
  println("Commands:");
  println("'e' to enroll a fingerprint.");
  println("'c' to clear fingerprint data.");
  println("'F' to list all fingerprint IDs.");
  println("'p' to stop the system.");
  println("'L' to resume the system.");
}

void draw() {
  // Clear radar if the system is stopped
  if (systemStopped) {
    fill(0); // Black screen to show system is stopped
    rect(0, 0, width, height);
    fill(255, 0, 0); // Red text for alert
    textSize(32);
    textAlign(CENTER, CENTER);
    text("System is stopped. Type 'L' to resume.", width / 2, height / 2);
    return;
  }

  // Create a fading effect by overlaying a semi-transparent rectangle
  fill(0, 10); // Set the transparency level
  noStroke();
  rect(0, 0, width, height); // Draw a transparent rectangle over the entire radar screen
  
  fill(98, 245, 31); // Green color for radar elements

  // Draw radar components (arcs, angle lines)
  drawRadar(); 
  
  // Draw the moving detection line (leave it on screen)
  drawLine();
  
  // Draw object detection (if any)
  drawObject();
  
  // Draw text for angle and distance
  drawText();
}

void serialEvent(Serial myPort) { 
  if (myPort.available() > 0) {
    data = myPort.readStringUntil('.');
    if (data != null && data.length() > 0) {
      data = data.trim();  // Clean up the data
      index1 = data.indexOf(",");
      if (index1 > -1) {
        angle = data.substring(0, index1);
        distance = data.substring(index1 + 1);
        
        // Convert to integers
        iAngle = int(angle);
        iDistance = int(distance);
        
        // Update the maximum detected distance
        if (iDistance > maxDistance) {
          maxDistance = iDistance;
        }
      }
    }
  }
}


void drawRadar() {
  pushMatrix();
  translate(width / 2, height - height * 0.074); // move to center bottom
  noFill();
  stroke(98, 245, 31);
  strokeWeight(2);

  // Adjusted radar arcs for 0.2m to 3m range (0.2m = 20 cm to 300 cm)
  arc(0, 0, 1000, 1000, PI, TWO_PI);  // largest arc for 300 cm
  arc(0, 0, 800, 800, PI, TWO_PI);    // arc for 240 cm
  arc(0, 0, 600, 600, PI, TWO_PI);    // arc for 180 cm
  arc(0, 0, 400, 400, PI, TWO_PI);    // arc for 120 cm
  arc(0, 0, 200, 200, PI, TWO_PI);    // smallest arc for 60 cm

  // Angle lines (ensuring all important lines are drawn)
  line(-500, 0, 500, 0);  // straight horizontal line (180 degrees)
  line(0, 0, -500 * cos(radians(30)), -500 * sin(radians(30))); // left 30 degrees
  line(0, 0, -500 * cos(radians(60)), -500 * sin(radians(60))); // left 60 degrees
  line(0, 0, -500 * cos(radians(90)), -500 * sin(radians(90))); // left 90 degrees (straight up)
  line(0, 0, -500 * cos(radians(120)), -500 * sin(radians(120))); // left 120 degrees
  line(0, 0, -500 * cos(radians(150)), -500 * sin(radians(150))); // left 150 degrees

  // Adding missing right-side lines
  line(0, 0, 500 * cos(radians(30)), -500 * sin(radians(30))); // right 30 degrees
  line(0, 0, 500 * cos(radians(60)), -500 * sin(radians(60))); // right 60 degrees
  line(0, 0, 500 * cos(radians(90)), -500 * sin(radians(90))); // right 90 degrees (straight up)
  line(0, 0, 500 * cos(radians(120)), -500 * sin(radians(120))); // right 120 degrees
  line(0, 0, 500 * cos(radians(150)), -500 * sin(radians(150))); // right 150 degrees
  line(0, 0, 500 * cos(radians(0)), -500 * sin(radians(0)));     // straight vertical line at 0 degrees (bottom center)

  popMatrix();

  // Adding labels along the horizontal line at the bottom
  fill(255);  // Set label color to white
  textSize(20);  // Set text size

  // Position the labels along the horizontal line
  text("300 cm", width / 2 + 490, height - height * 0.08); // Label for 300 cm
  text("240 cm", width / 2 + 380, height - height * 0.08); // Label for 240 cm
  text("180 cm", width / 2 + 270, height - height * 0.08); // Label for 180 cm
  text("120 cm", width / 2 + 160, height - height * 0.08); // Label for 120 cm
  text("60 cm",  width / 2 + 50, height - height * 0.08);  // Label for 60 cm
}

void drawObject() {
  // Modify to only display objects within the new range (10 cm to 300 cm)
  if (iDistance >= 10 && iDistance <= 300) {
    pushMatrix();
    translate(width / 2, height - height * 0.074);

    // Calculate the distance in pixels for drawing
    float pixsDistance = map(iDistance, 10, 300, 67, 1000);  // Adjust to map from 10 cm to 300 cm
    pixsDistance = constrain(pixsDistance, 67, 1000); // Ensure distance doesn't exceed radar bounds

    // Draw the undetected range (green) before the object detection
    stroke(98, 245, 31); // Green color for undetected range
    strokeWeight(9);
    line(0, 0, pixsDistance * cos(radians(iAngle)), -pixsDistance * sin(radians(iAngle))); 

    // Draw the detected range (red) from the object onward
    stroke(255, 10, 10); // Red color for detected object
    line(pixsDistance * cos(radians(iAngle)), -pixsDistance * sin(radians(iAngle)), 1000 * cos(radians(iAngle)), -1000 * sin(radians(iAngle)));
    
    popMatrix();
  }
}

void drawLine() {
  pushMatrix();
  translate(width / 2, height - height * 0.074);
  stroke(30, 250, 60); // Green radar line
  strokeWeight(9);
  
  // Draw the current detection line based on the current angle
  line(0, 0, 500 * cos(radians(iAngle)), -500 * sin(radians(iAngle))); // Radar sweep
  popMatrix();
}

void drawText() {
  fill(255); // Set text color to white
  textSize(20);

  // Add the furthest distance text to the top-left
  String furthestText = "Furthest Distance detected: " + maxDistance + " cm";
  text(furthestText, 10, 20); // Display text at (10, 20)

  // Existing angle and distance display
  text("Angle: " + iAngle + "°", 10, 50); // Position for angle
  text("Distance: " + iDistance + " cm", 10, 70); // Position for current distance
}

void keyPressed() {
  // Handle key inputs to send commands to Arduino
  if (key == 'e') {
    myPort.write('e');  // Send 'e' to enroll fingerprint
  }
  else if (key == 'c') {
    myPort.write('c');  // Send 'c' to clear all fingerprint data
  }
  else if (key == 'F') {
    myPort.write('F');  // Send 'F' to list fingerprint IDs
  }
  else if (key == 'p') {
    myPort.write("stop\n");  // Send 'p' command to stop the system
    systemStopped = true;    // Set the systemStopped flag to true
  }
  else if (key == 'L') {
    myPort.write("start\n");  // Send 'L' command to resume the system
    systemStopped = false;    // Reset the systemStopped flag
  }
}
