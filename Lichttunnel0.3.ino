#include <FastLED.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>

#include <Preferences.h>

Preferences preferences;

// Hier die Netzwerkkonfiguration vornehmen
//const char *ssid = "Illustratio-Lichttunnel";
//const char *password = "illustratio";

//const char *ssid = "TP-Link_9842";
//const char *password = "68409043";


const char *ssid = "TP-LINK_AP_1710";
const char *password = "93469128";


const char *APP_NAME = "Ltunnel";

//hier die Lightstrips konfigurieren
#define NUM_STRIPS 5
#define NUM_LEDS_PER_STRIP 50
#define NUM_LEDS NUM_LEDS_PER_STRIP * NUM_STRIPS
const float FADE_VAR = 0.7;
const float OFF_VAR = 0.4;
//hier wert von 1-4 angeben.
#define ESP_POSITION 2
#define ESP_AMOUNT 2
const boolean isSolo = false;

CRGB leds[NUM_STRIPS * NUM_LEDS_PER_STRIP];



//hält die standardlänge des delays. Eventuell anpassen
int standardDelayPerLight = 700;

String ipBase = "http://192.168.0.";

//Standard IP Konfiguration vom Master (self hosted network)
//Local IP: 192.168.4.XXX
//Subnet Mask: 255.255.255.0
//Gateway IP: 192.168.4.1
//DNS 1: 192.168.4.1
//DNS 2: 0.0.0.0


//tplink 9842
//14:47:57.918 -> Local IP: 192.168.0.101
//14:47:57.918 -> Subnet Mask: 255.255.255.0
//14:47:57.918 -> Gateway IP: 192.168.0.1
//14:47:57.918 -> DNS 1: 192.168.0.1
//14:47:57.918 -> DNS 2: 0.0.0.0

//TPLINK_AP_1710
//16:09:19.748 -> Local IP: 192.168.0.101
//16:09:19.748 -> Subnet Mask: 255.255.255.0
//16:09:19.748 -> Gateway IP: 192.168.0.254
//16:09:19.748 -> DNS 1: 192.168.0.254
//16:09:19.748 -> DNS 2: 0.0.0.0
//



//für die statische IP der Sub-Geräte
//staticIP wird basierend auf ESP-Pos berechnet (2,3,4)
IPAddress staticIP(192, 168, 0, ((ESP_POSITION) * 10) + 1 );
//gateway in einem vom ESP gehosteten netzwerk
IPAddress gateway(192, 168, 0, 254 );
//subnet ist immer dieses
IPAddress subnet(255, 255, 255, 0);
//dns = Server-IP
IPAddress dns(192, 168, 0, 254);

//Change these to alter where the sensors are connected
#define forwardSensor 17
#define backwardSensor 14

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

//fillmode sorgt dafür das verschiedene füllmuster benutzt werden können
int fillMode = 0;

const int maxLightDelay = 2000;
const int minLightDelay = 200;

bool isForward = false;
bool isBackward = false;

long lastReactionTime;
long nextReactionTime;



//erstelle Server Objekt
AsyncWebServer server(80);
IPAddress ip;


void printArrayState() {
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
}

