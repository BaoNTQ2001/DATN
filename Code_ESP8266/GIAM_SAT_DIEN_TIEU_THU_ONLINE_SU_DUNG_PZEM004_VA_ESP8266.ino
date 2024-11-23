#define BLYNK_TEMPLATE_ID "TMPL6OoE4ymot"
#define BLYNK_TEMPLATE_NAME "DATN"
#define BLYNK_AUTH_TOKEN "oCGXjNwxoDVAvhsFK8qt8U0AInA6hlLT"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <TimeLib.h>
#include <WidgetRTC.h>
#include <EEPROM.h>
#include <PZEM004Tv30.h>
#include <Adafruit_GFX.h>       
#include <Adafruit_ILI9341.h>
#include <My_Logo.h> 

#include <SoftwareSerial.h>

SoftwareSerial mySerial1(3, 1);  // (RX, TX) 

#define Tx D1     //--->Rx pzem
#define Rx D2     //--->Tx pzem
#define TFT_CS    D8     // TFT CS  pin is connected to NodeMCU pin D8
#define TFT_RST   D0     // TFT RST pin is connected to NodeMCU pin D0
#define TFT_DC    D3     // TFT DC  pin is connected to NodeMCU pin D3
//initialize ILI9341 TFT library with hardware SPI module

//SCK (CLK) ---> NodeMCU pin D5 (GPIO14)
//MOSI(DIN) ---> NodeMCU pin D7 (GPIO13)

#define relay     D4
//#define relay1    D4

// Assign names to common 16-bit color values:
#define BLACK    0x0000
#define WHITE    0xFFFF
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define YELLOW   0xFFE0

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

PZEM004Tv30 pzem(Rx, Tx);

float voltage=0;
float current=0;
float power=0;
float energy=0;
float frequency=0;
float pf;
float csHomqua, csHomtruoc, csThangroi, csThangtruoc;
int ngayChotSo=1;
int gioChotSo=23, phutChotSo=59;
int pinValue = 0;
int door = 0;
boolean resetE = false;
//boolean resetE = true;

unsigned long times = millis();
//----------------------------
//const char ssid[] = "A005";  
//const char pass[] = "vnhcmute"; 

const char ssid[] = "DATN";
const char pass[] = "12345678";

//const char ssid[] = "IoT Lab";
//const char pass[] = "IoT@123456";

char auth[] = BLYNK_AUTH_TOKEN;
WidgetRTC rtc;
//WidgetLED led_connect(V0);
BlynkTimer timer;
boolean savedata = false;

void setup() {
  Serial.begin(9600);
  mySerial1.begin(9600);
//  pinMode(button,INPUT_PULLUP);
//  pinMode(led,OUTPUT);
//  digitalWrite(led,HIGH);
// attachInterrupt(digitalPinToInterrupt(button),resetEnergy,FALLING);
  
  pinMode(relay,OUTPUT);
  //pinMode(relay1,OUTPUT);
  Blynk.begin(auth, ssid, pass);
  
  EEPROM.begin(512);
  delay(10);
  readChiSo();
  setSyncInterval(60*60);
  timer.setInterval(60000L,saveData);
  
  tft.begin();
  tft.fillScreen(BLACK);
  TFT_Display();
}

void loop() {
  if(millis() - times>5000){
    times = millis();
    readPzem();
    TFT_Display_Var();
    Blynk.virtualWrite(V1, voltage);
    Blynk.virtualWrite(V2, current);
    Blynk.virtualWrite(V3, power);
    Blynk.virtualWrite(V4, frequency);
    Blynk.virtualWrite(V5,energy-csHomqua);
    Blynk.virtualWrite(V6,csHomqua-csHomtruoc);
    Blynk.virtualWrite(V7,energy-csThangroi);
    Blynk.virtualWrite(V8,csThangroi-csThangtruoc);
    Blynk.virtualWrite(V9,energy);

  }
  if(resetE==true){
    resetE = false;
    pzem.resetEnergy();
    for (int i = 0; i < 16; ++i) {
    EEPROM.write(i, 0);           
      delay(10);
      //digitalWrite(led,!digitalRead(led));
    }
    EEPROM.commit();
    readChiSo();
  }
  timer.run();
  if(savedata==true){
    savedata=false;
    writeChiSo();
  }
  Blynk.run();

  send_data();
  //receive_data();

}

//==============Chương trình con======================//
void readPzem(){
    voltage = pzem.voltage();
    if( !isnan(voltage) ){
        Serial.print("Voltage: "); Serial.print(voltage); Serial.println("V");
    }
    current = pzem.current();
    if( !isnan(current) ){
        Serial.print("Current: "); Serial.print(current); Serial.println("A");
    }
    power = pzem.power();
    if( !isnan(power) ){
        Serial.print("Power: "); Serial.print(power); Serial.println("W");
    }
    energy = pzem.energy();
    if( !isnan(energy) ){
        Serial.print("Energy: "); Serial.print(energy,3); Serial.println("kWh");
    } else {
        Serial.println("Error reading energy");
    }

    frequency = pzem.frequency();
    if( !isnan(frequency) ){
        Serial.print("Frequency: "); Serial.print(frequency, 1); Serial.println("Hz");
    }
    pf = pzem.pf();
    if( !isnan(pf) ){
        Serial.print("PF: "); Serial.println(pf);
        Serial.print("ngay chot so: "); Serial.println(ngayChotSo);
        Serial.print("trang thai: "); Serial.println(pinValue);
    }
    //Serial.println();
}

