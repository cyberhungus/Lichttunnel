String MainPage() {
  String html = "";
  if (ESP_POSITION == 1) {
 html ="<!DOCTYPE html> <html lang=\"en\"> <head> <meta charset=\"UTF-8\"> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"> <title>Illustratio Lichttunnel </title> </head> <style> /* Allgemeine Stile */ body { font-family: Arial, sans-serif; } /* Flex-Container für das gesamte Layout */ .flex-container { display: flex; flex-direction: row; /* Elemente vertikal anordnen */ flex-wrap: wrap; align-items: center; /* Horizontal zentrieren */ justify-content: start; /* Am Anfang ausrichten */ min-height: 1vh; /* Minimale Höhe auf Fenstergröße setzen */ padding: 20px; } button { padding: 10px; /* Inneren Abstand der Buttons erhöhen */ margin: 5px; /* Außenabstand der Buttons */ cursor: pointer; /* Cursor auf 'Pointer' setzen für eine bessere UX */ border-radius: 5px; /* Ecken abrunden */ } #colorWheel { margin-top: 20px; /* Abstand oben vergrößern */ min-width: 300px; min-height: 200px; } /* Stylt die Slider-Ausgabe */ #sliderValue { font-weight: bold; margin-left: 10px; } /* Anpassungen für Buttons zur Geschwindigkeitseinstellung */ #fasterButton, #slowerButton { background-color: #007BFF; /* Farbe anpassen */ color: white; /* Textfarbe */ border: none; /* Rahmen entfernen */ margin: 10px; } /* Stil-Anpassungen für Vorwärts- und Rückwärts-Test-Buttons */ #testForwardButton, #testBackwardButton { background-color: #28a745; /* Hintergrundfarbe */ color: white; /* Textfarbe */ border: none; /* Keinen Rahmen anzeigen */ } /* Layout-Anpassungen für den Farbauswahl- und Änderungs-Button */ #changeColorButton { background-color: #17a2b8; /* Hintergrundfarbe */ color: white; /* Textfarbe */ border: none; /* Rahmen entfernen */ margin-top: 20px; /* Abstand nach oben */ } #PatternButton1{ background-color: ghostwhite; } </style> <body> <h1>Lichttunnel Steuerung</h1> <div class=\"flex-container\"> <label for=\"modes\">TimeMode:</label> <select name=\"mode\" id=\"modes\"> <option value=\"0\">Konstant</option> <option value=\"1\">Leicht Schneller</option> <option value=\"2\">Leicht Langsamer</option> <option value=\"3\">Fix Langsamer</option> <option value=\"4\">Exponentiell Langsamer</option> <option value=\"\"5>Exponentiell Schneller</option> </select> </div> <div class=\"flex-container\"> <label for=\"fill\">FillMode:</label> <select name=\"fill\" id=\"fill\"> <option value=\"0\">Solid</option> <option value=\"1\">Both Sides</option> <option value=\"2\">From Front</option> <option value=\"3\">From Back</option> </select> </div> <div class=\"flex-container\"> <p>Tunnel Nr: </p> <input type=\"radio\" id=\"option1\" name=\"radio-group\" value=\"0\"><label for=\"option1\" id=\"label0\">LED 1</label> <input type=\"radio\" id=\"option2\" name=\"radio-group\" value=\"1\"><label for=\"option2\" id=\"label1\">LED 2</label> <input type=\"radio\" id=\"option3\" name=\"radio-group\" value=\"2\"><label for=\"option3\" id=\"label2\">LED 3</label> <input type=\"radio\" id=\"option4\" name=\"radio-group\" value=\"3\"><label for=\"option4\" id=\"label3\">LED 4</label> <input type=\"radio\" id=\"option5\" name=\"radio-group\" value=\"4\"><label for=\"option5\" id=\"label4\">LED 5</label> </div> <div class=\"flex-container\"> <input type=\"color\" id=\"colorWheel\"> <button id=\"changeColorButton\">Farbe ändern</button> </div> <div class=\"flex-container\"> <button id=\"PatternButton1\">Mild White</button> <button id=\"PatternButton2\">P2</button> <button id=\"PatternButton3\">P3</button> <button id=\"PatternButton4\">P4</button> <button id=\"PatternButton5\">P5</button> <button id=\"PatternButton6\">P6</button> </div> <hr> <div class=\"flex-container\"> <p>Verzögerung (ms): 2000<span id=\"sliderValue\"></span></p> <input type=\"range\" id=\"mySlider\" min=\"200\" max=\"10000\" value=\"2000\"> <button id=\"fasterButton\">Schneller</button> <button id=\"slowerButton\">Langsamer</button> </div> <hr> <div class=\"flex-container\"> <button id=\"testForwardButton\">Test Vorwärts</button> <button id=\"testBackwardButton\">Test Rückwärts</button> </div> <script> document.addEventListener('DOMContentLoaded', function() { var fasterButton = document.getElementById(\"fasterButton\"); var slowerButton = document.getElementById(\"slowerButton\"); var testForwardButton = document.getElementById(\"testForwardButton\"); var testBackwardButton = document.getElementById(\"testBackwardButton\"); var changeColorButton = document.getElementById(\"changeColorButton\"); fasterButton.addEventListener(\"click\", goFaster); slowerButton.addEventListener(\"click\", goSlower); testForwardButton.addEventListener(\"click\", forwardTest); testBackwardButton.addEventListener(\"click\", backwardTest); changeColorButton.addEventListener(\"click\", updateColor); var mySlider = document.getElementById(\"mySlider\"); mySlider.addEventListener(\"input\", updateSliderValue); changeRadioButtonColor(1); changeRadioButtonColor(2); changeRadioButtonColor(3); changeRadioButtonColor(4); changeRadioButtonColor(0); document.getElementById(\"PatternButton1\").addEventListener('click', changePatternTo1); document.getElementById(\"PatternButton2\").addEventListener('click', changePatternTo2); document.getElementById(\"PatternButton3\").addEventListener('click', changePatternTo3); document.getElementById(\"PatternButton4\").addEventListener('click', changePatternTo4); document.getElementById(\"PatternButton5\").addEventListener('click', changePatternTo5); document.getElementById(\"PatternButton6\").addEventListener('click', changePatternTo6); document.getElementById(\"modes\").addEventListener('change', changeAccelMode); document.getElementById(\"fill\").addEventListener('change', changeFillMode); }); function goFaster(){ var slider = document.getElementById(\"mySlider\"); var val = parseInt(slider.value); slider.value = Math.max(200, val - 100); updateSliderValue(); } function goSlower(){ var slider = document.getElementById(\"mySlider\"); var val = parseInt(slider.value); slider.value = Math.min(10000, val + 100); updateSliderValue(); } function forwardTest(){ sendRequest(\"/forward\"); } function backwardTest(){ sendRequest(\"/backward\"); } function updateSliderValue() { var slider = document.getElementById(\"mySlider\"); var output = document.getElementById(\"sliderValue\"); output.innerHTML = slider.value; callSpeed(slider.value); } function callSpeed(newSpeed){ var url = \"changeSpeed/?speed=\" + newSpeed; sendRequest(url); } function updateColor() { var radioValue = document.querySelector('input[name=\"radio-group\"]:checked').value; var colorValue = document.getElementById(\"colorWheel\").value; var lightArray = hexToRgb(colorValue); var url = \"changeColor/?lightnumber=\" + radioValue +\"&RVal=\" + lightArray.r +\"&BVal=\" + lightArray.b +\"&GVal=\" + lightArray.g; sendRequest(url); changeRadioButtonColor(radioValue); } function changeAccelMode(){ var newAccelMode = document.getElementById(\"modes\").value; var url = \"changeAccelMode/?accelmode=\"+newAccelMode; sendRequest(url); } function changeFillMode(){ var newAccelMode = document.getElementById(\"fill\").value; var url = \"changeFillMode/?fillmode=\"+newAccelMode; sendRequest(url); } function updateColorParam(radioValue,r,b,g ) { var url = \"changeColor/?lightnumber=\" + radioValue +\"&RVal=\" + r +\"&BVal=\" + b +\"&GVal=\" + g; sendRequest(url); changeRadioButtonColor(radioValue); } function hexToRgb(hex) { var shorthandRegex = /^#?([a-f\\d])([a-f\\d])([a-f\\d])$/i; hex = hex.replace(shorthandRegex, function(m, r, g, b) { return r + r + g + g + b + b; }); var result = /^#?([a-f\\d]{2})([a-f\\d]{2})([a-f\\d]{2})$/i.exec(hex); return result ? { r: parseInt(result[1], 16), g: parseInt(result[2], 16), b: parseInt(result[3], 16) } : null; } function changePatternTo1() { console.log(\"Pattern changed to 1\"); updateColorParam(0,100,100,100); updateColorParam(1,100,100,100); updateColorParam(2,100,100,100); updateColorParam(3,100,100,100); updateColorParam(4,100,100,100); } function changePatternTo2() { console.log(\"Pattern changed to 2\"); } function changePatternTo3() { console.log(\"Pattern changed to 3\"); } function changePatternTo4() { console.log(\"Pattern changed to 4\"); } function changePatternTo5() { console.log(\"Pattern changed to 5\"); } function changePatternTo6() { console.log(\"Pattern changed to 6\"); } function changeRadioButtonColor(lightNum) { var xhr = new XMLHttpRequest(); xhr.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { var responseText = this.responseText; var rgbValues = responseText.match(/\\d+/g); if (rgbValues && rgbValues.length === 3) { var color = 'rgb(' + rgbValues[0] + ', ' + rgbValues[1] + ', ' + rgbValues[2] + ')'; var radioButton = document.getElementById(\"label\"+ lightNum); if (radioButton) { radioButton.style.backgroundColor = color; } } else { console.error('Invalid RGB values received from the API.'); } } else if (this.readyState == 4) { console.error('Error fetching the color data:', this.statusText); } }; xhr.open(\"GET\", `/api/color?lightNum=${lightNum}`, true); xhr.send(); } function sendRequest(url){ var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { console.log(this.responseText); } }; xhttp.open(\"GET\", url, true); xhttp.send(); } </script> </body> </html>";

  }
  else {
    html = "Please use 192.168.4.1 to control the installation";
  }

  return html;
}
String ErrorPage() {
  return "Error ";
}
