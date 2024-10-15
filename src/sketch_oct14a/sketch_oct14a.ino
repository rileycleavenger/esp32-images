#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>

#define NUM_PIXELS 64  // 8x8 Matrix
#define PIN 5         // Pin where NeoPixel is connected

// Your WiFi credentials
const char* ssid = "TylersCox";
const char* password = "ilovepizza";

// NeoPixel matrix setup
Adafruit_NeoPixel matrix = Adafruit_NeoPixel(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Web server on port 80
WebServer server(80);

// Buffer to hold uploaded image data (8x8 matrix = 64 pixels, 3 bytes per pixel for RGB)
uint8_t pixelData[NUM_PIXELS * 3];
int currentPixel = 0;

void handleRoot() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  String htmlPage = "<form method='POST' action='/upload' enctype='multipart/form-data'>";
  htmlPage += "<input type='file' name='image'>";
  htmlPage += "<input type='submit' value='Upload Image'>";
  htmlPage += "</form>";
  server.send(200, "text/html", htmlPage);
}

void processImageData() {
  // Process the pixel data and display it on the NeoPixel matrix
  for (int i = 0; i < NUM_PIXELS; i++) {
    int r = pixelData[i * 3];     // Red value
    int g = pixelData[i * 3 + 1]; // Green value
    int b = pixelData[i * 3 + 2]; // Blue value
    matrix.setPixelColor(i, matrix.Color(r, g, b)); // Set the pixel color
  }
  matrix.show(); // Display the image
}

void handleImageUpload() {
  HTTPUpload& upload = server.upload();
  
  if (upload.status == UPLOAD_FILE_START) {
    Serial.printf("Upload File Start: %s\n", upload.filename.c_str());
    currentPixel = 0; // Reset pixel counter
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    Serial.printf("Upload File Write: %u bytes\n", upload.currentSize);
    // Process each byte of the uploaded file (3 bytes per pixel for RGB)
    for (size_t i = 0; i < upload.currentSize && currentPixel < NUM_PIXELS * 3; i++) {
      pixelData[currentPixel++] = upload.buf[i]; // Store RGB data
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    Serial.printf("Upload File End: %u bytes\n", upload.totalSize);
    server.send(200, "text/plain", "Image received and displayed!");

    // Process and display the image after upload is complete
    processImageData();
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize NeoPixel matrix
  matrix.begin();
  matrix.show();  // Initialize all pixels to 'off'

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi.");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  // Print the IP address
  Serial.println(WiFi.localIP());

  // Setup web server
  server.on("/", HTTP_GET, handleRoot);
  server.on("/upload", HTTP_POST, []() {
    server.send(200);
  }, handleImageUpload);

  server.begin();
}

void loop() {
  // Handle client requests
  server.handleClient();
}