//gibt Netzwerkdaten aus
void printNetworkData() {
  Serial.print("ESP-Position: ");
  Serial.println(ESP_POSITION);
  Serial.print("Is Solo: ");
  Serial.println(isSolo);
  Serial.print("NET SSID: ");
  Serial.println(ssid);
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("Subnet Mask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway IP: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("DNS 1: ");
  Serial.println(WiFi.dnsIP(0));
  Serial.print("DNS 2: ");
  Serial.println(WiFi.dnsIP(1));
}

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
void turnLightStripSolid(int stripNumber, float intensity) {
  fill_solid( leds + (stripNumber * NUM_LEDS_PER_STRIP), NUM_LEDS_PER_STRIP, CRGB(int(lightValues[stripNumber][0] * intensity) , int(lightValues[stripNumber][1] * intensity), int(lightValues[stripNumber][2] * intensity)));
  FastLED.show();
}

void turnLightStripRGB(int stripNumber, int r, int g, int b ) {

  fill_solid( leds + (stripNumber * NUM_LEDS_PER_STRIP), NUM_LEDS_PER_STRIP, CRGB(r, g, b));
  FastLED.show();
}

//funktion die lichter auf einen im array lightValues angegebenen farbwert stellt
//füllt von beiden seiten
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

void turnLightStripFromFront(int stripNumber, float intensity  ) {
  for (int i = stripNumber * NUM_LEDS_PER_STRIP; i < (stripNumber * NUM_LEDS_PER_STRIP) + NUM_LEDS_PER_STRIP; i++) {
    CRGB(int(lightValues[stripNumber][0] * intensity) , int(lightValues[stripNumber][1] * intensity), int(lightValues[stripNumber][2] * intensity));
    FastLED.show();
  }
}

//for (int i = stripNumber * NUM_LEDS_PER_STRIP; i < (stripNumber * NUM_LEDS_PER_STRIP) + NUM_LEDS_PER_STRIP; i++) {

void turnLightStripFromBack(int stripNumber, float intensity  ) {
  for (int i = (stripNumber * NUM_LEDS_PER_STRIP) + NUM_LEDS_PER_STRIP; i > stripNumber * NUM_LEDS_PER_STRIP; i--) {
    CRGB(int(lightValues[stripNumber][0] * intensity) , int(lightValues[stripNumber][1] * intensity), int(lightValues[stripNumber][2] * intensity));
    FastLED.show();
  }
}


void handleLightChange(int stripNumber, int intensity ) {


  switch (fillMode) {
    case 0:
      turnLightStripSolid(stripNumber, intensity);
      break;
    case 1:
      turnLightStripPredefined(stripNumber, intensity);
      break;
    case 2:
      turnLightStripFromFront(stripNumber, intensity);
      break;
    case 3:
      turnLightStripFromBack(stripNumber, intensity);
      break;
    default:
      // Tue etwas, im Defaultfall
      // Dieser Fall ist optional
      break; // Wird nicht benötigt, wenn Statement(s) vorhanden sind
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
    turnLightStripSolid(arrayposition, 1);
    delay(200);
    turnLightStripSolid(arrayposition, 0);
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
  if (isSolo == false) {
    //nicht den nächsten strip anrufen wenn letzter strip
    if (ESP_POSITION == ESP_AMOUNT) {
      Serial.println("last device, stopping");
      return;
    }
    else {

      handleColorChange(ipBase + String(((ESP_POSITION + 1) * 10) + 1));
      String url2 = ipBase + String(((ESP_POSITION + 1) * 10) + 1) + "/changeSpeed?speed=" + delayPerLight;
      callWebsite(url2);
      String url = ipBase + String(((ESP_POSITION + 1) * 10) + 1) + "/forward";
      Serial.println("Calling website:" + url);
      callWebsite(url);
    }
  }
}

void handleAccelForward() {
  if (isSolo == false) {
    //nicht den nächsten strip anrufen wenn letzter strip
    if (ESP_POSITION == ESP_AMOUNT) {
      Serial.println("last device, stopping");
      return;
    }
    else {
      //((ESP_POSITION-1)*10)+1
      String url = ipBase + String(((ESP_POSITION + 1) * 10) + 1) + "/changeAccelMode?accelmode=" + String(accelMode);
      Serial.println("Calling website:" + url);
      callWebsite(url);
    }
  }
}

void handleFillForward() {
  if (isSolo == false) {
    //nicht den nächsten strip anrufen wenn letzter strip
    if (ESP_POSITION == ESP_AMOUNT) {
      Serial.println("last device, stopping");
      return;
    }
    else {

      String url = ipBase + String(((ESP_POSITION + 1) * 10) + 1) + "/changeFillMode?fillmode=" + String(fillMode);
      Serial.println("Calling website:" + url);
      callWebsite(url);
    }
  }
}



void handleNetworkBackward() {
  if (isSolo == false) {
    //nicht den nächsten strip anrufen wenn letzter strip
    if (ESP_POSITION == 1) {
      Serial.println("first device, stopping");
      return;
    }
    else {
      handleColorChange(ipBase + String((((ESP_POSITION - 1) * 10) + 1) - 10));
      String url2 = ipBase + String((((ESP_POSITION - 1) * 10) + 1) - 10) + "/changeSpeed?speed=" + delayPerLight;
      callWebsite(url2);
      String url = ipBase + String((((ESP_POSITION - 1) * 10) + 1) - 10) + "/backward";
      Serial.println("Calling website:" + url);
      callWebsite(url);
    }
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
      delayPerLight = delayPerLight - (delayPerLight / 10);
      Serial.println("Accelmode with case 1-new delay is: " + String(delayPerLight));

      nextReactionTime = millis() + delayPerLight;
      break;
    case 2:
      delayPerLight = delayPerLight + (delayPerLight / 10);
      Serial.println("Accelmode with case 1-new delay is: " + String(delayPerLight));

      nextReactionTime = millis() + delayPerLight;
      break;
    case 3:
      delayPerLight = delayPerLight - (delayPerLight / 2);
      Serial.println("Accelmode with case 1-new delay is: " + String(delayPerLight));

      nextReactionTime = millis() + delayPerLight;
      break;
    case 4:
      delayPerLight = delayPerLight * 2;
      Serial.println("Accelmode with case 1-new delay is: " + String(delayPerLight));

      nextReactionTime = millis() + delayPerLight;
      break;

    case 5:
      delayPerLight = delayPerLight / 2;
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

  FastLED.addLeds<WS2811, 18, BRG>(leds, 0, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2811, 19, BRG>(leds, NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2811, 23, BRG>(leds, 2 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2811, 5, BRG>(leds, 3 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2811, 12, BRG>(leds, 4 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  lightTest();

  //server started und stellt netzwerk bereit.

  //ToDo: Implement a Self-Hosting Boolean which turns the self hosting on
  if (ESP_POSITION == 1) {
    //EInkommentieren für Netzwerk Selbst gehosted
    //    Serial.println("Erstelle Netzwerk");
    //    WiFi.softAP(ssid, password);
    //    //Printet die IP des Servers
    //    Serial.println("Netzwerk IP addresse: ");
    //    Serial.println(WiFi.softAPIP());
    //    showConfirm();


    //konfiguriere die Slaves auf feste IP-Addressen
    if (WiFi.config(staticIP, gateway, subnet, dns, dns) == false) {
      Serial.println("Configuration failed.");
    }
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(200);
      showConnectionLost();
      Serial.println("Verbindungsversuch...");
    }
    showConfirm();
    Serial.println("Netzwerkverbindung hergestellt");
    Serial.print("Local ESP32 IP: ");
    Serial.println(WiFi.localIP());

  }
  else {


    //konfiguriere die Slaves auf feste IP-Addressen
    if (WiFi.config(staticIP, gateway, subnet, dns, dns) == false) {
      Serial.println("Configuration failed.");
    }
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(200);
      showConnectionLost();
      Serial.println("Verbindungsversuch...");
    }
    showConfirm();
    Serial.println("Netzwerkverbindung hergestellt");
    Serial.print("Local ESP32 IP: ");
    Serial.println(WiFi.localIP());
  }
  //start preferences module
  preferences.begin(APP_NAME, false);

  //gibt informationen über Netzwerk as
  printNetworkData();

  //wenn die index page (domainname/) besucht wird, gib folgendes zurück (siehe Webpages)
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/html", MainPage());
  });

  //server pfas konfigurationen
  server.on("/forward", HTTP_GET, DOforward) ;
  server.on("/backward", HTTP_GET, DObackward);
  server.on("/changeColor", HTTP_GET, DOchangecolor);
  server.on("/changeSpeed", HTTP_GET, DOchangespeed);
  server.on("/changeAccelMode", HTTP_GET, DOchangeaccelmode);
  server.on("/changeFillMode", HTTP_GET, DOchangefillmode);
  server.on("/api/color", HTTP_GET, DOgetcolor);
  server.on("/emergency", HTTP_GET, DOEmergency);

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

  //  FastLED.addLeds<NEOPIXEL, 25>(leds, 0, NUM_LEDS_PER_STRIP);
  //  FastLED.addLeds<NEOPIXEL, 26>(leds, NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  //  FastLED.addLeds<NEOPIXEL, 27>(leds, 2 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  //  FastLED.addLeds<NEOPIXEL, 5>(leds, 3 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  //  FastLED.addLeds<NEOPIXEL, 17>(leds, 4 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);

  pinMode(forwardSensor, INPUT_PULLUP);
  pinMode(backwardSensor, INPUT_PULLUP);
  retrieveArrayData();
  Serial.println("System Start");

}



void loop() {




  //check for wifi connection, if none, attempt reconnect.

  if (WiFi.status() != WL_CONNECTED) {
    showConnectionLost();
    WiFi.mode(WIFI_STA);
    //konfiguriere die Slaves auf feste IP-Addressen
    if (WiFi.config(staticIP, gateway, subnet, dns, dns) == false) {
      Serial.println("Configuration failed.");
    }
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(200);
      showConnectionLost();
      Serial.println("Verbindungsversuch...");
    }
    Serial.println("Netzwerkverbindung hergestellt");
    Serial.print("Local ESP32 IP: ");
    Serial.println(WiFi.localIP());
  }



  //read forward sensor
  int result = digitalRead(forwardSensor);
  if (result == LOW && isForward == false && isBackward == false) {
    delayPerLight = standardDelayPerLight;
    isForward = true;
    activeLight = 0;
    //nextReactionTime = millis() + delayPerLight;
    handleNextReactionTime();
    //digitalWrite(lightPins[activeLight], HIGH);
    handleLightChange(activeLight,  1);

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
    handleLightChange(activeLight,  1);

    Serial.println("Backward Start ");
  }



  if (millis() > nextReactionTime) {
    if (isForward) {
      //WIRD AM ENDE AUFGERUFEN
      if (activeLight == lightCount - 1) {

        handleLightChange(activeLight - 1,  OFF_VAR);

        handleLightChange(activeLight,   FADE_VAR);
        //hier nächsten oder vorigen Strip aufrufen
        handleNextReactionTime();
        if (isSolo == false) {
          handleNetworkForward();
        }
        delay(delayPerLight);
        handleLightChange(activeLight,  OFF_VAR);
        isForward = false;

        Serial.println("forward ended");

      }
      //normales aufrufen, iteriert durch
      else {
        // digitalWrite(lightPins[activeLight], LOW);
        if (activeLight >= 1) {
          handleLightChange(activeLight - 1,  OFF_VAR);
        }

        handleLightChange(activeLight,   FADE_VAR);
        //  turnLightStripOn(activeLight, 0);

        activeLight++;
        //digitalWrite(lightPins[activeLight], HIGH);

        handleLightChange(activeLight,  1);

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

        handleLightChange(activeLight + 1,  OFF_VAR);


        handleLightChange(activeLight,   FADE_VAR);

        handleNextReactionTime();
        if (isSolo == false) {
          handleNetworkBackward();
        }
        delay(delayPerLight);

        handleLightChange(activeLight,  OFF_VAR);
        isBackward = false;
        Serial.println("backward ended");

      }
      //Normales Aufrufen, iteriert durch
      else {
        if (activeLight <= 3) {
          handleLightChange(activeLight + 1,  OFF_VAR);
        }
        handleLightChange(activeLight,   FADE_VAR);
        activeLight--;
        Serial.print("Backward change: ");
        Serial.print(activeLight);
        Serial.println(" is on");
        //digitalWrite(lightPins[activeLight], HIGH);
        handleLightChange(activeLight,  1);

        //nextReactionTime = millis() + delayPerLight;
        handleNextReactionTime();
      }
    }
  }
}
//speichert die arraydaten im persistenten speicher
void saveArrayData() {
  //nur der master muss diese daten persistent speichern
  if (ESP_POSITION == 1) {
    for (int i = 0; i < 5; i++) { // Outer loop for the first dimension
      for (int j = 0; j < 3; j++) { // Inner loop for the second dimension
        // Create a key name that includes both the i and j indices
        String key = "arrayItem" + String(i) + "_" + String(j);

        // Save the item at myArray[i][j] under the generated key
        preferences.putInt(key.c_str(), lightValues[i][j]);

      }
    }
    Serial.println("lightValues Array gespeichert");
  }
}


void retrieveArrayData() {
  //nur der master muss die daten erhalten
  if (ESP_POSITION == 1) {
    // Retrieve the array from preferences
    for (int i = 0; i < 5; i++) { // Outer loop
      for (int j = 0; j < 3; j++) { // Inner loop
        // Reconstruct the key
        String key = "arrayItem" + String(i) + "_" + String(j);

        // Get the item from preferences, defaulting to 100 if not found
        lightValues[i][j] = preferences.getInt(key.c_str(), 100);

      }
    }
    Serial.println("LightValues Array daten aus speicher geladen");
    printArrayState();
  }
}


void showConfirm() {
  Serial.println("Show Confirm");
  turnLightStripRGB(2, 0, 255, 0);
  delay(200);
  turnLightStripRGB(2, 0, 0, 0);
  delay(200);
  turnLightStripRGB(2, 0, 255, 0);
  delay(200);
  turnLightStripRGB(2, 0, 0, 0);
}
void showConnectionLost() {
  Serial.println("Show Connection ");
  turnLightStripRGB(2, 255, 255, 0);
  delay(200);
  turnLightStripRGB(2, 0, 0, 0);
  delay(200);
  turnLightStripRGB(2, 255, 255, 0);
  delay(200);
  turnLightStripRGB(2, 0, 0, 0);
}

void showError() {
  Serial.println("Show Error");
  turnLightStripRGB(2, 255, 0, 0);
  delay(200);
  turnLightStripRGB(2, 0, 0, 0);
  delay(200);
  turnLightStripRGB(2, 255, 0, 0);
  delay(200);
  turnLightStripRGB(2, 0, 0, 0);
}

void lightTest() {
  Serial.println("testing lights");
  for (int strip = 0; strip < NUM_STRIPS; strip++) {

    turnLightStripRGB(strip, 100, 100, 100);
    delay(30);
  }
  for (int strip = 0; strip < NUM_STRIPS ; strip++) {

    turnLightStripRGB(strip, 0, 0, 0);
    delay(30);
  }
}
