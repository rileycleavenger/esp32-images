#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
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
AsyncWebServer server(80);

// Buffer to hold uploaded image data (8x8 matrix = 64 pixels, 3 bytes per pixel for RGB)
uint8_t pixelData[NUM_PIXELS * 3];

// Function to send CORS headers
void sendCORSHeaders(AsyncWebServerResponse *response) {
  response->addHeader("Access-Control-Allow-Origin", "*");
  response->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  response->addHeader("Access-Control-Allow-Headers", "Content-Type");
}

// Handle OPTIONS (CORS preflight) requests
void handlePreflight(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response = request->beginResponse(200);
  sendCORSHeaders(response);
  request->send(response);
}

// Convert comma-separated string to uint8_t array
bool convertToUint8Array(const String& input, uint8_t* output, size_t outputSize) {
  int index = 0;
  int start = 0;
  for (int i = 0; i < input.length(); i++) {
    if (input[i] == ',' || i == input.length() - 1) {
      if (i == input.length() - 1) {
        i++;
      }
      String number = input.substring(start, i);
      output[index++] = number.toInt();
      start = i + 1;
      if (index >= outputSize) {
        return true; // Successfully converted
      }
    }
  }
  return false; // Conversion failed
}

// Adjust brightness of pixel data
void adjustBrightness(uint8_t* data, size_t length, float brightness) {
  for (size_t i = 0; i < length; i++) {
    data[i] = static_cast<uint8_t>(data[i] * brightness);
  }
}

// Handle image upload
void handleImageUpload(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
  // Print the data as hexadecimal values
  // Serial.print("Data: ");
  // for (size_t i = 0; i < len; i++) {
  //   Serial.print(data[i], HEX);
  //   Serial.print(" ");
  // }
  // Serial.println();

  // // Print the length and index
  // Serial.println("Len: " + String(len));
  // Serial.println("Index: " + String(index));

  // if (index == 0) {
  //   Serial.println("Upload started");
  // }
  
  // Accumulate the data into pixelData buffer
  for (size_t i = 0; i < len; i++) {
    if (index + i < sizeof(pixelData)) {
      pixelData[index + i] = data[i];
    } else {
      Serial.println("Error: Buffer overflow detected!");
      return;
    }
  }

  if (final) {
    Serial.println("Upload complete, displaying image");

    // Adjust brightness
    adjustBrightness(pixelData, sizeof(pixelData), 0.5); // Adjust brightness to 50%

    // Process and display the image
    for (int i = 0; i < NUM_PIXELS; i++) {
      int r = pixelData[i * 3];
      int g = pixelData[i * 3 + 1];
      int b = pixelData[i * 3 + 2];
      // Serial.printf("Pixel %d: R=%d, G=%d, B=%d\n", i, r, g, b); // Debugging statement
      matrix.setPixelColor(i, matrix.Color(r, g, b));
    }
    matrix.show();
    
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Image uploaded and displayed!");
    sendCORSHeaders(response);
    request->send(response);
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize NeoPixel matrix
  matrix.begin();
  matrix.show();  // Initialize all pixels to 'off'

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.println(WiFi.localIP());

  // Setup web server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Hello, this is the ESP32 server");
    sendCORSHeaders(response);
    request->send(response);
  });

  // Handle image upload with POST
  server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request){
    Serial.println("POST request received");
    Serial.print("content len: ");
    Serial.println(request->contentLength());

    if (request->hasParam("pixelData", true)) {
      String encodedData = request->getParam("pixelData", true)->value();
      uint8_t data[NUM_PIXELS * 3];
      if (convertToUint8Array(encodedData, data, sizeof(data))) {
        handleImageUpload(request, "upload", 0, data, sizeof(data), true);
      } else {
        Serial.println("Error: Failed to convert pixel data");
        AsyncWebServerResponse *response = request->beginResponse(400, "text/plain", "Invalid pixel data");
        sendCORSHeaders(response);
        request->send(response);
      }
    } else {
      Serial.println("Error: No pixel data found");
      AsyncWebServerResponse *response = request->beginResponse(400, "text/plain", "No pixel data found");
      sendCORSHeaders(response);
      request->send(response);
    }
  });

  // Handle CORS preflight request
  server.on("/upload", HTTP_OPTIONS, handlePreflight);

  server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    Serial.println("Body received");
  });

  // Start server
  server.begin();
}

void loop() {
  // No need for code in loop() for the async server
}