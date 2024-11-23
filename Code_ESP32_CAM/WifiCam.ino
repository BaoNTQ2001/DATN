#include <WebServer.h>
#include <WiFi.h>
#include <esp32cam.h>
#include <ESP32Servo.h>

#define RXD1 12
#define TXD1 13

//const char* WIFI_SSID = "A005";
//const char* WIFI_PASS = "vnhcmute";

const char* WIFI_SSID = "DATN";
const char* WIFI_PASS = "12345678";

//const char* WIFI_SSID = "C201";
//const char* WIFI_PASS = "Nlttc201";
 
//const char* WIFI_SSID = "IoT Lab";
//const char* WIFI_PASS = "IoT@123456";


#define ServoPin 14
#define True_Face 1
#define False_Face 0

//const char* ssid = "ESP32CAM"; 
//const char* password = "12345﻿";
///* Gắn cho ESP32 IP tĩnh */
//IPAddress local_ip(192,168,1,1);
//IPAddress gateway(192,168,1,1);
//IPAddress subnet(255,255,255,0);

IPAddress staticIP(192, 168, 1, 100); // Địa chỉ IP cố định bạn muốn sử dụng
IPAddress gateway(192, 168, 1, 1);   // Địa chỉ của Gateway (Router)
IPAddress subnet(255, 255, 255, 0);   // Subnet Mask


char Face;
WebServer server(80);
Servo myservo; 
 
static auto loRes = esp32cam::Resolution::find(320, 240);
static auto midRes = esp32cam::Resolution::find(350, 530);
static auto hiRes = esp32cam::Resolution::find(800, 600);
void serveJpg()
{
  auto frame = esp32cam::capture();
  if (frame == nullptr) {
    Serial.println("CAPTURE FAIL");
    server.send(503, "", "");
    return;
  }
  Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                static_cast<int>(frame->size()));
 
  server.setContentLength(frame->size());
  server.send(200, "image/jpeg");
  WiFiClient client = server.client();
  frame->writeTo(client);
}
 
void handleJpgLo()
{
  if (!esp32cam::Camera.changeResolution(loRes)) {
    Serial.println("SET-LO-RES FAIL");
  }
  serveJpg();
}
 
void handleJpgHi()
{
  if (!esp32cam::Camera.changeResolution(hiRes)) {
    Serial.println("SET-HI-RES FAIL");
  }
  serveJpg();
}
 
void handleJpgMid()
{
  if (!esp32cam::Camera.changeResolution(midRes)) {
    Serial.println("SET-MID-RES FAIL");
  }
  serveJpg();
}
 
void opendoor_server()
{
  handleJpgLo();
  Face = True_Face;
//  myservo.write(90);
//  delay(5000);
//  myservo.write(0);
  //ESP.restart();
}

void CheckFace()
{
  if(Face == True_Face)
  {
    myservo.write(90);
    //Serial1.write('1111111111');
    delay(5000);
    myservo.write(0);
    Face = False_Face;
  } 
}

 
void  setup(){
  myservo.attach(ServoPin);
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, RXD1, TXD1); // UART1
  Serial.println();
  {
    using namespace esp32cam;
    Config cfg;
    cfg.setPins(pins::AiThinker);
    cfg.setResolution(hiRes);
    cfg.setBufferCount(2);
    cfg.setJpeg(80);
 
    bool ok = Camera.begin(cfg);
    Serial.println(ok ? "CAMERA OK" : "CAMERA FAIL");
  }
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  if (!WiFi.config(staticIP, gateway, subnet)) {
    Serial.println("Failed to configure Static IP");
  }
           
   Serial.print("http://");
   Serial.println(WiFi.localIP());

   Serial.println("  /cam-lo.jpg");
   Serial.println("  /cam-hi.jpg");
   Serial.println("  /cam-mid.jpg");
 
  server.on("/cam-lo.jpg", handleJpgLo);
  server.on("/cam-hi.jpg", handleJpgHi);
  server.on("/cam-mid.jpg", handleJpgMid);
  server.on("/opendoor", opendoor_server);
 
  server.begin();
}
 
void loop()
{
  server.handleClient();
    
  CheckFace();
  UART ();
}

void UART ()
{
  if (Serial1.available() > 0) {
    char receivedChar = Serial1.read(); // Đọc ký tự từ UART

    if (receivedChar == '1') {
      myservo.write(90);
    } else if (receivedChar == '0') {
      myservo.write(0);
    }
}
}
