#include "driver/i2s.h"
#include "WavData.h"
#include "data.h"
#include "PCF2119.h"
#include "ESP32_New_TimerInterrupt.h"
#include <esp_now.h>
#include <WiFi.h>

// Pin-Definitionen \\

#define Trigger_Pin 1
#define Receiver_Pin 2
#define Menue_Pin 3
#define R_Led_Pin 4
#define G_Led_Pin 5
#define B_Led_Pin 6
#define Laser_Pin 7
#define Display_SDA 8
#define Display_SCL 9
#define Display_Reset 10
#define IR_Pin 11
#define Enable_Pin 12
#define I2S_BCLK_Pin 13
#define I2S_LRCL_Pin 14
#define I2S_DOUT_Pin 15

// I2S-Audio Definition \\

static const i2s_port_t i2s_num = I2S_NUM_0;  // I2S Port Nummer
unsigned const char* TheData1;                // Schuss-Sound
unsigned const char* TheData2;                // Schnellfeuer-Sound
unsigned const char* TheData3;                // Damage-Sound
unsigned const char* TheData4;                // Magazin-Leer-Sound
unsigned const char* TheData5;                // Magazin-Laden-Sound
uint32_t DataIdx=0;                           // Offset in TheData

struct WavHeader_Struct
{
  //   RIFF Section    
  char RIFFSectionID[4];          // Letters "RIFF"
  uint32_t Size;                  // Size of entire file less 8
  char RiffFormat[4];             // Letters "WAVE"
      
  //   Format Section    
  char FormatSectionID[4];        // letters "fmt"
  uint32_t FormatSize;            // Size of format section less 8
  uint16_t FormatID;              // 1=uncompressed PCM
  uint16_t NumChannels;           // 1=mono,2=stereo
  uint32_t SampleRate;            // 44100, 16000, 8000 etc.
  uint32_t ByteRate;              // =SampleRate * Channels * (BitsPerSample/8)
  uint16_t BlockAlign;            // =Channels * (BitsPerSample/8)
  uint16_t BitsPerSample;         // 8,16,24 or 32
   
  // Data Section
  char DataSectionID[4];          // The letters "data"
  uint32_t DataSize;              // Size of the data that follows
}WavHeader1, WavHeader2, WavHeader3, WavHeader4, WavHeader5;
    
static const i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 44100,                                         // Note, this will be changed later
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,                     // hohe Interrupt-Priorität
    .dma_buf_count = 8,                                           // 8 Puffer-Speicher
    .dma_buf_len = 1024,                                          // 1K pro Puffer -> 8k 
    .use_apll=0,
    .tx_desc_auto_clear= true, 
    .fixed_mclk=-1    
};

static const i2s_pin_config_t pin_config = {
    .bck_io_num = 13,                                   // Pin für Bit-Clock
    .ws_io_num = 14,                                    // Pin für Word Select
    .data_out_num = 15,                                 // Pin für Data Out
    .data_in_num = I2S_PIN_NO_CHANGE                    // I2S Eingang -> Uninteressant
};

// Display-Definition \\

pcf2119 display = pcf2119(Display_Reset);               // Erzeugung des Display-Objektes

// Globale Variablen \\

Daten x;                                                // Struktur für alle Spiel-relevanten Daten
int Status = LOW, Status_alt = LOW;                     // Zustandsvariablen für Trigger
uint8_t Magazin;                                        // Anzahl der Schüsse im Magazin
volatile uint8_t Leben;					// Anzahl der erlaubten Treffer
volatile uint8_t dauer = -1;                            // Dauer des Spiels bisher in Minuten
volatile int reload = -1;                               // Variable zur Koordination des Nachlade-Zyklus
volatile unsigned long ID[22];                          // Variable für Treffer-Identifikation
volatile int count = 0;
volatile bool change = false;
uint8_t beacon = 0;
bool success1 = false;                                  // Zustand der empfangenen Daten
bool success2 = false;
uint8_t mac0[] = {0x7C, 0xDF, 0xA1, 0x40, 0xCC, 0xD8};  // MAC-Adresse des Servers
int press = 0;                                          // Zustandsvariable für Beenden des Spiels

