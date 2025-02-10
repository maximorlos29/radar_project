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
    DynamicJsonDocument radarData(1024);
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
      case WStype_DISCONNECTED:
        Serial.printf("Client %d disconnected\n", client_num);
        break;
      case WStype_CONNECTED:
        Serial.printf("Client %d connected\n", client_num);
        break;
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
      case WStype_BIN:
        Serial.printf("Received binary data from Client %d, size: %d bytes\n", client_num, length);
        break;
      case WStype_ERROR:
        Serial.printf("WebSocket Error from Client %d\n", client_num);
        break;
    }
}

// functions of root handling of the servers
void setupWebServerRooting(){
  WebServer.on("/", HTTP_GET,handleRoot);
  WebServer.onNotFound(handleNotFound);
  WebServer.begin();
}
void handleRoot() {
    const char htmlContent[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Radar Control</title>
    <style>
      body {
        font-family: Arial, sans-serif;
        background-color: #f4f4f9;
        margin: 0;
        padding: 0;
        display: flex;
        justify-content: center;
        align-items: center;
        height: 100vh;
      }

      #container {
        text-align: center;
        border: 2px solid #333;
        padding: 20px;
        background-color: #fff;
        box-shadow: 0 0 15px rgba(0, 0, 0, 0.1);
      }
      h1 {
        font-size: 24px;
      }

      #data p {
        font-size: 18px;
      }

      #controls button {
        padding: 10px 20px;
        font-size: 16px;
        margin: 10px;
        cursor: pointer;
      }
      #controls button:active {
        background-color: #ddd;
      }
    </style>
  </head>
  <body>
    <div id="container">
        <h1>Radar Control</h1>
        <div id="data">
          <p>Angle: <span id="angle">0</span>°</p>
          <p>Distance: <span id="distance">0</span> cm</p>
        </div>
        <div id="controls">
          <button id="startBtn">Start</button>
          <button id="stopBtn">Stop</button>
        </div>
    </div>
    <script>
      // Establish WebSocket connection to the ESP32 server
      const socket = new WebSocket('ws://' + location.hostname + ':81'); // Change if your WebSocket port is different

      // Reference to the DOM elements
      const angleElement = document.getElementById("angle");
      const distanceElement = document.getElementById("distance");
      const startButton = document.getElementById("startBtn");
      const stopButton = document.getElementById("stopBtn");
      // Event handler when WebSocket opens
      socket.onopen = function() {
        console.log("Connected to WebSocket server");
      };

      // Event handler for receiving messages from the WebSocket server
      socket.onmessage = function(event) {
        const data = JSON.parse(event.data); // Assuming data is JSON
        if (data.angle && data.distance) {
          angleElement.textContent = data.angle;
          distanceElement.textContent = data.distance;
        }
      };

      // Event handler when WebSocket is closed
      socket.onclose = function() {
        console.log("Disconnected from WebSocket server");
      };

      // Send message when the Start button is clicked
      startButton.onclick = function() {
        const message = {
          action: "Start"
        };
        socket.send(JSON.stringify(message)); // Send a start command to ESP32
      };

      // Send message when the Stop button is clicked
      stopButton.onclick = function() {
        const message = {
          action: "Stop"
        };
        socket.send(JSON.stringify(message)); // Send a stop command to ESP32
      };
    </script>
  </body>
</html>
    )rawliteral";
    WebServer.send(200, "text/html", htmlContent);
}
void handleNotFound(){
  WebServer.send(404, "text/plain", "404: Not Found");
}