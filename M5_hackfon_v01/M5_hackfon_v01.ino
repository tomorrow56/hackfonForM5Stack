/***************************************************
 * Hackfon for M5Stack
 * by Team GENPEI
 *
 * 2019/02/02
 * Customized for M5Stack by ThousanDIY
****************************************************/
#include "ArduinoJson.h"

#include <M5Stack.h>
#include "M5StackUpdater.h"

#include <WiFi.h>
// #include <Time.h>        // http://playground.arduino.cc/Code/time
#include <TimeLib.h>        // https://forum.arduino.cc/index.php?topic=415296.0
#include <DNSServer.h>      // https://github.com/zhouhan0126/DNSServer---esp32
#include <WebServer.h>      // https://github.com/zhouhan0126/WebServer-esp32
#include "WiFiManager.h"   // https://github.com/zhouhan0126/WIFIMANAGER-ESP32

// Use Hackfon Logo
extern const unsigned char gImage_logoFR[];

// Interrupt Pin for dial detection
#define INTPIN 17

// Data pin  for dial detection
const int DPIN[4] = {13, 36, 35, 16};

#define onhookPin 34   // the number of the pushbutton pin

// コマンドバッファ
uint8_t command_buffer[10] = {};
uint8_t received_length = 0;
boolean command_mode = false;

const uint8_t COMMANDMAXLEN = 10;

// GENPEI WebAPI設定
const char* DEST_HOST = "192.168.11.108";
const uint16_t DEST_PORT = 80;
const char* DEST_URL = "/hue/api.php?command=";

// レスポンス用バッファ
const int BUFFER_SIZE = JSON_OBJECT_SIZE(100) + JSON_ARRAY_SIZE(100);
StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;

// 状態保持用変数
uint8_t last_code;
unsigned long last_time = 0;
boolean send_flag = false;
boolean onhook_flag = true;

// WiFi系インスタンス
WiFiManager wifiManager;
WiFiClient client;

/*
 * バッファオーバーフローエラーの処理
 */
void bof_error (void) {
  Serial.print("BOF error: ");
  for (uint8_t i = 0; i < COMMANDMAXLEN; i++) {
      Serial.print(command_buffer[i]);
  }
  Serial.println("");
  received_length = 0;
  command_mode = false;
}

/*
 * コマンドを送信する
 */
void send_command (void) {
  String line;
  String url;
  if (received_length > 0) {
    // URLの生成と受信コマンドの表示
    url = String(DEST_URL);
    Serial.print("command: ");
    for (uint8_t i = 0; i < received_length; i++) {
      url += command_buffer[i];
      Serial.print(command_buffer[i]);
    }
    Serial.println("");
    // 送信フラグを戻す
    send_flag = false;
    // バッファの初期化
    received_length = 0;
    command_mode = false;

    if (!client.connect(DEST_HOST, DEST_PORT)) {
      // 送信のための接続に失敗した時
      Serial.println("connection failed");
      return;
    }

    client.print(String("GET ") + url + String(" HTTP/1.1\r\n") +
      String("Host: ") + String(DEST_HOST) + String("\r\n") +
      String("Connection: close\r\n\r\n"));
    delay(500);

//    //get rid of the HTTP headers
//    while(client.available()){
//      line = client.readStringUntil('\r');
//      Serial.print(line);
//      line.trim();
//      if (line.length() == 0) {
//        break;
//      }
//    }
//
//    //get http content
//    String buffer="";
//    while(client.available()){
//      line = client.readStringUntil('\r');
//      line.trim();
//      buffer.concat(line);
//    }
client.stop();
    //parse json data
//    char json[buffer.length() + 1];
//    buffer.toCharArray(json, sizeof(json));
//    Serial.println(json);
//    JsonObject& root = jsonBuffer.parseObject(json);
//    if (!root.success()) {
//      Serial.println("parseObject() failed");
//      return;
//    }

    Serial.println("closing connection");
    Serial.println("");
  }
}

/*
 * DTMFレシーバからデータ取り込み
 */
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

  // コマンドの認識
  switch (code) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
      if (command_mode && !send_flag) {
        if (received_length >COMMANDMAXLEN) {
          send_command();
          bof_error();
        } else {
          command_buffer[received_length] = code % 10;
          received_length++;
          if (received_length == 4) {
            send_flag = true;
          }
        }
      }
      break;
    case 11:
      Serial.println("command start");
      M5.Lcd.setTextSize(3);
      M5.Lcd.setCursor(40, 80);
      M5.Lcd.print("             ");
      M5.Lcd.setCursor(40, 80);
      M5.Lcd.print("command start");

//      digitalWrite(ledOnline, true);
      received_length = 0;
      command_mode = true;
      break;
    case 12:
      send_command();
      send_flag = true;
      break;
  }
  Serial.println(code);
  M5.Lcd.setTextSize(4);
  M5.Lcd.setCursor(140, 120);
  M5.Lcd.print("  ");
  M5.Lcd.setCursor(140, 120);
  M5.Lcd.print(code);
}

void setup() {
  String line;
  String url;

  // TFT_BL OFF
  pinMode(32, OUTPUT);
  digitalWrite(32, LOW);

  M5.begin();

  if(digitalRead(BUTTON_A_PIN) == 0) {
    Serial.println("Will Load menu binary");
//    updateFromFS(SD, fileInfo[ M5Menu.getListID() ].fileName);
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

  delay(1000);

  for(int i = 127; i >= 0; i--){
    M5.Lcd.setBrightness(i);
    delay(10);
  }

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
  wifiManager.setDebugOutput(true);

  // WiFiManagerによる接続（接続できない場合はHackfonConfigAPというAPが立ち上がる）
  wifiManager.autoConnect("HackfonConfigAP");

   //if you get here you have connected to the WiFi
  IPAddress ipadr = WiFi.localIP();
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
}

void loop() {
  if(onhook_flag != !digitalRead(onhookPin)){
    if(onhook_flag == true){
      Serial.println("on-hook");
      M5.Lcd.fillRect(300, 10, 10, 10, TFT_BLACK);
      M5.Lcd.setCursor(40, 80);
      M5.Lcd.print("             ");
      M5.Lcd.setCursor(140, 120);
      M5.Lcd.print("  ");
    }else{
      Serial.println("off-hook");
      M5.Lcd.fillRect(300, 10, 10, 10, TFT_MAGENTA);
    }
    onhook_flag = !digitalRead(onhookPin);
  }
  if (send_flag) {
    noInterrupts();
    Serial.println("send command");

    M5.Lcd.setTextSize(3);
    M5.Lcd.setCursor(40, 80);
    M5.Lcd.print("             ");
    M5.Lcd.setCursor(40, 80);
    M5.Lcd.print("send command");
    send_command();
//    digitalWrite(ledOnline, false);
    send_flag = false;
    interrupts();
  }
  delay(50);
}
