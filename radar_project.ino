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
 
    )rawliteral";
    WebServer.send(200, "text/html", htmlContent);
}