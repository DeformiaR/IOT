import processing.serial.*;  // Import the Serial library

Serial myPort;  // Create object from Serial class
float distance = 0;  // Variable to store distance
float angle = 0;  // Variable to store the current angle of the radar sweep
boolean authorized = false;  // Variable to track authorization status


void setup() {
  size(1280, 720);  // Set the size of the window
  myPort = new Serial(this, Serial.list()[0], 115200);  // Initialize serial communication
  myPort.bufferUntil('\n');  // Buffer the serial input until newline
  background(0);  // Set background to black for radar screen
}

void draw() {
  // Set background to black, but keep previous lines for radar sweep trail
  fill(0, 10);  // Slightly transparent fill to create a fading trail effect
  rect(0, 0, width, height);  // Fill the whole window
  
  drawRadarGrid();  // Draw the radar grid and markers
  drawRadarSweep();  // Draw the sweeping radar line
  drawDistanceText();  // Display the current angle and distance
}

// Function to draw the radar grid with distance and angle markers
void drawRadarGrid() {
  translate(width / 2, height);  // Move origin to bottom center
  
  stroke(0, 255, 0);  // Set stroke color to green for the grid
  noFill();  // No fill for the grid circles

  // Draw distance circles (representing 60 cm, 120 cm, 180 cm, 240 cm)
  for (int i = 1; i <= 4; i++) {
    ellipse(0, 0, i * 150, i * 150);  // Circles with increasing radius
  }

  // Label the distance for each arc
  textSize(16);  // Set text size for the labels
  fill(0, 255, 0);  // Green text color
  
  for (int i = 1; i <= 4; i++) {
    String label = (i * 60) + " cm";  // Generate the label for each arc (60 cm, 120 cm, etc.)
    float labelX = (i * 150) / 2 * cos(radians(45));  // Calculate x position of the label (angle: 45°)
    float labelY = (i * 150) / 2 * sin(radians(45));  // Calculate y position of the label
    text(label, labelX - 20, -labelY);  // Draw the label near each arc
  }

  // Draw angle lines and markers (every 30 degrees)
  for (int i = 0; i <= 180; i += 30) {
    float x = 250 * cos(radians(i));  // Calculate the x position of the angle line
    float y = 250 * sin(radians(i));  // Calculate the y position of the angle line
    line(0, 0, x, -y);  // Draw angle line
    text(i + "°", x - 10, -y + 20);  // Label the angle
  }
}


// Function to draw the radar sweep
void drawRadarSweep() {
  stroke(0, 255, 0);  // Green stroke for the sweeping radar line
  float sweepX = 500 * cos(radians(angle));  // Calculate x position of the sweeping line
  float sweepY = 500 * sin(radians(angle));  // Calculate y position of the sweeping line
  line(0, 0, sweepX, -sweepY);  // Draw the sweeping line
  
  // Draw the object detected line based on distance
  float objectX = distance * 2.5 * cos(radians(angle));  // Scale the distance for visualization
  float objectY = distance * 2.5 * sin(radians(angle));  // Scale the distance for visualization
  stroke(255, 0, 0);  // Set stroke color to red for detected object
  line(0, 0, objectX, -objectY);  // Draw the object line
}

// Function to display distance and angle information
void drawDistanceText() {
  fill(0, 255, 0);  // Set text color to green
  textSize(20);  // Set text size
  
  // Display angle and distance at the bottom-left corner
  text("Angle: " + int(angle), -width / 2 + 10, -height + 30);  // Display the angle
  text("Distance: " + distance + " cm", -width / 2 + 10, -height + 60);  // Display the distance
  
  // Display authorization status in the top-right corner
  textAlign(RIGHT);  // Align the text to the right
  String status = authorized ? "Authorized" : "Unauthorized";  // Determine the status message
  fill(authorized ? color(0, 255, 0) : color(255, 0, 0));  // Green if authorized, red if unauthorized
  text("Authorization: " + status, width / 2 - 10, -height + 30);  // Display the authorization status
  
  // Reset text alignment to default (left) for other text
  textAlign(LEFT);
}



// Function that gets called whenever serial data is available
void serialEvent(Serial myPort) {
  String data = myPort.readStringUntil('\n');  // Read the serial data until a newline character
  if (data != null) {
    data = trim(data);  // Remove any leading/trailing whitespace
    println("Received data: " + data);  // Print the received data to the Processing console

    // Check if the message is about authorization
    if (data.startsWith("AUTH:")) {
      String status = data.substring(5);  // Extract the status after "AUTH:"
      if (status.equals("Authorized")) {
        authorized = true;  // Set authorized to true
      } else if (status.equals("Unauthorized")) {
        authorized = false;  // Set authorized to false
      }
    } else {
      // Split the incoming data into angle and distance (existing functionality)
      String[] values = split(data, ':');
      if (values.length == 2) {
        try {
          angle = float(values[0]);  // Convert the angle to a float
          distance = float(values[1]);  // Convert the distance to a float
        } catch (Exception e) {
          println("Error parsing data: " + data);  // Catch any errors while parsing
        }
      }
    }
  }
}
