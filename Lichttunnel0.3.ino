#include <FastLED.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>

// Hier die Netzwerkkonfiguration vornehmen
const char *ssid = "Illustratio-Lichttunnel";
const char *password = "illustratio";

#define NUM_STRIPS 5
#define NUM_LEDS_PER_STRIP 5
#define NUM_LEDS NUM_LEDS_PER_STRIP * NUM_STRIPS
const float FADE_VAR = 0.2;

CRGB leds[NUM_STRIPS * NUM_LEDS_PER_STRIP];

//hier wert von 1-4 angeben.
#define ESP_POSITION 2

int standardDelayPerLight = 200;

String ipBase = "http://192.168.4.";

//Change these to alter where the sensors are connected
#define forwardSensor 32
#define backwardSensor 33

int lightValues[5][3] = {
  {100, 200, 100},  // First entry with 3 numbers
  {200, 100, 100},  // Second entry
  {00, 100, 200},   // Third entry
  {00, 00, 200},  // Fourth entry
  {100, 00, 00} // Fifth entry
};


//hold how many lights are connected
int lightCount = NUM_STRIPS;

//holds which light is active
int activeLight = 0;

//how long until the light switches from one light to the next
int delayPerLight = 200;


//accelmode wird benutzt um verschiedene Animationen zu machen
int accelMode = 0;

const int maxLightDelay = 2000;
const int minLightDelay = 200;

bool isForward = false;
bool isBackward = false;

long lastReactionTime;
long nextReactionTime;

//erstelle Server Objekt
AsyncWebServer server(80);
IPAddress ip;

void callWebsite(String url) {

  if (WiFi.status() == WL_CONNECTED || ESP_POSITION == 1) {
    HTTPClient http;
    http.setConnectTimeout(30);
    http.begin(url); // Specify your URL here
    int httpCode = http.GET();
    // Check the returning code
    if (httpCode > 0) {
      String payload = http.getString();
      Serial.print("Connection to " + url + " served code: " );
      Serial.println(httpCode);

    } else {
      Serial.println("Error on HTTP request");
    }
    http.end(); //Free resources
  }
  else {
    Serial.println("Error Calling Website: " + url);
  }
}


//funktion die lichter auf einen weisswert stellt
void turnLightStripOn(int stripNumber, int intensity) {
  Serial.print("Turning Number: ");
  Serial.print(stripNumber);
  Serial.print(" to intensity: ");
  Serial.println(intensity);
  int startIndex = stripNumber * NUM_LEDS_PER_STRIP;
  int endIndex = startIndex + NUM_LEDS_PER_STRIP - 1;

  for (int start = startIndex, end = endIndex; start <= end; ++start, --end) {
    Serial.println(start);
    leds[start] = CRGB(intensity, intensity, intensity);
    if (start != end) {
      leds[end] = CRGB(intensity, intensity, intensity);
    }
    FastLED.show(); // Consider moving this outside of the loop if updating every LED individually isn't required
  }


}


//funktion die lichter auf einen im array lightValues angegebenen farbwert stellt
void turnLightStripPredefined(int stripNumber, float intensity  ) {
  Serial.print("Turning Number: ");
  Serial.print(stripNumber);
  Serial.print(" to predefined color: ");
  Serial.print(lightValues[stripNumber][0]);
  Serial.print(":");
  Serial.print(lightValues[stripNumber][1]);
  Serial.print(":");
  Serial.println(lightValues[stripNumber][2]);
  int startIndex = stripNumber * NUM_LEDS_PER_STRIP;
  int endIndex = startIndex + NUM_LEDS_PER_STRIP - 1;
  for (int start = startIndex, end = endIndex; start <= end; ++start, --end) {
    Serial.println(start);
    leds[start] = CRGB(int(lightValues[stripNumber][0] * intensity) , int(lightValues[stripNumber][1] * intensity), int(lightValues[stripNumber][2] * intensity));
    if (start != end) {
      leds[end] = CRGB(int(lightValues[stripNumber][0] * intensity) , int(lightValues[stripNumber][1] * intensity), int(lightValues[stripNumber][2] * intensity));
    }
    FastLED.show(); // Consider moving this outside of the loop if updating every LED individually isn't required
  }
}


