#include <esp_now.h>
#include <WiFi.h>
#include "ESP32_New_TimerInterrupt.h"

// Pin-Definitionen \\

#define Enable_Pin 1
#define Menue_Pin 2
#define R_Led_Pin 3
#define G_Led_Pin 4
#define B_Led_Pin 5  
#define Receiver_Pin 6
#define IR_Pin 7

// Funkübertragung \\

uint8_t mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};                             // MAC-Adresse Broadcast
uint8_t num = 1;
bool success = false;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)            // Prüfung der Sendung
{
  if(status == ESP_NOW_SEND_SUCCESS) success = true;
  else success = false;
}

// Timer und Interrupts \\

volatile bool state = true;
int i = 0;

ESP32Timer ITimer0(0);                                  // Timer0 initialisieren
 
bool IRAM_ATTR TimerHandler0(void * timerNo)            // Timer-Handler für Timer0
{
  if(state)
  {
    LED(0);
    state = false;
  }
  else
  {
    LED(num);
    state = true;
  }
  return true;
}

void setup()
{
  // Pin-Initialisierung \\

  pinMode(Enable_Pin, OUTPUT);
  digitalWrite(Enable_Pin, HIGH);
  pinMode(Menue_Pin, INPUT);
  pinMode(R_Led_Pin, OUTPUT);
  digitalWrite(R_Led_Pin, HIGH);
  pinMode(G_Led_Pin, OUTPUT);
  digitalWrite(G_Led_Pin, HIGH);
  pinMode(B_Led_Pin, OUTPUT);
  digitalWrite(B_Led_Pin, HIGH);
  pinMode(Receiver_Pin, INPUT);
  pinMode(IR_Pin, OUTPUT);

  // Block für WLAN-Verbindung
  
  WiFi.mode(WIFI_STA);                      // Wifi im Station-Mode
  esp_now_init();                           // ESP-NOW initialisieren

  esp_now_register_send_cb(OnDataSent);     // Senden aktivieren

  esp_now_peer_info_t peerInfo;             // Verbindung mit Server aufbauen
  memcpy(peerInfo.peer_addr, mac, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
  LED(num);
}

void loop()
{ 
  esp_now_send(mac, &num, sizeof(num));                                   // Senden an alle Clients
  delay(10000);
  if(success) ITimer0.attachInterruptInterval(1000000, TimerHandler0);    // Aktiviere Timer0
  else
  {
    ITimer0.stopTimer();
    LED(num);
    state = true;
  }
  if(digitalRead(Menue_Pin) == HIGH) i++;
  else i = 0;
  if(i == 2) digitalWrite(Enable_Pin, LOW);
}

void LED(uint8_t i)
{
  if(i == 0)                      //Aus
  {
    digitalWrite(R_Led_Pin, HIGH);
    digitalWrite(G_Led_Pin, HIGH);
    digitalWrite(B_Led_Pin, HIGH);
  }
  else if(i == 1)                 //Rot
  {
    digitalWrite(R_Led_Pin, LOW);
    digitalWrite(G_Led_Pin, HIGH);
    digitalWrite(B_Led_Pin, HIGH);
  }
  else if(i == 2)                 //Grün
  {
    digitalWrite(R_Led_Pin, HIGH);
    digitalWrite(G_Led_Pin, LOW);
    digitalWrite(B_Led_Pin, HIGH);
  }
  else if(i == 3)                 //Blau
  {
    digitalWrite(R_Led_Pin, HIGH);
    digitalWrite(G_Led_Pin, HIGH);
    digitalWrite(B_Led_Pin, LOW);
  }
  else if(i == 4)                 //Gelb
  {
    digitalWrite(R_Led_Pin, LOW);
    digitalWrite(G_Led_Pin, LOW);
    digitalWrite(B_Led_Pin, HIGH);
  }
  else if(i == 5)                 //Cyan
  {
    digitalWrite(R_Led_Pin, HIGH);
    digitalWrite(G_Led_Pin, LOW);
    digitalWrite(B_Led_Pin, LOW);
  }
  else if(i == 6)                 //Magenta
  {
    digitalWrite(R_Led_Pin, LOW);
    digitalWrite(G_Led_Pin, HIGH);
    digitalWrite(B_Led_Pin, LOW);
  }
  else if(i == 7)                 //Weiß
  {
    digitalWrite(R_Led_Pin, LOW);
    digitalWrite(G_Led_Pin, LOW);
    digitalWrite(B_Led_Pin, LOW);
  }
}
