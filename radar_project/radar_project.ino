#include <WiFi.h> //for the wifi AP
#include <WebServer.h> // for serving the web app
#include <SPIFFS.h>
#include <WebSocketsServer.h> //for the communication between the external device and the radar
WebSocketsServer webSocket = WebSocketsServer(81);
WebServer webServer = WebServer(80);
const int RotorPin = 13;
const int TRIG_PIN =32;
const int ECHO_PIN =35;


void setup() {
  pinMode(RotorPin,OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  Serial.begin(9600);
  setupWiFiAP();
  setupWebSocket();
  if(!SPIFFS.begin(true)){
    Serial.println("Failed to mount SPIFFS");
    return;
  }
  else{
    Serial.println("Succeded to mount SPIFFS");
  }
  webServer.on("/", HTTP_GET,handleRoot); // path of the root, when a user asks for the root
  webServer.on("/style.css", HTTP_GET,handleCSS);
  webServer.on("/script.js", HTTP_GET, handleJS);
  webServer.begin();
  Serial.println("Server is opened");
}

void loop() {
  webServer.handleClient();
  for(int i=0;i<=2400;i=i+75){
    sendPWM(i,RotorPin);
    float distance = calculateDistance();
    Serial.print("angle :"); 
    Serial.print(5.6*(i/75)); // this is the angle format
    if (distance == -1){
      Serial.println(", no object detected");
    }
    else{
      Serial.print(", distance: ");
      Serial.print(distance);
      Serial.println("cm.");
    }
    delay(300);
  }
  for(int i=2400;i>=0;i=i-75){
    sendPWM(i,RotorPin);
    float distance = calculateDistance();
    Serial.print("angle :"); 
    Serial.print(5.6*(i/75)); // this is the angle format
    if (distance == -1){
      Serial.println(", no object detected");
    }
    else{
      Serial.print(", distance: ");
      Serial.print(distance);
      Serial.println("cm.");
   }
   delay(300);
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
void handleRoot(){
  File file = SPIFFS.open("/index.html", "r");
    if(!file){
      webServer.send(404,"text/plain", "File Not Found");
      return;
    }
    webServer.streamFile(file,"text/html");
    file.close();
}
void handleCSS(){
  File file = SPIFFS.open("/style.css", "r");
    if(!file){
      webServer.send(404,"text/plain", "File Not Found");
      return;
    }
    webServer.streamFile(file,"text/css");
    file.close();
}
void handleJS(){
  File file = SPIFFS.open("/script.js", "r");
    if(!file){
      webServer.send(404,"text/plain", "File Not Found");
      return;
    }
    webServer.streamFile(file,"application/javascript");
    file.close();
}