//funktion die die farbe eines lightstrips ändert
void changeColorinArray(int arrayposition, int R, int G, int B) {
  Serial.print("Called Color Change in Array with pos: ");
  Serial.println(arrayposition);

  lightValues[arrayposition][0] = R;
  lightValues[arrayposition][1] = G;
  lightValues[arrayposition][2] = B;
  for (int i = 0; i < 5; i++) {
    Serial.print("Arrayposition: ");
    Serial.print(i);
    Serial.print("-r: ");
    Serial.print(lightValues[i][0]);
    Serial.print("-g: ");
    Serial.print(lightValues[i][1]);
    Serial.print("-b: ");
    Serial.println(lightValues[i][2]);

  }

  if (ESP_POSITION == 1) {
    turnLightStripPredefined(arrayposition, 1);
    delay(200);
    turnLightStripPredefined(arrayposition, 0);
  }

}

String MainPage();

void handleColorChange(String ip) {
  for (int i = 0; i < NUM_STRIPS; i++) {
    String url = ip + "/changeColor?lightnumber=" + String(i) + "&RVal=" + lightValues[i][0] + "&BVal=" + lightValues[i][2] + "&GVal=" + lightValues[i][1];
    callWebsite(url);
  }
}


void handleNetworkForward() {

  //nicht den nächsten strip anrufen wenn letzter strip
  if (ESP_POSITION == 4) {
    Serial.println("last device, stopping");
    return;
  }
  else {
    
    handleColorChange(ipBase + String(ESP_POSITION + 1));
    String url2 = ipBase + String(ESP_POSITION + 1) + "/changeSpeed?speed=" + delayPerLight;
    callWebsite(url2);
    String url = ipBase + String(ESP_POSITION + 1) + "/forward";
    Serial.println("Calling website:" + url);
    callWebsite(url);
  }
}

void handleAccelForward() {

  //nicht den nächsten strip anrufen wenn letzter strip
  if (ESP_POSITION == 4) {
    Serial.println("last device, stopping");
    return;
  }
  else {
    
    String url = ipBase + String(ESP_POSITION + 1) + "/changeAccelMode?accelmode="+String(accelMode);
    Serial.println("Calling website:" + url);
    callWebsite(url);
  }
}



void handleNetworkBackward() {

  //nicht den nächsten strip anrufen wenn letzter strip
  if (ESP_POSITION == 1) {
    Serial.println("first device, stopping");
    return;
  }
  else {
    handleColorChange(ipBase + String(ESP_POSITION - 1));
    String url2 = ipBase + String(ESP_POSITION - 1) + "/changeSpeed?speed=" + delayPerLight;
    callWebsite(url2);
    String url = ipBase + String(ESP_POSITION - 1) + "/backward";
    Serial.println("Calling website:" + url);
    callWebsite(url);
  }
}


int easeOutExpo(float x) {
  return int((x == 1.0) ? 1.0 : 1 - pow(2, -10 * x));
}

