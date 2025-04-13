#include "CTBot.h"
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFiClient.h> // Tambahkan library WiFiClient
#include <ESP8266HTTPClient.h>
#include "HTTPSRedirect.h"
#include <WiFiClientSecureBearSSL.h>

#define TRIGGER_PIN D2
#define ECHO_PIN D1
#define SERVO_PIN D3

Servo myservo;
CTBot myBot;

const char* ssid = "Flip";
const char* password = "12345678";
const char* botToken = "6996511954:AAHyA1BNGXLxRTdbPzkpZcnWn7FGMlg8orI";
const int id = 1914599897;

int kucing = 0;
int baru = 0;
String current;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

void setup() {
  Serial.begin(9600);
  Serial.println("Memulai telegram BOT :");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Menghubungkan ke wifi...");
  }

  myBot.setTelegramToken(botToken);

  if (myBot.testConnection())
    Serial.println("Koneksi Berhasil");
  else
    Serial.println("Koneksi Gagal");

  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  myservo.attach(SERVO_PIN);

  timeClient.begin();
  timeClient.setTimeOffset(25200); 
}



void kirimDataGsheet(int kucing) {

  if (WiFi.status() == WL_CONNECTED) {

    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    client->setInsecure();


    HTTPClient https;
    String url = "https://script.google.com/macros/s/AKfycbzCgRO1zpBXQgM89PCu7IudIDmEaDszMI1Wu359pbPayb148cSrlEg1VhfFucp7Zy7k/exec";
    String datanya = url + "?kucing=" + String(kucing); ;
    Serial.println(datanya);
    Serial.print("[HTTPS] begin...\n");
    if (https.begin(*client, datanya.c_str())) { 
      int httpCode = https.GET();

      if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }
}



void loop() {
  long duration, distance;
  TBMessage msg;

  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.034 / 2;


  if (distance < 10) {
    myservo.write(180);
    delay(800);
    myservo.write(90);
    kucing++;
    String currentTime = String(timeClient.getHours()) + ":" + String(timeClient.getMinutes()) + ":" + String(timeClient.getSeconds()); 
    current = currentTime;
    delay(1000);

    // Memanggil fungsi untuk memperbarui spreadsheet setiap kali kucing++ terjadi
    // updateGoogleSheet();
    kirimDataGsheet(kucing);
  }

  timeClient.update();
  if (myBot.getNewMessage(msg)) {
    Serial.println("Pesan Masuk : " + msg.text);

    String pesan = msg.text;
    if (pesan == "menu") {
      myBot.sendMessage(id, "Menu : \n1 = Pemberian makan sekarang  \n2 = Berapa kali kucing makan \n3 = Notifikasi tempat makan kucing \n4 = Reset hitungan makan kucing \n5 = Log history makanan");
    }
    if (pesan == "1") {
      myservo.write(180);
      delay(800);
      myservo.write(90);
      kucing++;
      String currentTime = String(timeClient.getHours()) + ":" + String(timeClient.getMinutes()) + ":" + String(timeClient.getSeconds()); 
      current = currentTime;
      myBot.sendMessage(id, "Pemberian makan berhasil");

      // Memanggil fungsi untuk memperbarui spreadsheet setiap kali kucing++ terjadi
      //updateGoogleSheet();
      kirimDataGsheet(kucing);
    }
    if (pesan == "2") {
      String teks = "Kucing baru makan ";
      String total = teks + kucing + "x";
      myBot.sendMessage(id, total);
    }
    if (pesan == "3") {
      // Periksa status kucing dan kirim notifikasi sesuai dengan persentase makanan yang tersisa
      if (kucing > 10) {
        myBot.sendMessage(id, "makan kucing tinggal 0%");
      }
      else if (kucing == 9) {
        myBot.sendMessage(id, "makan kucing tinggal 10%");
      }
      else if (kucing == 8) {
        myBot.sendMessage(id, "makan kucing tinggal 20%");
      }
      else if (kucing == 7) {
        myBot.sendMessage(id, "makan kucing tinggal 30%");
      }
      else if (kucing == 6) {
        myBot.sendMessage(id, "makan kucing tinggal 40%");
      }
      else if (kucing < 6) {
        myBot.sendMessage(id, "makan kucing masih ada");
      }
    }

    if (pesan == "4") {
      myBot.sendMessage(id, "Hitungan makanan udah di reset");
      kucing = 0;
    }

    if (pesan == "5") {
      if (kucing != baru){
        baru = kucing;
        myBot.sendMessage(id, "Kucing terakhir kali makan pada : " + current);
      }
      else if (kucing == 0){
        myBot.sendMessage(id, "Kucing belum makan ");
      }
      else if (kucing == baru){
        myBot.sendMessage(id, "Kucing terakhir kali makan pada : " + current);
      }
    }
  }
  delay(2000);
}