#include <WiFi.h> //for the wifi AP
#include <WebServer.h> // for serving the web app
#include <WebSocketsServer.h> //for the communication between the external device and the radar
WebSocketsServer webSocket = WebSocketsServer(81);
WebServer webServer = WebServer(80);
const int RotorPin = 13;
const int TRIG_PIN =32;
const int ECHO_PIN =35;
bool forward = true;
const int maximalPWMSignal = 2400;
const int minimalPWMSignal =0;
int currentPWMSignal =0;

void setup() {
  pinMode(RotorPin,OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  Serial.begin(9600);
  setupWiFiAP();
  setupWebSocket();
  webServer.on("/", HTTP_GET,handleRoot); // path of the root, when a user asks for the root
  webServer.on("/style.css", HTTP_GET,handleCSS);
  webServer.on("/script.js", HTTP_GET, handleJS);
  webServer.begin();
  Serial.println("Server is opened");
}

void loop() {
  sendPWM(currentPWMSignal,RotorPin);
  float distance = calculateDistance();
  Serial.print("angle: " + String(5.6*(currentPWMSignal/75))); // this is the angle format
  if(distance == -1 || distance >= 400){
    Serial.println(", no object detected.");
  }
  else{
    Serial.println(", distance: " + String(distance));
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
  int currentTime = millis();
  while(millis() - currentTime < 100){
    webServer.handleClient();
  }
}

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
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
      case WStype_CONNECTED:
          Serial.printf("Client %u connected\n", num);
          break;
      case WStype_DISCONNECTED:
          Serial.printf("Client %u disconnected\n", num);
          break;
      case WStype_TEXT:
          Serial.printf("Received from client %u: %s\n", num, payload);
          break;
      default:
          break;
  }
}

void setupWiFiAP(){
  WiFi.softAP("ESP32_AP","25042007");
  Serial.println("Wi-Fi Access Point started");
  Serial.print("IP Address:");
  Serial.println(WiFi.softAPIP());
}
void setupWebSocket(){
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("WebSocket server started");
}
void handleRoot() {
    const char htmlContent[] PROGMEM = R"rawliteral(
        <!DOCTYPE html>
        <html lang="en">
        <head>
            <meta charset="UTF-8">
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <title>Hello World</title>
            <style>
                body {
                    font-family: Arial, sans-serif;
                    margin: 0;
                    padding: 0;
                    display: flex;
                    justify-content: center;
                    align-items: center;
                    height: 100vh;
                    background-color: #f4f4f4;
                }
                h1 {
                    color: #333;
                }
            </style>
        </head>
        <body>
            <h1>Hello, World!</h1>
        </body>
        </html>
    )rawliteral";

    webServer.send(200, "text/html", htmlContent);
}
void handleCSS(){
  
}
void handleJS(){
  
}
