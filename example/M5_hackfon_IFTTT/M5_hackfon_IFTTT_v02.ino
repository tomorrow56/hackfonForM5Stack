/***************************************************
 * Hackfon for M5Stack
 * by Team GENPEI and FutuRocket
 *
 * 2020/05/21
 * Customized for M5Stack by ThousanDIY
****************************************************/
#include <M5Stack.h>
#include "M5StackUpdater.h"

#include <WiFi.h>
// #include <Time.h>        // http://playground.arduino.cc/Code/time
#include <TimeLib.h>        // https://forum.arduino.cc/index.php?topic=415296.0
#include <DNSServer.h>
#include <WebServer.h>
#include "WiFiManager.h"   // https://github.com/zhouhan0126/WIFIMANAGER-ESP32

File myFile;

boolean debug = false;

// IFTTT key and event triggers are  
const char* server = "maker.ifttt.com";  // Server URL
String makerKey = ""; // MakerKey of Webhooks
String makerEvent = ""; // Trigger of Webhooks

String Event[12];
boolean sendFlag[12];

// Use Hackfon Logo
extern const unsigned char gImage_logoFR[];

// Interrupt Pin for dial detection
#define INTPIN 17

// Data pin  for dial detection
const int DPIN[4] = {13, 36, 35, 16};

#define onhookPin 34   // the number of the pushbutton pin

// Valiables for Status
uint8_t last_code;
unsigned long last_time = 0;
boolean send_flag = false;
boolean onhook_flag = true;

// WiFi instanse
WiFiManager wifiManager;
WiFiClient client;
IPAddress ipadr;


// read config from SD
void loadConfig() {
  String readLineDummy[100];
  String readLine = "";
  int i = 0;
  int num;

  // open the file for reading:
  myFile = SD.open("/hackfon.cfg");
  if (myFile) {
   if(debug){
     Serial.println("hackfon.cfg:");
   }

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      readLine.concat(char(myFile.read()));
      if(readLine.endsWith("\n")){
        readLineDummy[i] = readLine.substring(0, readLine.indexOf("\n", 0) - 1);
        readLine = "";
        i++;
      }
    }
    num = i;
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening hackfon.cfg");
    M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(10, 25);
    M5.Lcd.print("error opening hackfon.cfg");
  }

  String EventDummy;
  String dataDummy;
  
  for(i = 0; i < num; i++){
    if(readLineDummy[i].length() > 0){
      EventDummy = readLineDummy[i].substring(0, readLineDummy[i].indexOf("=", 0) - 1);
      EventDummy.trim();
      dataDummy = readLineDummy[i].substring(readLineDummy[i].indexOf("=", 0) + 1, readLineDummy[i].length());
      dataDummy.trim();
      if(EventDummy == "makerKey"){
        makerKey = dataDummy;
      }
      if(EventDummy == "Event1"){
        Event[0] = dataDummy;
      }
      if(EventDummy == "Event2"){
        Event[1] = dataDummy;
      }
      if(EventDummy == "Event3"){
        Event[2] = dataDummy;
      }
      if(EventDummy == "Event4"){
        Event[3] = dataDummy;
      }
      if(EventDummy == "Event5"){
        Event[4] = dataDummy;
      }
      if(EventDummy == "Event6"){
        Event[5] = dataDummy;
      }
      if(EventDummy == "Event7"){
        Event[6] = dataDummy;
      }
      if(EventDummy == "Event8"){
        Event[7] = dataDummy;
      }
      if(EventDummy == "Event9"){
        Event[8] = dataDummy;
      }
      if(EventDummy == "Event10"){
        Event[9] = dataDummy;
      }
      if(EventDummy == "Event11"){
        Event[10] = dataDummy;
      }
      if(EventDummy == "Event12"){
        Event[11] = dataDummy;
      }
      if(EventDummy == "send1"){
        if(dataDummy == "true"){
          sendFlag[0] = true;
        }else{
          sendFlag[0] = false;
        }
      }
      if(EventDummy == "send2"){
        if(dataDummy == "true"){
          sendFlag[1] = true;
        }else{
          sendFlag[1] = false;
        }
      }
      if(EventDummy == "send3"){
        if(dataDummy == "true"){
          sendFlag[2] = true;
        }else{
          sendFlag[2] = false;
        }
      }
      if(EventDummy == "send4"){
        if(dataDummy == "true"){
          sendFlag[3] = true;
        }else{
          sendFlag[3] = false;
        }
      }
      if(EventDummy == "send5"){
        if(dataDummy == "true"){
          sendFlag[4] = true;
        }else{
          sendFlag[4] = false;
        }
      }
      if(EventDummy == "send6"){
        if(dataDummy == "true"){
          sendFlag[5] = true;
        }else{
          sendFlag[5] = false;
        }
      }
      if(EventDummy == "send7"){
        if(dataDummy == "true"){
          sendFlag[6] = true;
        }else{
          sendFlag[6] = false;
        }
      }
      if(EventDummy == "send8"){
        if(dataDummy == "true"){
          sendFlag[7] = true;
        }else{
          sendFlag[7] = false;
        }
      }
      if(EventDummy == "send9"){
        if(dataDummy == "true"){
          sendFlag[8] = true;
        }else{
          sendFlag[8] = false;
        }
      }
      if(EventDummy == "send10"){
        if(dataDummy == "true"){
          sendFlag[9] = true;
        }else{
          sendFlag[9] = false;
        }
      }
      if(EventDummy == "send11"){
        if(dataDummy == "true"){
          sendFlag[10] = true;
        }else{
          sendFlag[10] = false;
        }
      }
      if(EventDummy == "send12"){
        if(dataDummy == "true"){
          sendFlag[11] = true;
        }else{
          sendFlag[11] = false;
        }
      }
    }
  }

  if(debug){
    Serial.println("makerKey = " + makerKey);
    for(i = 0; i < 12; i++){
      Serial.println("Event" + String(i + 1) + " = " +Event[i]);
      Serial.println("send" + String(i + 1) + " = " + String(sendFlag[i]));
    }
  }
}

