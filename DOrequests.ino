void DOforward(AsyncWebServerRequest *request) {
  if ( isForward == false && isBackward == false) {
    delayPerLight = standardDelayPerLight;
    isForward = true;
    activeLight = 0;
    nextReactionTime = millis() + delayPerLight;
    //digitalWrite(lightPins[activeLight], HIGH);
    turnLightStripOn(activeLight, 200);

    Serial.println("Forward Start ");
    request->redirect("/");
  }
  else {
    request->redirect("/error");
  }
}

void DObackward(AsyncWebServerRequest *request) {

  if ( isForward == false && isBackward == false) {
    delayPerLight = standardDelayPerLight;
    isBackward = true;
    activeLight = lightCount - 1;
    nextReactionTime = millis() + delayPerLight;
    //digitalWrite(lightPins[activeLight], HIGH);
    turnLightStripOn(activeLight, 200);

    Serial.println("Backward Start ");
    request->redirect("/");
  }
  else {
    request->redirect("/error");
  }
}




void DOchangecolor(AsyncWebServerRequest *request) {
  Serial.println("Change color Called");
  int intLightNum = -1;
  int intRVal = 100;
  int intGVal = 100;
  int intBVal = 100;

  if (request->hasParam("lightnumber")) {
    String lightNumber = request->getParam("lightnumber")->value();
    intLightNum = lightNumber.toInt();
    Serial.println("Has lightNumber");



    if (request->hasParam("RVal")) {
      String RVal = request->getParam("RVal")->value();
      intRVal = RVal.toInt();
      Serial.println("Has red");
    }
    if (request->hasParam("GVal")) {
      String GVal = request->getParam("GVal")->value();
      intGVal = GVal.toInt();
      Serial.println("Has green");
    }
    if (request->hasParam("BVal")) {
      String BVal = request->getParam("BVal")->value();
      intBVal = BVal.toInt();
      Serial.println("Has blue");


    }

    changeColorinArray(intLightNum, intRVal, intGVal, intBVal);

    saveArrayData();
    request->redirect("/");
  }
  else {
    request->redirect("/error");
  }
}

void DOchangespeed(AsyncWebServerRequest *request) {
  if (request->hasParam("speed")) {
    String lightNumber = request->getParam("speed")->value();
    delayPerLight = lightNumber.toInt();
    standardDelayPerLight = delayPerLight;

    Serial.print("Changed Speed via Website to: ");
    Serial.println(delayPerLight);


    request->redirect("/");

  }
  else {
    request->redirect("/error");
  }
}


void DOchangeaccelmode(AsyncWebServerRequest *request) {
  if (request->hasParam("accelmode")) {
    String accMode = request->getParam("accelmode")->value();
    accelMode = accMode.toInt();
    handleAccelForward();
    Serial.print("Changed AccelMode via Website to: ");
    Serial.println(accelMode);
    request->redirect("/");

  }
  else {
    request->redirect("/error");
  }
}


void DOchangefillmode(AsyncWebServerRequest *request) {
  if (request->hasParam("fillmode")) {
    String fMode = request->getParam("fillmode")->value();
    fillMode = fMode.toInt();
    handleFillForward();
    Serial.print("Changed FillMode via Website to: ");
    Serial.println(fillMode);
    request->redirect("/");

  }
  else {
    request->redirect("/error");
  }
}

void DOgetcolor(AsyncWebServerRequest *request) {
  if (request->hasParam("lightNum")) {
    String lightNumber = request->getParam("lightNum")->value();
    int intlightnum = lightNumber.toInt();

    String toReturn = "{" + String(lightValues[intlightnum][0]) + "," +  String(lightValues[intlightnum][1])  + "," +  String(lightValues[intlightnum][2]) + "}";
    Serial.print("Called DOgetcolor for Strip Number: ");
    Serial.print(intlightnum);
    Serial.print(" returns: ");
    Serial.println(toReturn);


    request->send(200, "text/html", toReturn);

  }
  else {
    request->redirect("/error");
  }
}

void DOEmergency(AsyncWebServerRequest *request) {
  for (int strip = 0; strip < NUM_STRIPS; strip++) {

    turnLightStripRGB(strip, 255, 255, 255);
    delay(100);
  }
  if (ESP_POSITION != ESP_AMOUNT && isSolo == false) {
    String url = ipBase + String((((ESP_POSITION - 1) * 10) + 1) - 10) + "/emergency";
    Serial.println("Calling website:" + url);
    callWebsite(url);
  }
  request->redirect("/");
}