// Timer und Interrupts \\

ESP32Timer ITimer0(0);                                  // Timer0 initialisieren
 
bool IRAM_ATTR TimerHandler0(void * timerNo)            // Timer-Handler für Timer0
{
  dauer++;
  return true;
}

ESP32Timer ITimer1(1);                                  // Timer1 initialisieren
 
bool IRAM_ATTR TimerHandler1(void * timerNo)            // Timer-Handler für Timer1
{
  reload++;
  return true;
}

ESP32Timer ITimer2(2);                                  // Timer2 initialisieren
 
bool IRAM_ATTR TimerHandler2(void * timerNo)            // Timer-Handler für Timer2
{
  if(Leben < x.Leben_Reload) Leben++;
  if(Leben == x.Leben_Reload) ITimer2.stopTimer();
  change = true;
  return true;
}

void IRAM_ATTR ISR()                                    // ISR-Handler für IR-Receiver
{
    if(count < 22)
    {
      ID[count] = micros();
      count++;
    }
    else
    {
      count = 0;
      detachInterrupt(Receiver_Pin);
    }
}

// Funkübertragung \\

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len)        // Verarbeitung empfangener Daten
{
  if(len > 8)
  {
    memcpy(&x, incomingData, sizeof(x));
    success1 = true;
  }
  else
  {
    memcpy(&beacon, incomingData, sizeof(beacon));
    success2 = true;
  }
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)            // Prüfung der Sendung
{
  if(status == ESP_NOW_SEND_SUCCESS) success1 = true;
  else success1 = false;
}