ICACHE_RAM_ATTR void resetEnergy(){
  resetE = true;
  Serial.println("Reset energy!");
}

//-------------BLYNK----------------------
BLYNK_CONNECTED() {
  rtc.begin();
  Blynk.syncVirtual(V10);
  Blynk.syncVirtual(V11);
  Blynk.syncVirtual(V12);
  Blynk.syncVirtual(V13);
}

BLYNK_WRITE(V10){
  ngayChotSo = param.asInt();
}

BLYNK_WRITE(V11)
{
  pinValue = param.asInt();
  digitalWrite(relay,pinValue);
  
}

BLYNK_WRITE(V12)
{
  door = param.asInt();
}

BLYNK_WRITE(V13) {  // Button Widget chọn V0
  if (param.asInt() == 1) {  //nếu nhấn nút reset thì gọi hàm
    Blynk.virtualWrite(V13, 0);
    pzem.resetEnergy(); // reset PZEM
  } else {  

  }}

//-------------Ghi dữ liệu kiểu float vào bộ nhớ EEPROM----------------------//
float readFloat(unsigned int addr){
  union{
    byte b[4];
    float f;
  }data;
  for(int i=0; i<4; i++){
    data.b[i]=EEPROM.read(addr+i);
  }
  return data.f;
}
void writeFloat(unsigned int addr, float x){
  union{
    byte b[4];
    float f;
  }data;
  data.f=x;
  for(int i=0; i<4;i++){
    EEPROM.write(addr+i,data.b[i]);
  }
}
void readChiSo(){
  csHomqua = readFloat(0);
  csHomtruoc = readFloat(4);
  csThangroi = readFloat(8);
  csThangtruoc = readFloat(12);
  Serial.print("Chỉ số hôm qua: ");Serial.println(csHomqua);
  Serial.print("Chỉ số hôm trước: ");Serial.println(csHomtruoc);
  Serial.print("Chỉ số tháng rồi: ");Serial.println(csThangroi);
  Serial.print("Chỉ số tháng trước: ");Serial.println(csThangtruoc);
}
void saveData(){
  savedata = true;
}
void writeChiSo(){
  if(hour()==gioChotSo && minute() == phutChotSo){
    Serial.println("Ghi số ngày mới!");
    writeFloat(4,csHomqua);
    writeFloat(0,energy);
    if(day()==ngayChotSo){
      Serial.println("Ghi số tháng mới!");
      writeFloat(12,csThangroi);
      writeFloat(8,energy);
    }
    EEPROM.commit();
    readChiSo();
  }
}
//void blinkled(){
//  if(led_connect.getValue()){
//    led_connect.off();
//  }else{
//    led_connect.on();
//  }
//}

void TFT_Display() {
    tft.drawRGBBitmap(2, 2, LOGO, 237, 53);

    tft.drawRect(0,54,118,84,CYAN);  
    tft.drawRect(2,56,114,80,YELLOW);    
    printText("U:",CYAN,53,60,2); 

    
    tft.drawRect(120,54,120,84,CYAN);    
    tft.drawRect(122,56,116,80,YELLOW);
    printText("I:",CYAN,173,60,2);      

  
    tft.drawRect(0,142,118,84,CYAN);  
    tft.drawRect(2,144,114,80,YELLOW);
    printText("P:",CYAN,53,146,2);
  
    tft.drawRect(120,142,120,84,CYAN); 
    tft.drawRect(122,144,116,80,YELLOW);
    printText("F:",CYAN,173,146,2);      

     

    tft.drawRect(0,230,118,84,CYAN);
    tft.drawRect(2,232,114,80,YELLOW);
    printText("PF:",CYAN,50,234,2);


    tft.drawRect(120,230,120,84,CYAN);   
    tft.drawRect(122,232,116,80,YELLOW);
    printText("A:",CYAN,173,234,2);

}


void TFT_Display_Var() {
  
  tft.setTextSize(2);                                                       
  tft.setTextColor(CYAN);                                                  

  tft.setCursor(25,100);
  tft.fillRect(25,100,90,30,BLACK);
  tft.print(voltage);
  tft.print("V");

  tft.setCursor(155,100);
  tft.fillRect(155,100,80,30,BLACK);
  tft.print(current);
  tft.print("A");

  tft.setCursor(35,186);
  tft.fillRect(35,186,80,30,BLACK);
  tft.print(power);
  tft.print("W");  

  tft.setCursor(145,186);
  tft.fillRect(145,186,90,30,BLACK);
  tft.print(frequency);
  tft.print("Hz"); 

  tft.setCursor(45,274);
  tft.fillRect(45,274,60,30,BLACK);
  tft.print(pf);

  tft.setCursor(135,274);
  tft.fillRect(135,274,100,30,BLACK);
  tft.print(energy,3);
  tft.print("kWh");  
}

void printText(char *text, uint16_t color, int x, int y,int textSize ){
  tft.setCursor(x, y);
  tft.setTextColor(color);
  tft.setTextSize(textSize);
  tft.print(text);
}

void send_data()
{
  if( 1 == door)
  {
     mySerial1.write('1');
  }
  else
  {
     mySerial1.write('0');
  }
}

//void receive_data()
//{
//  if (Serial1.available() > 0)
//  {
//    Blynk.virtualWrite(V12, 1);
//    delay(5000);
//    Blynk.virtualWrite(V12, 0);
//  }
//}
