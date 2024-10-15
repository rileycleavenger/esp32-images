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

// Handle image upload
void handleImageUpload(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
  Serial.println("entered handle image upload func");
  Serial.println("Index: " + String(index));
  if (index == 0) {
    Serial.println("Upload started");
  }
  
  // Accumulate the data into pixelData buffer
  for (size_t i = 0; i < len; i++) {
    if (index + i < sizeof(pixelData)) {
      pixelData[index + i] = data[i];
    }
  }

  if (final) {
    Serial.println("Upload complete, displaying image");
    
    // Process and display the image
    for (int i = 0; i < NUM_PIXELS; i++) {
      int r = pixelData[i * 3];
      int g = pixelData[i * 3 + 1];
      int b = pixelData[i * 3 + 2];
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
    // Print all parameters
    int params = request->params();
    Serial.println("num of params: " + params);
    for(int i=0;i<params;i++){
        AsyncWebParameter* p = request->getParam(i);
        Serial.print("Param name: ");
        Serial.println(p->name());
        Serial.print("Param value: ");
        Serial.println(p->value());
    }

    // Read the request body
    if (request->hasParam("body", true)) {
      Serial.println("POST req has body");
      AsyncWebParameter* p = request->getParam("body", true);
      const uint8_t* data = (const uint8_t*)p->value().c_str();
      size_t len = p->value().length();

      // Call handleImageUpload with the accumulated data
      handleImageUpload(request, "upload", 0, (uint8_t*)data, len, true);
    }

    // Send response
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Upload endpoint");
    sendCORSHeaders(response);
    request->send(response);
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