// Send trigger to IFTTT
void sendIFTTT(String event) {
  send_flag = false;

  // If connection failed
  if (!client.connect(server, 80)){
    Serial.println("connection failed");
    return;
  }

  Serial.println("\nStarting connection to server...");
  if (!client.connect(server, 80)) {
    Serial.println("Connection failed!");
  } else {
    Serial.println("Connected to " + String(server));
    // Make a HTTP request:
    String url = "/trigger/" + event + "/with/key/" + makerKey;
    client.println("POST " + url + " HTTP/1.1");
    client.print("Host: ");
    client.println(server);

    Serial.println(url);
    delay(100);

    client.println("Connection: close");
    client.println();

    Serial.print("Waiting for response ");
    int count = 0;
    while (!client.available()) {
      delay(50); //
      Serial.print(".");
      count++;
      if (count > 20 * 20) { // about 20s
        Serial.println("(send) failed!");
        return;
      }
    }
    // if there are incoming bytes available
    // from the server, read them and print them:
    while (client.available()) {
      char c = client.read();
      Serial.write(c);
    }

    // if the server's disconnected, stop the client:
    if (!client.connected()) {
      Serial.println();
      Serial.println("disconnecting from server.");
      client.stop();
    }
  }
  client.stop();
  Serial.println("closing connection");
  Serial.println("");
}

// Read data from DTMF receiver
void readDtmfData() {
  uint8_t code = 0;
  unsigned long time;
  for (uint8_t i = 0; i < 4; i++) {
    code += (digitalRead(DPIN[i]) << i);
  }

  time = millis();
  if (time - last_time < 200) {
    last_time = time;
    if (last_code == code) {
      last_code = code;
      return;
    }
  }
  last_time = time;
  last_code = code;

  M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);

  // Event of detection result
  makerEvent = Event[code - 1];
  send_flag = sendFlag[code - 1];

  if(send_flag == true){
    M5.Lcd.setTextSize(4);
    M5.Lcd.setCursor(140, 120);
    M5.Lcd.print(code);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(10, 25);
    M5.Lcd.print(makerEvent);
  }

  Serial.println("code = " + String(code));
  Serial.println("makerEvent = " + makerEvent);
}