void handleNextReactionTime() {


  switch (accelMode) {
    case 0:
      nextReactionTime = millis() + delayPerLight;
      break;
    case 1:
      delayPerLight = delayPerLight-(delayPerLight/10);
      Serial.println("Accelmode with case 1-new delay is: " + String(delayPerLight));

      nextReactionTime = millis() + delayPerLight;
      break;
    case 2:
      delayPerLight = delayPerLight+(delayPerLight/10);
      Serial.println("Accelmode with case 1-new delay is: " + String(delayPerLight));

      nextReactionTime = millis() + delayPerLight;
      break;
    case 3:
      delayPerLight = delayPerLight-(delayPerLight/2);
      Serial.println("Accelmode with case 1-new delay is: " + String(delayPerLight));

      nextReactionTime = millis() + delayPerLight;
      break;
    case 4:
            delayPerLight = delayPerLight*2;
      Serial.println("Accelmode with case 1-new delay is: " + String(delayPerLight));

      nextReactionTime = millis() + delayPerLight;
      break;

         case 5:
            delayPerLight = delayPerLight/2;
      Serial.println("Accelmode with case 1-new delay is: " + String(delayPerLight));

      nextReactionTime = millis() + delayPerLight;
      break;
    default:
      // Tue etwas, im Defaultfall
      // Dieser Fall ist optional
      break; // Wird nicht benötigt, wenn Statement(s) vorhanden sind
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Device Number: " + String(ESP_POSITION));

  if (ESP_POSITION == 1) {
    Serial.println("Erstelle Netzwerk");
    WiFi.softAP(ssid, password);
    //Printet die IP des Servers
    Serial.println("Netzwerk IP addresse: ");
    Serial.println(WiFi.softAPIP());

  }
  else {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println("Verbindungsversuch...");
    }
    Serial.println("Netzwerkverbindung hergestellt");
    Serial.print("Local ESP32 IP: ");
    Serial.println(WiFi.localIP());
  }




  //wenn die index page (domainname/) besucht wird, gib folgendes zurück (siehe Webpages)
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/html", MainPage());
  });

  server.on("/forward", HTTP_GET, DOforward) ;
  server.on("/backward", HTTP_GET, DObackward);
  server.on("/changeColor", HTTP_GET, DOchangecolor);
  server.on("/changeSpeed", HTTP_GET, DOchangespeed);
  server.on("/changeAccelMode", HTTP_GET, DOchangeaccelmode);
  server.on("/api/color", HTTP_GET, DOgetcolor);


  //show error
  server.on("/error", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/html", ErrorPage());
  });


  //get the current speed var
  server.on("/api/speed", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/html", String(delayPerLight));
  });


  server.begin();
  //
  //  FastLED.addLeds<WS2811, 12, RGB>(leds, 0, NUM_LEDS_PER_STRIP);
  //  FastLED.addLeds<WS2811, 14, RGB>(leds, NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  //  FastLED.addLeds<WS2811, 27, RGB>(leds, 2 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  //  FastLED.addLeds<WS2811, 13, RGB>(leds, 3 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  //  FastLED.addLeds<WS2811, 26, RGB>(leds, 4 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);

  FastLED.addLeds<NEOPIXEL, 25>(leds, 0, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<NEOPIXEL, 26>(leds, NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<NEOPIXEL, 27>(leds, 2 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<NEOPIXEL, 15>(leds, 3 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<NEOPIXEL, 14>(leds, 4 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);

  pinMode(forwardSensor, INPUT_PULLUP);
  pinMode(backwardSensor, INPUT_PULLUP);

  Serial.println("System Start");
}



void loop() {


  //read forward sensor
  int result = digitalRead(forwardSensor);
  if (result == LOW && isForward == false && isBackward == false) {
    delayPerLight = standardDelayPerLight;
    isForward = true;
    activeLight = 0;
    //nextReactionTime = millis() + delayPerLight;
    handleNextReactionTime();
    //digitalWrite(lightPins[activeLight], HIGH);
    turnLightStripPredefined(activeLight,  1);

    Serial.println("Forward Start ");
  }

  result = digitalRead(backwardSensor);
  if (result == LOW && isForward == false && isBackward == false) {
    delayPerLight = standardDelayPerLight;
    isBackward = true;
    activeLight = lightCount - 1;
    //nextReactionTime = millis() + delayPerLight;
    handleNextReactionTime();
    //digitalWrite(lightPins[activeLight], HIGH);
    turnLightStripPredefined(activeLight,  1);

    Serial.println("Backward Start ");
  }

  if (millis() > nextReactionTime) {
    if (isForward) {
      //WIRD AM ENDE AUFGERUFEN
      if (activeLight == lightCount - 1) {

        turnLightStripPredefined(activeLight - 1,  0);

        turnLightStripPredefined(activeLight,   FADE_VAR);
        //hier nächsten oder vorigen Strip aufrufen
        handleNetworkForward();
        delay(delayPerLight);
        turnLightStripPredefined(activeLight,  0);
        isForward = false;

        Serial.println("forward ended");
      }
      //normales aufrufen, iteriert durch
      else {
        // digitalWrite(lightPins[activeLight], LOW);
        if (activeLight >= 1) {
          turnLightStripPredefined(activeLight - 1,  0);
        }

        turnLightStripPredefined(activeLight,   FADE_VAR);
        //  turnLightStripOn(activeLight, 0);

        activeLight++;
        //digitalWrite(lightPins[activeLight], HIGH);

        turnLightStripPredefined(activeLight,  1);

        //nextReactionTime = millis() + delayPerLight;
        handleNextReactionTime();
        Serial.print("Forward change: ");
        Serial.print(activeLight);
        Serial.println(" is on");
      }


    }
    else if (isBackward) {
      //WIRD AM ENDE AUFGERUFEN
      if (activeLight == 0) {

        //  digitalWrite(lightPins[activeLight], LOW);
        // turnLightStripOn(activeLight, 0);
        turnLightStripPredefined(activeLight + 1,  0);

        turnLightStripPredefined(activeLight,   FADE_VAR);
        //hier call an nächsten/vorigen Strip
        handleNetworkBackward();
        delay(delayPerLight);

        turnLightStripPredefined(activeLight,  0);
        isBackward = false;
        Serial.println("backward ended");
      }
      //Normales Aufrufen, iteriert durch
      else {
        turnLightStripPredefined(activeLight + 1,  0);

        turnLightStripPredefined(activeLight,   FADE_VAR);
        activeLight--;
        //digitalWrite(lightPins[activeLight], HIGH);
        turnLightStripPredefined(activeLight,  1);

        //nextReactionTime = millis() + delayPerLight;
        handleNextReactionTime();
      }
    }
  }
}
