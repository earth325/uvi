#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <limits.h>

// Replace with your network credentials and API key
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
const String apiKey = "openweatherapi";
const String location = "lat= x & lon=y";  // Replace with your coordinates

// UV index threshold
const float uvThreshold = 6.0;

const int ledPin = 5;

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Get UV index and control LED
  getMaxUVIndex();
}

void loop() {
  // Update UV index every day
  delay(900000);
  getMaxUVIndex();
}

void getMaxUVIndex() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "openweatherlocationurl";
    http.begin(url);
    int httpCode = http.GET();
    Serial.println(httpCode);

    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println("API Response: ");
      Serial.println(payload);
      
      float maxUVIndex = parseMaxUVIndex(payload);
      Serial.print("Max UV Index: ");
      Serial.println(maxUVIndex);

      if (maxUVIndex > uvThreshold) {
        Serial.println("UV Index is above the threshold. Turning on LED.");
        digitalWrite(ledPin, HIGH); // Turn on LED
      } else {
        Serial.println("UV Index is below the threshold. Turning off LED.");
        digitalWrite(ledPin, LOW); // Turn off LED
      }
    } else {
      Serial.println("Error on HTTP request");
    }

    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}

float parseMaxUVIndex(String payload) {
  // Parse the JSON response to find the maximum UV index
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, payload);

  float maxUVIndex = 0;
  long currentTime = LONG_MAX - 53200;

  JsonArray hourly = doc["hourly"];

  for (JsonObject hour : hourly) {
    long dt = hour["dt"];
    if (currentTime + 43200 < dt) {
      break;
    }

    if (currentTime == LONG_MAX - 53200) {
      currentTime = dt;
    }

    float uvIndex = hour["uvi"];
    if (uvIndex > maxUVIndex) {
      maxUVIndex = uvIndex;
    }
  }

  return maxUVIndex;
}