void showStatusBar(){
  M5.Lcd.setTextColor(GREEN);
  M5.Lcd.setCursor(0, 3);
  M5.Lcd.setTextSize(1);
  M5.Lcd.fillRect(0, 0, 320, 15, TFT_BLUE);
  M5.Lcd.println((String)ipadr[0] + "." + (String)ipadr[1] + "." + (String)ipadr[2] + "." + (String)ipadr[3]);
}

void clearScreen(){
  M5.Lcd.fillRect(0, 16, 320, 224, TFT_BLACK);
}

void setup() {
  // TFT_BL OFF
  pinMode(32, OUTPUT);
  digitalWrite(32, LOW);

  M5.begin();

  if(digitalRead(BUTTON_A_PIN) == 0) {
    Serial.println("Will Load menu binary");
    updateFromFS(SD);
    ESP.restart();
  }

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setBrightness(0);

  delay(200);

  // FutuRocket Logo
  M5.Lcd.pushImage(0, 0, 320, 240, (uint16_t *)gImage_logoFR);

  for(int i = 0; i <= 127; i++){
    M5.Lcd.setBrightness(i);
    delay(20);
  }

  delay(500);

  for(int i = 127; i >= 0; i--){
    M5.Lcd.setBrightness(i);
    delay(10);
  }

  loadConfig();

  M5.Lcd.fillScreen(BLACK);
  delay(500);
  M5.Lcd.setBrightness(127);

  Serial.println("Hackfone for M5Stack");
  Serial.println("");

  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);

  M5.Lcd.println("Hackfone by team GENPEI");
  M5.Lcd.println("");
  M5.Lcd.println(" AP name: HackfonConfigAP");
  M5.Lcd.println(" IP adrs: 192.168.4.1");

  // Display QRCode
  M5.Lcd.qrcode("http://192.168.4.1", 150, 70, 150, 6);

  // WiFiManagerのデバッグ用メッセージ表示
  wifiManager.setDebugOutput(debug);

  // WiFiManagerによる接続（接続できない場合はHackfonConfigAPというAPが立ち上がる）
  wifiManager.autoConnect("HackfonConfigAP");

   //if you get here you have connected to the WiFi
  ipadr = WiFi.localIP();
  Serial.println("connected!");
  Serial.println(WiFi.SSID());
  Serial.println(ipadr);

  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextColor(GREEN);
  M5.Lcd.setCursor(0, 0);

  M5.Lcd.setTextSize(2);

  M5.Lcd.println("Wifi Connected!");
  M5.Lcd.println("");
  M5.Lcd.println(" SSID: " + WiFi.SSID());
  M5.Lcd.println(" IP adrs: " + (String)ipadr[0] + "." + (String)ipadr[1] + "." + (String)ipadr[2] + "." + (String)ipadr[3]);

  // ピンモード設定
  for (uint8_t i = 0; i < 4; i++) {
    pinMode(DPIN[i], INPUT_PULLUP);      // 入力に設定
  }

  last_time = millis();
  last_code = 0;

  //  attachInterrupt(INTPIN, readDtmfData, RISING);
  attachInterrupt(digitalPinToInterrupt(INTPIN), readDtmfData, RISING);

  // initialize the onhook pin as an input:
  pinMode(onhookPin, INPUT);

  delay(1500);
  M5.Lcd.fillScreen(TFT_BLACK);
  showStatusBar();
  M5.Lcd.setTextColor(GREEN);
}

void loop() {
  if(onhook_flag != !digitalRead(onhookPin)){
    if(onhook_flag == true){
      Serial.println("on-hook");
      M5.Lcd.fillRect(300, 3, 10, 10, TFT_BLUE);
      clearScreen();
    }else{
      Serial.println("off-hook");
      M5.Lcd.fillRect(300, 3, 10, 10, TFT_MAGENTA);
    }
    onhook_flag = !digitalRead(onhookPin);
  }
  if (send_flag) {
    noInterrupts();
    Serial.println("send command");
    M5.Lcd.setTextSize(3);
    M5.Lcd.setCursor(40, 80);
    M5.Lcd.print("send command");
    sendIFTTT(makerEvent);
    send_flag = false;
    clearScreen();
    interrupts();
  }
  delay(50);
}
