#include <WiFi.h>

const char* ssid = "TylersCox";
const char* password = "ilovepizza";

void setup() {
  Serial.begin(115200);  // Start serial communication

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("Connected!");
  
  // Print the ESP32's local IP address
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Your loop code here
}
