#include <WiFi.h> //for the wifi AP
#include <WebSocketsServer.h> //for the communication between the external device and the radar
#include <WebServer.h> // for sending the web app to the clients
#include <ArduinoJson.h> //for the JSON format 
WebSocketsServer WebSocketServer(81);
WebServer WebServer(80);
const int RotorPin = 13;
const int TRIG_PIN =32;
const int ECHO_PIN =35;
bool forward = true;
const int maximalPWMSignal = 2400;
const int minimalPWMSignal =0;
int currentPWMSignal =0;
float currentDistance;
// variables for the functions of the clients (in the future)
bool startScan = true;


void setup() {
  pinMode(RotorPin,OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  Serial.begin(9600); // for debugging
  setupWiFiAP();
  setupWebServerRooting();
  setupWebSocket();
  Serial.println("Server is opened");
}

void loop() {
  if(startScan){
    sendPWM(currentPWMSignal,RotorPin);
    currentDistance = calculateDistance();

    Serial.print("angle: " + String(5.6*(currentPWMSignal/75))); // this is the angle format
    if(currentDistance == -1 || currentDistance >= 400){
      Serial.println(", no object detected."); // change to send data accordingly
    }
    else{
      Serial.println(", distance: " + String(currentDistance));// change to send data accordingly
    }
    if(forward){
      if(currentPWMSignal == maximalPWMSignal){
        forward = false;
        currentPWMSignal -=75;
      }
      else{
        currentPWMSignal += 75;
      }
    }
    else{
      if(currentPWMSignal == minimalPWMSignal){
        forward = true;
        currentPWMSignal +=75;
      }
      else{
        currentPWMSignal -= 75;
      }
    }
    // saving the radar data in JSON format
    StaticJsonDocument<200> radarData;
    radarData["angle"] =String(5.6*(currentPWMSignal/75));
    radarData["distance"] = currentDistance;
    String jsonString;
    serializeJson(radarData,jsonString);
    // sending the radar data to all the clients
    int currentTime = millis();
    while(millis() - currentTime < 100){
      WebServer.handleClient();
    }
    int newCurrentTime = millis();
    while(millis() - newCurrentTime < 100){
      WebSocketServer.loop();
      WebSocketServer.broadcastTXT(jsonString);
    }
  }
  else{ // servers working just staying on the same angle and distance
    DynamicJsonDocument radarData(1024);
    radarData["angle"] =String(5.6*(currentPWMSignal/75));
    radarData["distance"] = currentDistance;
    String jsonString;
    serializeJson(radarData,jsonString);
    int currentTime = millis();
    while(millis() - currentTime < 100){
      WebServer.handleClient();
    }
    int newCurrentTime = millis();
    while(millis() - newCurrentTime < 100){
      WebSocketServer.loop();
      WebSocketServer.broadcastTXT(jsonString);
    }
  }
}
// functions of the radar's calculations and movenemnt
void sendPWM(int PulseWidthMicroseconds, int Pin){
  digitalWrite(Pin,HIGH);
  delayMicroseconds(PulseWidthMicroseconds);
  digitalWrite(Pin,LOW);
  delayMicroseconds(20000 - PulseWidthMicroseconds);
}

float calculateDistance(){
  digitalWrite(TRIG_PIN,HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN,LOW);
  float time = pulseIn(ECHO_PIN,HIGH);
  if (time == 0){
    return -1;
  }
  else{
    return (time * 0.016565 + 0.00075); // might change it to dependant on temperature
  }
  delayMicroseconds(25000);
}
// functions of the servers setup and rooting
void setupWiFiAP(){
  WiFi.softAP("ESP32_AP","25042007");
  Serial.println("Wi-Fi Access Point started");
  Serial.print("IP Address:");
  Serial.println(WiFi.softAPIP());
}
void setupWebSocket(){
  WebSocketServer.onEvent(onWebSocketEvent);
  WebSocketServer.begin();
}
void onWebSocketEvent(uint8_t client_num, WStype_t type, uint8_t* payload, size_t length) {
    String message;
    StaticJsonDocument<200> clientsMessage;
    const char* action;
    switch (type) {
      case WStype_TEXT:
        message = String((char*)payload, length);
        Serial.printf("Recieved message: %s \n", message.c_str()); // for debug
        deserializeJson(clientsMessage, message);
        action = clientsMessage["action"];
        if(strcmp(action,"Start")==0){
          startScan = true;
          Serial.println("Start scan activated");
        }
        else if(strcmp(action,"Stop")==0){
          startScan = false;
          Serial.println("stop scan activated");
        }
        break;
    }
}

// functions of root handling of the servers
void setupWebServerRooting(){
  WebServer.on("/", HTTP_GET,handleRoot);
  WebServer.onNotFound(handleNotFound);
  WebServer.begin();
}
void handleNotFound(){
  WebServer.send(404, "text/plain", "404: Not Found");
}
void handleRoot() {
    const char htmlContent[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>Radar Control</title>
    <style>
      body { font-family: Arial, sans-serif; background-color: #f4f4f9; margin: 0; padding: 0; display: flex; justify-content: center; align-items: center; height: 100vh; }
      #container { text-align: center; border: 2px solid #333; padding: 20px; background-color: #fff; box-shadow: 0 0 15px rgba(0, 0, 0, 0.1); }
      h1 { font-size: 24px; }
      #data p { font-size: 18px; }
      #controls button { padding: 10px 20px; font-size: 16px; margin: 10px; cursor: pointer; }
      #controls button:active { background-color: #ddd; }
      #radar { display: block; margin: 20px auto; }
      #scanLine { transform-origin: 150px 140px; transition: transform 0.1s linear, stroke 0.1s linear; }
      #objectIndicator { transition: opacity 2s linear; }
      #redAlert { color: red; font-weight: bold; display: none; }
    </style>
  </head>
  <body>
    <div id="container">
      <h1>Radar Control</h1>
      <svg id="radar" width="300" height="150" viewBox="0 0 300 150">
        <path d="M 20,140 A 130,130 0 0,1 280,140" stroke="black" stroke-width="2" fill="none" />
        <line id="scanLine" x1="150" y1="140" x2="280" y2="140" stroke="green" stroke-width="2" />
        <circle id="objectIndicator" cx="150" cy="140" r="5" fill="green" style="display: none; opacity: 1;" />
      </svg>
      <div id="data">
        <p>Angle: <span id="angle">0</span>°</p>
        <p>Distance: <span id="distance">0</span> cm</p>
      </div>
      <div id="redAlert">Object Too Close!</div>
      <div id="controls">
        <button id="startBtn">Start</button>
        <button id="stopBtn">Stop</button>
      </div>
    </div>
    <script>
      const socket = new WebSocket("ws://" + location.hostname + ":81");
      const angleElement = document.getElementById("angle");
      const distanceElement = document.getElementById("distance");
      const scanLine = document.getElementById("scanLine");
      const objectIndicator = document.getElementById("objectIndicator");
      const redAlert = document.getElementById("redAlert");
      const startButton = document.getElementById("startBtn");
      const stopButton = document.getElementById("stopBtn");
      let fadeTimeout;
      socket.onopen = function() { console.log("Connected to WebSocket server"); };
      socket.onmessage = function(event) {
        const data = JSON.parse(event.data);
        if (data.angle && data.distance) {
          angleElement.textContent = data.angle;
          distanceElement.textContent = data.distance;
          scanLine.style.transform = `rotate(${-data.angle}deg)`;
        }
        if ((data.objectFound === true || data.objectFound === "true") && data.distance < 400) {
          scanLine.setAttribute("stroke", "red");
          const angleRad = data.angle * Math.PI / 180;
          const lineLength = 130;
          const pixelDistance = (data.distance / 400) * lineLength;
          const objectX = 150 + pixelDistance * Math.cos(angleRad);
          const objectY = 140 - pixelDistance * Math.sin(angleRad);
          objectIndicator.setAttribute("cx", objectX);
          objectIndicator.setAttribute("cy", objectY);
          objectIndicator.style.display = "block";
          objectIndicator.style.opacity = "1";
          if (data.distance < 100) {
            objectIndicator.setAttribute("fill", "red");
            redAlert.style.display = "block";
          } else {
            objectIndicator.setAttribute("fill", "green");
            redAlert.style.display = "none";
          }
          if (fadeTimeout) clearTimeout(fadeTimeout);
          fadeTimeout = setTimeout(() => {
            objectIndicator.style.opacity = "0";
            redAlert.style.display = "none";
            setTimeout(() => {
              objectIndicator.style.display = "none";
              objectIndicator.style.opacity = "1";
            }, 2100);
          }, 2000);
        } else {
          scanLine.setAttribute("stroke", "green");
          objectIndicator.style.display = "none";
          objectIndicator.style.opacity = "1";
          redAlert.style.display = "none";
        }
      };
      socket.onclose = function() { console.log("Disconnected from WebSocket server"); };
      startButton.onclick = function() { socket.send(JSON.stringify({ action: "Start" })); };
      stopButton.onclick = function() { socket.send(JSON.stringify({ action: "Stop" })); };
    </script>
  </body>
</html>
    )rawliteral";
    WebServer.send(200, "text/html", htmlContent);
}