void setup()
{
  // Pin-Initialisierung \\

  pinMode(Trigger_Pin, INPUT);
  pinMode(Receiver_Pin, INPUT);
  pinMode(Menue_Pin, INPUT);
  pinMode(R_Led_Pin, OUTPUT);
  digitalWrite(R_Led_Pin, LOW);
  pinMode(G_Led_Pin, OUTPUT);
  digitalWrite(G_Led_Pin, LOW);
  pinMode(B_Led_Pin, OUTPUT);
  digitalWrite(B_Led_Pin, LOW);
  pinMode(Laser_Pin, OUTPUT);
  digitalWrite(Laser_Pin, LOW);
  pinMode(IR_Pin, OUTPUT);
  pinMode(Enable_Pin, OUTPUT);
  digitalWrite(Enable_Pin, HIGH);
  pinMode(I2S_BCLK_Pin, OUTPUT);
  pinMode(I2S_LRCL_Pin, OUTPUT);
  pinMode(I2S_DOUT_Pin, OUTPUT);

  // PWM-Erzeugung für IR-Sender\\

  ledcSetup(0, 38000, 8);
  ledcAttachPin(IR_Pin, 0);

  // Wav-Daten vorbereiten \\
  
  memcpy(&WavHeader1,&schuss1,44);                            // Kopiere den Haeder von schuss1 in die Struktur
  memcpy(&WavHeader2,&schuss2,44);                            // Kopiere den Haeder von schuss2 in die Struktur
  memcpy(&WavHeader3,&treffer,44);                            // Kopiere den Haeder von treffer in die Struktur
  memcpy(&WavHeader4,&leer,44);                               // Kopiere den Haeder von leer in die Struktur
  memcpy(&WavHeader5,&laden,44);                              // Kopiere den Haeder von laden in die Struktur
  i2s_driver_install(i2s_num, &i2s_config, 0, NULL);          // ESP32 stellt Ressourcen für I2S
  i2s_set_pin(i2s_num, &pin_config);                          // Pin-Zuweisung
  i2s_set_sample_rates(i2s_num, WavHeader1.SampleRate);       // Setze Samplerate
  TheData1=schuss1;                                           // Setze Anfangspunkt für schuss1
  TheData1+=44;
  TheData2=schuss2;                                           // Setze Anfangspunkt für schuss2
  TheData2+=44;
  TheData3=treffer;                                           // Setze Anfangspunkt treffer
  TheData3+=44;
  TheData4=leer;                                              // Setze Anfangspunkt leer
  TheData4+=44;
  TheData5=laden;                                             // Setze Anfangspunkt laden
  TheData5+=44;

  // Display-Initialisierung \\

  display.set_hv_pump(PCF2119_VLCD_GENERATOR_STAGES_2);       // Ladungspumpe für 3,3V VCC konfigurieren
  display.set_mux(PCF2119_MUX_1_18);
  display.init();                                             // Initialisieren des Displays

  // Block für WLAN-Verbindung
  
  WiFi.mode(WIFI_STA);                      // Wifi im Station-Mode
  esp_now_init();                           // ESP-NOW initialisieren

  esp_now_register_send_cb(OnDataSent);     // Senden aktivieren

  esp_now_peer_info_t peerInfo;             // Verbindung mit Server aufbauen
  memcpy(peerInfo.peer_addr, mac0, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  esp_now_register_recv_cb(OnDataRecv);     // Empfang aktivieren
  
  {
    int i = 0;
    while((success1 == false)&&(i < 300))
    {
      delay(100);
      i++;
    }
    if(i == 300) digitalWrite(Enable_Pin, LOW);
  }
  x.num = 8;
  Magazin = x.Magazin_Reload;
  Leben = x.Leben_Reload;
  success1 = false;

  // Anfangsprozedur für das Spiel \\

  for(int i = 30; i > 0; i--)               // Countdown auf dem Display
  {
    display.clear_screen();
    display.return_home();
    display.set_ramposition(PCF2119_DISPLAY_START_POSITION);
    display.printf("Noch %ds", i);

    delay(1000);
  }

  display.clear_screen();
  display.return_home();
  display.set_ramposition(PCF2119_DISPLAY_START_POSITION);
  display.printf("Go");

  delay(500);

  display.clear_screen();                                     // Anzeigen des Spielernamens auf dem Display
  display.return_home();
  display.set_ramposition(PCF2119_DISPLAY_START_POSITION);
  display.printf("%-10s Lives: %d", x.name9.c_str(), Leben);

  LED(x.Teamfarbe[x.num]);                                    // Team-LED ein 

  attachInterrupt(Receiver_Pin, ISR, CHANGE);
        
  ITimer0.attachInterruptInterval(60000000, TimerHandler0);   // Aktiviere Timer0, zuständig für die Dauer des Spiels
}

void loop()
{ 
  if(dauer < x.Laenge)                      // Dauer des Spieles wird geprüft
  {                                               
    if(((ID[1]-ID[0]) > 900)&&((ID[1]-ID[0]) < 1100)&&((ID[2]-ID[1]) > 400)&&((ID[2]-ID[1]) < 600)&&((ID[21]-ID[20]) > 400)&&((ID[21]-ID[20]) < 600))       // Validieren des Start- und Stop-Bits
    {
      bool stat[9];
      for (int i = 2; i < 19; i+=2)        // Decodieren
      {
         if(((ID[i+1]-ID[i]) > 400)&&((ID[i+1]-ID[i]) < 600)&&((ID[i+2]-ID[i+1]) > 400)&&((ID[i+2]-ID[i+1]) < 600)) stat[i/2-1] = false;
         else if(((ID[i+1]-ID[i]) > 600)&&((ID[i+1]-ID[i]) < 800)&&((ID[i+2]-ID[i+1]) > 200)&&((ID[i+2]-ID[i+1]) < 400)) stat[i/2-1] = true;
      }
      if(stat[0]&&stat[1]&&stat[2]&&stat[3]&&stat[4]&&stat[5]&&stat[6]&&stat[7]&&stat[8]&&x.name10!="")
      {
        if((x.Teamfarbe[9] == x.Teamfarbe[x.num])&&x.Aktiv_Friendlyfire&&(x.Modus != 2)) Hit(9);
        else if(x.Teamfarbe[9] != x.Teamfarbe[x.num]) Hit(9);
      }
      else if(stat[0]&&stat[1]&&stat[2]&&stat[3]&&stat[4]&&stat[5]&&stat[6]&&!stat[7]&&!stat[8]&&x.name8!="")
      {
        if((x.Teamfarbe[7] == x.Teamfarbe[x.num])&&x.Aktiv_Friendlyfire&&(x.Modus != 2)) Hit(7);
        else if(x.Teamfarbe[7] != x.Teamfarbe[x.num]) Hit(7);
      }
      else if(stat[0]&&stat[1]&&stat[2]&&stat[3]&&stat[4]&&stat[5]&&!stat[6]&&!stat[7]&&!stat[8]&&x.name7!="")
      {
        if((x.Teamfarbe[6] == x.Teamfarbe[x.num])&&x.Aktiv_Friendlyfire&&(x.Modus != 2)) Hit(6);
        else if(x.Teamfarbe[6] != x.Teamfarbe[x.num]) Hit(6);
      }
      else if(stat[0]&&stat[1]&&stat[2]&&stat[3]&&stat[4]&&!stat[5]&&!stat[6]&&!stat[7]&&!stat[8]&&x.name6!="")
      {
        if((x.Teamfarbe[5] == x.Teamfarbe[x.num])&&x.Aktiv_Friendlyfire&&(x.Modus != 2)) Hit(5);
        else if(x.Teamfarbe[5] != x.Teamfarbe[x.num]) Hit(5);
      }
      else if(stat[0]&&stat[6]&&stat[2]&&stat[3]&&!stat[4]&&!stat[5]&&!stat[6]&&!stat[7]&&!stat[8]&&x.name5!="")
      {
        if((x.Teamfarbe[4] == x.Teamfarbe[x.num])&&x.Aktiv_Friendlyfire&&(x.Modus != 2)) Hit(4);
        else if(x.Teamfarbe[4] != x.Teamfarbe[x.num]) Hit(4);
      }
      else if(stat[0]&&stat[1]&&stat[2]&&!stat[3]&&!stat[4]&&!stat[5]&&!stat[6]&&!stat[7]&&!stat[8]&&x.name4!="")
      {
        if((x.Teamfarbe[3] == x.Teamfarbe[x.num])&&x.Aktiv_Friendlyfire&&(x.Modus != 2)) Hit(3);
        else if(x.Teamfarbe[3] != x.Teamfarbe[x.num]) Hit(3);
      }
      else if(stat[0]&&stat[1]&&!stat[2]&&!stat[3]&&!stat[4]&&!stat[5]&&!stat[6]&&!stat[7]&&!stat[8]&&x.name3!="")
      {
        if((x.Teamfarbe[2] == x.Teamfarbe[x.num])&&x.Aktiv_Friendlyfire&&(x.Modus != 2)) Hit(2);
        else if(x.Teamfarbe[2] != x.Teamfarbe[x.num]) Hit(2);
      }
      else if(stat[0]&&!stat[1]&&!stat[2]&&!stat[3]&&!stat[4]&&!stat[5]&&!stat[6]&&!stat[7]&&!stat[8]&&x.name2!="")
      {
        if((x.Teamfarbe[1] == x.Teamfarbe[x.num])&&x.Aktiv_Friendlyfire&&(x.Modus != 2)) Hit(1);
        else if(x.Teamfarbe[1] != x.Teamfarbe[x.num]) Hit(1);
      }
      else if(!stat[0]&&!stat[1]&&!stat[2]&&!stat[3]&&!stat[4]&&!stat[5]&&!stat[6]&&!stat[7]&&!stat[8]&&x.name1!="")
      {
        if((x.Teamfarbe[0] == x.Teamfarbe[x.num])&&x.Aktiv_Friendlyfire&&(x.Modus != 2)) Hit(0);
        else if(x.Teamfarbe[0] != x.Teamfarbe[x.num]) Hit(0);
      }
    }
    attachInterrupt(Receiver_Pin, ISR, CHANGE);
    if((micros()-ID[0]) > 12000)          // Reset unvollständige Daten bei Timeout
    {
      for (int i = 0; i < 22; i++)      // Reset IR-Empfangsbuffer
      {
        ID[i] = 0;
      }
      count = 0;
    }
    if(change)
    {
      change = false;
      display.clear_screen();
      display.return_home();
      display.set_ramposition(PCF2119_DISPLAY_START_POSITION);
      display.printf("%-10s Lives: %d", x.name9.c_str(), Leben);
    }
    if(success2&&(beacon != x.Teamfarbe[x.num])&&(beacon > 0)&&(beacon < 7)&&(x.Modus == 2))
    {
      x.Deaths[x.num]++;
      success2 = false;
      beacon = 0;
    }
    if(Magazin > 0)                         // Schuss nur wenn noch Munition im Magazin ist
    {
      if(x.Full_Auto)                       // Schnellfeuer
      {
        if(digitalRead(Trigger_Pin) == HIGH) Shoot();
      }
      else
      {
        Status_alt = Status;
        Status = digitalRead(Trigger_Pin);
        if((Status == HIGH)&&(Status_alt == LOW))
        {
          delay(1);
          if(digitalRead(Trigger_Pin) == HIGH) Shoot();
        }
      } 
    }
    else
    {
      Status_alt = Status;
      Status = digitalRead(Trigger_Pin);
      if((Status == HIGH)&&(Status_alt == LOW))
      {
        delay(1);
        if(digitalRead(Trigger_Pin) == HIGH)
        {
          while(DataIdx<=WavHeader4.DataSize)
          {
            size_t BytesWritten;                            
            i2s_write(i2s_num,TheData4+DataIdx,4,&BytesWritten,portMAX_DELAY); 
            DataIdx+=4;
          }
          DataIdx=0;
        }
      }
    }

// Magazin-Reload \\

    if((digitalRead(Menue_Pin) == HIGH)&&(reload == -1)&&x.Aktiv_Magazin)
    {
      reload = 0;
      ITimer1.attachInterruptInterval(1000000, TimerHandler1);
    }
    else if(reload == 5)
    {
      ITimer1.stopTimer();
      Magazin = x.Magazin_Reload;
      reload = -1;
      while(DataIdx<=WavHeader5.DataSize)
      {
        size_t BytesWritten;                            
        i2s_write(i2s_num,TheData5+DataIdx,4,&BytesWritten,portMAX_DELAY); 
        DataIdx+=4;
      }
      DataIdx=0;
      LED(x.Teamfarbe[x.num]);
    }
    else if((reload == 4)||(reload == 2))                    // Team-LED blinkt solang nachgeladen wird
    {
      LED(x.Teamfarbe[x.num]);
    }
    else if((reload == 3)||(reload == 1))
    {
      LED(0);
    }
    if((digitalRead(Trigger_Pin) == HIGH)&&(digitalRead(Menue_Pin) == HIGH))
    {
      press++;
      delay(10);
    }
    else press = 0;
    if(press == 500) digitalWrite(Enable_Pin, LOW);
  }
  else
  {
    display.clear_screen();
    display.return_home();
    display.set_ramposition(PCF2119_DISPLAY_START_POSITION);
    display.printf("Game Over!");

    LED(0);                                                               // Team-LED aus

    ITimer0.stopTimer();
    detachInterrupt(Receiver_Pin);

    delay(2800);

    esp_now_send(mac0, (uint8_t *) &x, sizeof(x));                        // Senden an Server
    int i = 0;
    while((success1 == false)&&(i < 600))                                  // Versuche erneut zu senden
    {
      delay(1000);
      i++;
      esp_now_send(mac0, (uint8_t *) &x, sizeof(x));
    }
  
    digitalWrite(Enable_Pin, LOW);                                        // Waffe aus
  }                              
}

void LED(uint8_t i)
{
  if(i == 0)                      //Aus
  {
    digitalWrite(R_Led_Pin, LOW);
    digitalWrite(G_Led_Pin, LOW);
    digitalWrite(B_Led_Pin, LOW);
  }
  else if(i == 1)                 //Rot
  {
    digitalWrite(R_Led_Pin, HIGH);
    digitalWrite(G_Led_Pin, LOW);
    digitalWrite(B_Led_Pin, LOW);
  }
  else if(i == 2)                 //Grün
  {
    digitalWrite(R_Led_Pin, LOW);
    digitalWrite(G_Led_Pin, HIGH);
    digitalWrite(B_Led_Pin, LOW);
  }
  else if(i == 3)                 //Blau
  {
    digitalWrite(R_Led_Pin, LOW);
    digitalWrite(G_Led_Pin, LOW);
    digitalWrite(B_Led_Pin, HIGH);
  }
  else if(i == 4)                 //Gelb
  {
    digitalWrite(R_Led_Pin, HIGH);
    digitalWrite(G_Led_Pin, HIGH);
    digitalWrite(B_Led_Pin, LOW);
  }
  else if(i == 5)                 //Cyan
  {
    digitalWrite(R_Led_Pin, LOW);
    digitalWrite(G_Led_Pin, HIGH);
    digitalWrite(B_Led_Pin, HIGH);
  }
  else if(i == 6)                 //Magenta
  {
    digitalWrite(R_Led_Pin, HIGH);
    digitalWrite(G_Led_Pin, LOW);
    digitalWrite(B_Led_Pin, HIGH);
  }
  else if(i == 7)                 //Weiß
  {
    digitalWrite(R_Led_Pin, HIGH);
    digitalWrite(G_Led_Pin, HIGH);
    digitalWrite(B_Led_Pin, HIGH);
  }
}

void Shoot()
{
  digitalWrite(Laser_Pin, HIGH);    // Laser ein
  ledcWrite(0, 127);
  delayMicroseconds(1000);
  ledcWrite(0, 0);
  delayMicroseconds(500);
  
  for (int i = 0; i < 10; i++)
  {
    if(i < x.num)
    {
      ledcWrite(0, 127);
      delayMicroseconds(700);
      ledcWrite(0, 0);
      delayMicroseconds(300);
    }
    else
    {
      ledcWrite(0, 127);
      delayMicroseconds(500);
      ledcWrite(0, 0);
      delayMicroseconds(500);
    }
  }
  digitalWrite(Laser_Pin, LOW);     // Laser aus
  
  for (int i = 0; i < 22; i++)      // Reset IR-Empfangsbuffer
  {
    ID[i] = 0;
  }
  count = 0;
  
  if(x.Full_Auto)                   // Sound für Schnellfeuer
  {
    while(DataIdx<=WavHeader2.DataSize)
    {
      size_t BytesWritten;                            
      i2s_write(i2s_num,TheData2+DataIdx,4,&BytesWritten,portMAX_DELAY); 
      DataIdx+=4;
    }
    delay(120);
    DataIdx=0;
  }
  else                              // Sound für normalen Schuss
  {
    while(DataIdx<=WavHeader1.DataSize)
    {
      size_t BytesWritten;                            
      i2s_write(i2s_num,TheData1+DataIdx,4,&BytesWritten,portMAX_DELAY); 
      DataIdx+=4;       
    }
    DataIdx=0;
  }
  if(x.Aktiv_Magazin) Magazin--;    // Magazin um eins reduzieren
  x.Schuss++;                       // Schuss-Zähler um eins erhöhen
}

void Hit(int value)
{
  display.clear_screen();
  display.return_home();
  display.set_ramposition(PCF2119_DISPLAY_START_POSITION);
  LED(0);                                                     // Team-LED aus

  while(DataIdx<=WavHeader3.DataSize)                         // Treffer-Sound
  {
    size_t BytesWritten;                            
    i2s_write(i2s_num,TheData3+DataIdx,4,&BytesWritten,portMAX_DELAY); 
    DataIdx+=4;       
  }
  DataIdx=0;

  if(value == 0)                                  // Hit by Spieler1
  {
    display.printf("Killed by %s", x.name1.c_str());
    if((x.Modus != 1)||(Leben == 1)) x.Deaths[0]++;
  }
  else if(value == 1)                             // Hit by Spieler2
  {
    display.printf("Killed by %s", x.name2.c_str());
    if((x.Modus != 1)||(Leben == 1)) x.Deaths[1]++;
  }
  else if(value == 2)                             // Hit by Spieler3
  {
    display.printf("Killed by %s", x.name3.c_str());
    if((x.Modus != 1)||(Leben == 1)) x.Deaths[2]++;
  }
  else if(value == 3)                             // Hit by Spieler4
  {
    display.printf("Killed by %s", x.name4.c_str());
    if((x.Modus != 1)||(Leben == 1)) x.Deaths[3]++;
  }
  else if(value == 4)                            // Hit by Spieler5
  {
    display.printf("Killed by %s", x.name5.c_str());
    if((x.Modus != 1)||(Leben == 1)) x.Deaths[4]++;
  }
  else if(value == 5)                           // Hit by Spieler6
  {
    display.printf("Killed by %s", x.name6.c_str());
    if((x.Modus != 1)||(Leben == 1)) x.Deaths[5]++;
  }
  else if(value == 6)                           // Hit by Spieler7
  {
    display.printf("Killed by %s", x.name7.c_str());
    if((x.Modus != 1)||(Leben == 1)) x.Deaths[6]++;
  }
  else if(value == 7)                           // Hit by Spieler8
  {
    display.printf("Killed by %s", x.name8.c_str());
    if((x.Modus != 1)||(Leben == 1)) x.Deaths[7]++;
  }
  else if(value == 9)                           // Hit by Spieler10
  {
    display.printf("Killed by %s", x.name10.c_str());
    if((x.Modus != 1)||(Leben == 1)) x.Deaths[9]++;
  }
  
  if(x.Modus == 0)
  {
    for (int i = 0; i < x.Spawn_Zeit; i++)
    {
      LED(7);
      if(dauer >= x.Laenge) break;
      delay(500);
      LED(0);
      if(dauer >= x.Laenge) break;
      delay(500);
    }
  }
  else if(x.Modus == 1)
  {
    if(Leben != x.Leben_Reload) ITimer2.stopTimer();
    ITimer2.attachInterruptInterval(60000000, TimerHandler2);
    Leben--;
    if(Leben == 0)
    {
      ITimer2.stopTimer();
      for (int i = 0; i < x.Reload_Zeit; i++)
      {
        LED(7);
        if(dauer >= x.Laenge) break;
        delay(500);
        LED(0);
        if(dauer >= x.Laenge) break;
        delay(500);
      }
      Leben = x.Leben_Reload;
    }
  }
  else if(x.Modus == 2)
  {
    while((success2 == false)||(beacon % 10 != 0)||((beacon/10) != x.Teamfarbe[x.num]))
    {
      delay(100);
      if(dauer >= x.Laenge) break;
    }
    success2 = false;
    beacon = 0;
  }

  ITimer1.stopTimer();
  Magazin = x.Magazin_Reload;
  reload = -1;
  LED(x.Teamfarbe[x.num]);                                    // Team-LED an
  display.clear_screen();
  display.return_home();
  display.set_ramposition(PCF2119_DISPLAY_START_POSITION);
  display.printf("%-10s Lives: %d", x.name9.c_str(), Leben);
}
