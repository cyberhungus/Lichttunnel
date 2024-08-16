#include <FastLED.h>
#include <WiFi.h>
#include <WebServer.h>



// Hier die Netzwerkkonfiguration vornehmen
const char *ssid = "Illustratio-Lichttunnel";
const char *password = "123456789";

#define NUM_STRIPS 3
#define NUM_LEDS_PER_STRIP 60
#define NUM_LEDS NUM_LEDS_PER_STRIP * NUM_STRIPS

CRGB leds[NUM_STRIPS * NUM_LEDS_PER_STRIP];

//Change these to alter where the sensors are connected
#define forwardSensor A0
#define backwardSensor A1
//CHange these to alter where the buttons are connected
#define fasterButton A2
#define slowerButton A3

//lightPins contains the lights in order
int lightPins[] = {12, 11, 10, 9, 8};
//hold how many lights are connected
int lightCount = NUM_STRIPS;

//holds which light is active
int activeLight = 0;

//how long until the light switches from one light to the next
int delayPerLight = 200;

const int maxLightDelay = 2000;
const int minLightDelay = 200;

bool isForward = false;
bool isBackward = false;

long lastReactionTime;
long nextReactionTime;

//erstelle Server Objekt
WebServer server(80);

void setup() {
  Serial.begin(115200);
  Serial.println("Erstelle Netzwerk");
  WiFi.softAP(ssid, password);
  //Printet die IP des Servers
  Serial.println("Netzwerk IP addresse: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", "PLACEHOLDER"); // Send HTTP status 200 (OK) and type of content
  });

  server.on("/forward", HTTP_GET, []() {
    server.send(200, "text/html", "PLACEHOLDER for Forward"); // Send HTTP status 200 (OK) and type of content
  });

  server.on("/backward", HTTP_GET, []() {
    server.send(200, "text/html", "PLACEHOLDER for backward"); // Send HTTP status 200 (OK) and type of content
  });
  // tell FastLED there's 60 NEOPIXEL leds on pin 2, starting at index 0 in the led array
  FastLED.addLeds<NEOPIXEL, 2>(leds, 0, NUM_LEDS_PER_STRIP);

  // tell FastLED there's 60 NEOPIXEL leds on pin 3, starting at index 60 in the led array
  FastLED.addLeds<NEOPIXEL, 3>(leds, NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);

  // tell FastLED there's 60 NEOPIXEL leds on pin 4, starting at index 120 in the led array
  FastLED.addLeds<NEOPIXEL, 4>(leds, 2 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);

  //NO LONGER NEEDED, KEPT FOR ROLLBACKS
  //  for (int i = 0; i < lightCount; i++) {
  //    pinMode(lightPins[i], OUTPUT);
  //    digitalWrite(lightPins[i],HIGH);
  //    delay(100);
  //    digitalWrite(lightPins[i],LOW);
  //  }


  pinMode(forwardSensor, INPUT_PULLUP);
  pinMode(backwardSensor, INPUT_PULLUP);

  pinMode(fasterButton, INPUT_PULLUP);
  pinMode(slowerButton, INPUT_PULLUP);

  Serial.println("System Start");
}

void loop() {

  //server Funktionalität, vllt auf 2. Kern auslagern damit das Problemlos läuft
  server.handleClient();


  //read forward sensor
  int result = digitalRead(forwardSensor);
  if (result == LOW && isForward == false && isBackward == false) {
    isForward = true;
    activeLight = 0;
    nextReactionTime = millis() + delayPerLight;
    //digitalWrite(lightPins[activeLight], HIGH);
    turnLightStripOn(activeLight, 200);

    Serial.println("Forward Start ");
  }

  result = digitalRead(backwardSensor);
  if (result == LOW && isForward == false && isBackward == false) {
    isBackward = true;
    activeLight = lightCount - 1;
    nextReactionTime = millis() + delayPerLight;
    //digitalWrite(lightPins[activeLight], HIGH);
    turnLightStripOn(activeLight, 200);

    Serial.println("Backward Start ");
  }

  result = digitalRead(fasterButton);
  if (result == LOW) {

    delayPerLight += 100;
    if (delayPerLight > maxLightDelay) {
      delayPerLight = maxLightDelay;
    }
    Serial.print("Faster Pressed: ");
    Serial.print(delayPerLight);
    Serial.println(" ms");
    delay(300);
  }

  result = digitalRead(slowerButton);
  if (result == LOW) {
    delayPerLight -= 100;
    if (delayPerLight < minLightDelay) {
      delayPerLight = minLightDelay;
    }
    Serial.print("Slower Pressed: ");
    Serial.print(delayPerLight);
    Serial.println(" ms");
    delay(300);
  }

  if (millis() > nextReactionTime) {
    if (isForward) {
      //turn off condition
      if (activeLight == lightCount - 1) {
        isForward = false;
        // digitalWrite(lightPins[activeLight], LOW);
        turnLightStripOn(activeLight, 0);

        Serial.println("forward ended");
      }
      //iteration condition
      else {
        // digitalWrite(lightPins[activeLight], LOW);
        turnLightStripOn(activeLight, 0);

        activeLight++;
        //digitalWrite(lightPins[activeLight], HIGH);
        turnLightStripOn(activeLight, 200);

        nextReactionTime = millis() + delayPerLight;
        Serial.print("Forward change: ");
        Serial.print(lightPins[activeLight]);
        Serial.println(" is on");
      }


    }
    else if (isBackward) {
      //turn off condition
      if (activeLight == 0) {
        isBackward = false;
        //  digitalWrite(lightPins[activeLight], LOW);
        turnLightStripOn(activeLight, 0);

        Serial.println("backward ended");
      }
      //iteration condition
      else {
        //digitalWrite(lightPins[activeLight], LOW);
        turnLightStripOn(activeLight, 0);

        activeLight--;
        //digitalWrite(lightPins[activeLight], HIGH);
        turnLightStripOn(activeLight, 200);

        nextReactionTime = millis() + delayPerLight;
      }
    }
  }
}

void turnLightStripOn(int stripNumber, int intensity) {

  for (int i = stripNumber * NUM_LEDS_PER_STRIP; i < (stripNumber * NUM_LEDS_PER_STRIP) + NUM_LEDS_PER_STRIP; i++) {
    leds[i] = CRGB(intensity, intensity, intensity);
    FastLED.show();

  }
}
