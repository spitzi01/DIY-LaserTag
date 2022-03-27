#include <esp_now.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define Start_Pin 1
#define Config_Pin 2
#define Spiel_Pin 3
#define Ergebnis_Pin 4

AsyncWebServer server(80);                        // Initialisierung des Webserver auf Port 80   

struct Daten                                      // Datenpaket für jeden Spieler
{  
  uint8_t num;                                    // Dient der Identifikation

  uint8_t Magazin_Reload = 9;                     // Nachlade-Wert für das Magazin
  uint8_t Modus;                                  // Spielmodus 0: Standard, 1: Battle Royal, 2: Capture the Flag
  bool Aktiv_Magazin;				                      // Magazin-Feature Aktivierung
  bool Full_Auto;                                 // Schnellfeuer Aktivierung
  uint8_t Laenge;                                 // Dauer des Spiels in Minuten
  uint8_t Leben_Reload = 5;                       // Leben für Battle Royal Spielmodus
  short Reload_Zeit = 60;                         // Respawn-Zeit für Battle Royal Spielmodus
  short Spawn_Zeit = 12;                          // Respawn-Zeit allgemein

  uint8_t Teamfarbe[10];                              // 0 = Aus, 1 = Rot, 2 = Grün, 3 = Blau, 4 = Gelb, 5 = Cyan, 6 = Magent, 7 = Weiß
  short Schuss = 0;                                   // Anzahl der abgegebenen Schüsse
  uint8_t Deaths[10] = {0,0,0,0,0,0,0,0,0,0};         // Anzahl der erhaltenen Treffer pro Spieler
  String name1 = "";                                  // Namen aller Spieler, max. 10 Zeichen
  String name2 = "";
  String name3 = "";
  String name4 = "";
  String name5 = "";
  String name6 = "";
  String name7 = "";
  String name8 = "";
  String name9 = "";
  String name10 = "";
};

int Spieler_Anzahl;                                 // Anzahl der Spieler im Spiel
Daten x,y;                                          // Spieldaten
int d = 0;
short schuss[10];
uint8_t deaths[10][10];
bool pakete[10] = {false, false, false, false, false, false, false, false, false, false};          // Anzahl der bisher empfangenen Pakete
uint8_t mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};   // MAC-Adresse Broadcast

  // Parameter um Daten aus html1 zu bekommen

const char* input_parameter1 = "input1";
const char* input_parameter2 = "input2";
const char* input_parameter3 = "input3";
const char* input_parameter4 = "input4";
const char* input_parameter5 = "input5";
const char* input_parameter6 = "input6_1";
const char* input_parameter7 = "input6_2";
const char* input_parameter8 = "input7_1";
const char* input_parameter9 = "input7_2";
const char* input_parameter10 = "input8_1";
const char* input_parameter11 = "input8_2";
const char* input_parameter12 = "input9_1";
const char* input_parameter13 = "input9_2";
const char* input_parameter14 = "input10_1";
const char* input_parameter15 = "input10_2";
const char* input_parameter16 = "input11_1";
const char* input_parameter17 = "input11_2";
const char* input_parameter18 = "input12_1";
const char* input_parameter19 = "input12_2";
const char* input_parameter20 = "input13_1";
const char* input_parameter21 = "input13_2";
const char* input_parameter22 = "input14_1";
const char* input_parameter23 = "input14_2";
const char* input_parameter24 = "input15_1";
const char* input_parameter25 = "input15_2";

  // Konfiguration-Website \\

const char html1[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head>
<title>Konfiguration</title>
<meta http-equiv="content-type" content="text/html; charset=utf-8"></head>
<body>
<h1>Spiel-Starter</h1>
<h3>Einstellen der Spiel-Parameter</h3>
<form action="/get">
  <label for="num">Anzahl der Spieler:</label>
  <select id="num" name="input1">
    <option value="2">2</option>
    <option value="3">3</option>
    <option value="4">4</option>
    <option value="5">5</option>
    <option value="6">6</option>
    <option value="7">7</option>
    <option value="8">8</option>
    <option value="9">9</option>
    <option value="10">10</option>
  </select><br>
  <label for="mode">Spielmodus:</label>
  <select id="mode" name="input2">
    <option value="0">Standard</option>
    <option value="1">Battle Royal</option>
    <option value="2">Capture the Flag</option>
  </select><br>
  <label for="dauer">Dauer des Spiels in Minuten (1-60):</label>
  <input type="number" id="dauer" name="input3" min="1" max="60"><br>
  <label for="mag">Aktivierung des Magazins:</label>
  <select id="mag" name="input4">
    <option value="1">Ja</option>
    <option value="0">Nein</option>
  </select><br>
  <label for="full">Schnellfeuer-Modus:</label>
  <select id="full" name="input5">
    <option value="1">Ja</option>
    <option value="0">Nein</option>
  </select><hr>
<h3>Team-Konfiguration</h3>
<label for="name1">Name für Spieler1:</label>
  <input type="text" id="name1" name="input6_1" maxlength="10">
<label for="team1">Team-Farbe:</label>
  <select id="team1" name="input6_2">
    <option value="0">-</option>
    <option value="1">Rot</option>
    <option value="2">Grün</option>
    <option value="3">Blau</option>
    <option value="4">Gelb</option>
    <option value="5">Cyan</option>
    <option value="6">Magenta</option>
  </select><br>
<label for="name2">Name für Spieler2:</label>
  <input type="text" id="name2" name="input7_1" maxlength="10">
<label for="team2">Team-Farbe:</label>
  <select id="team2" name="input7_2">
    <option value="0">-</option>
    <option value="1">Rot</option>
    <option value="2">Grün</option>
    <option value="3">Blau</option>
    <option value="4">Gelb</option>
    <option value="5">Cyan</option>
    <option value="6">Magenta</option>
  </select><br>
<label for="name3">Name für Spieler3:</label>
  <input type="text" id="name3" name="input8_1" maxlength="10">
<label for="team3">Team-Farbe:</label>
  <select id="team3" name="input8_2">
    <option value="0">-</option>
    <option value="1">Rot</option>
    <option value="2">Grün</option>
    <option value="3">Blau</option>
    <option value="4">Gelb</option>
    <option value="5">Cyan</option>
    <option value="6">Magenta</option>
  </select><br>
<label for="name4">Name für Spieler4:</label>
  <input type="text" id="name4" name="input9_1" maxlength="10">
<label for="team4">Team-Farbe:</label>
  <select id="team4" name="input9_2">
    <option value="0">-</option>
    <option value="1">Rot</option>
    <option value="2">Grün</option>
    <option value="3">Blau</option>
    <option value="4">Gelb</option>
    <option value="5">Cyan</option>
    <option value="6">Magenta</option>
  </select><br>
<label for="name5">Name für Spieler5:</label>
  <input type="text" id="name5" name="input10_1" maxlength="10">
<label for="team5">Team-Farbe:</label>
  <select id="team5" name="input10_2">
    <option value="0">-</option>
    <option value="1">Rot</option>
    <option value="2">Grün</option>
    <option value="3">Blau</option>
    <option value="4">Gelb</option>
    <option value="5">Cyan</option>
    <option value="6">Magenta</option>
  </select><br>
<label for="name6">Name für Spieler6:</label>
  <input type="text" id="name6" name="input11_1" maxlength="10">
<label for="team6">Team-Farbe:</label>
  <select id="team6" name="input11_2">
    <option value="0">-</option>
    <option value="1">Rot</option>
    <option value="2">Grün</option>
    <option value="3">Blau</option>
    <option value="4">Gelb</option>
    <option value="5">Cyan</option>
    <option value="6">Magenta</option>
  </select><br>
<label for="name7">Name für Spieler7:</label>
  <input type="text" id="name7" name="input12_1" maxlength="10">
<label for="team7">Team-Farbe:</label>
  <select id="team7" name="input12_2">
    <option value="0">-</option>
    <option value="1">Rot</option>
    <option value="2">Grün</option>
    <option value="3">Blau</option>
    <option value="4">Gelb</option>
    <option value="5">Cyan</option>
    <option value="6">Magenta</option>
  </select><br>
<label for="name8">Name für Spieler8:</label>
  <input type="text" id="name8" name="input13_1" maxlength="10">
<label for="team8">Team-Farbe:</label>
  <select id="team8" name="input13_2">
    <option value="0">-</option>
    <option value="1">Rot</option>
    <option value="2">Grün</option>
    <option value="3">Blau</option>
    <option value="4">Gelb</option>
    <option value="5">Cyan</option>
    <option value="6">Magenta</option>
  </select><br>
<label for="name9">Name für Spieler9:</label>
  <input type="text" id="name9" name="input14_1" maxlength="10">
<label for="team9">Team-Farbe:</label>
  <select id="team9" name="input14_2">
    <option value="0">-</option>
    <option value="1">Rot</option>
    <option value="2">Grün</option>
    <option value="3">Blau</option>
    <option value="4">Gelb</option>
    <option value="5">Cyan</option>
    <option value="6">Magenta</option>
  </select><br>
<label for="name10">Name für Spieler10:</label>
  <input type="text" id="name10" name="input15_1" maxlength="10">
<label for="team10">Team-Farbe:</label>
  <select id="team10" name="input15_2">
    <option value="0">-</option>
    <option value="1">Rot</option>
    <option value="2">Grün</option>
    <option value="3">Blau</option>
    <option value="4">Gelb</option>
    <option value="5">Cyan</option>
    <option value="6">Magenta</option>
  </select><br><br>
<p>Alle Systeme sollten kurz vor dem Start des Spieles eingeschalten werden!
Wenn der Spielmodus Capture the Flag ausgewählt wurde, müssen Base- und Spawn-Beacons eingeschaltet werden.
Auch dürfen nur zwei Teams (Rot und Blau) verwendet werden.</p>
<br>
<input type="submit" value="Start des Spieles"></form>
</body></html>)rawliteral";

  // Ergebnis-Website \\

const char html2[] PROGMEM = R"rawliteral(<!DOCTYPE html><html><head><title>Ergebnis</title>
<meta http-equiv="content-type" content="text/html; charset=utf-8"></head><body>

<h1>Ergebnisse</h1>

<h3>Spieler-Auswertung</h3>

<style type="text/css">
.tg  {border-collapse:collapse;border-spacing:0;}
.tg td{font-family:Arial, sans-serif;font-size:14px;padding:10px 5px;border-style:solid;border-width:1px;overflow:hidden;word-break:normal;}
.tg th{font-family:Arial, sans-serif;font-size:14px;font-weight:normal;padding:10px 5px;border-style:solid;border-width:1px;overflow:hidden;word-break:normal;}
</style>
<table class="tg">
  <tr>
    <th class="tg-031e" rowspan="2">Team</th>
    <th class="tg-031e" rowspan="2">Name</th>
    <th class="tg-031e" rowspan="2">Treffer</th>
    <th class="tg-031e" rowspan="2">Schüsse</th>
    <th class="tg-031e" rowspan="2">Trefferquote</th>
    <th class="tg-031e" colspan="10">Tode</th>
    <th class="tg-031e" rowspan="2">Punktzahl</th>
  </tr>
  <tr>
    <td class="tg-031e">1</td>
    <td class="tg-031e">2</td>
    <td class="tg-031e">3</td>
    <td class="tg-031e">4</td>
    <td class="tg-031e">5</td>
    <td class="tg-031e">6</td>
    <td class="tg-031e">7</td>
    <td class="tg-031e">8</td>
    <td class="tg-031e">9</td>
    <td class="tg-031e">10</td>
  </tr>
  <tr>
    <td class="tg-031e">%1%</td>
    <td class="tg-031e">%2%</td>
    <td class="tg-031e">%3%</td>
    <td class="tg-031e">%4%</td>
    <td class="tg-031e">%5%</td>
    <td class="tg-031e">%6%</td>
    <td class="tg-031e">%7%</td>
    <td class="tg-031e">%8%</td>
    <td class="tg-031e">%9%</td>
    <td class="tg-031e">%10%</td>
    <td class="tg-031e">%11%</td>
    <td class="tg-031e">%12%</td>
    <td class="tg-031e">%13%</td>
    <td class="tg-031e">%14%</td>
    <td class="tg-031e">%15%</td>
    <td class="tg-031e">%16%</td>
  </tr>
  <tr>
    <td class="tg-031e">%17%</td>
    <td class="tg-031e">%18%</td>
    <td class="tg-031e">%19%</td>
    <td class="tg-031e">%20%</td>
    <td class="tg-031e">%21%</td>
    <td class="tg-031e">%22%</td>
    <td class="tg-031e">%23%</td>
    <td class="tg-031e">%24%</td>
    <td class="tg-031e">%25%</td>
    <td class="tg-031e">%26%</td>
    <td class="tg-031e">%27%</td>
    <td class="tg-031e">%28%</td>
    <td class="tg-031e">%29%</td>
    <td class="tg-031e">%30%</td>
    <td class="tg-031e">%31%</td>
    <td class="tg-031e">%32%</td>
  </tr>
  <tr>
    <td class="tg-031e">%33%</td>
    <td class="tg-031e">%34%</td>
    <td class="tg-031e">%35%</td>
    <td class="tg-031e">%36%</td>
    <td class="tg-031e">%37%</td>
    <td class="tg-031e">%38%</td>
    <td class="tg-031e">%39%</td>
    <td class="tg-031e">%40%</td>
    <td class="tg-031e">%41%</td>
    <td class="tg-031e">%42%</td>
    <td class="tg-031e">%43%</td>
    <td class="tg-031e">%44%</td>
    <td class="tg-031e">%45%</td>
    <td class="tg-031e">%46%</td>
    <td class="tg-031e">%47%</td>
    <td class="tg-031e">%48%</td>
  </tr>
  <tr>
    <td class="tg-031e">%49%</td>
    <td class="tg-031e">%50%</td>
    <td class="tg-031e">%51%</td>
    <td class="tg-031e">%52%</td>
    <td class="tg-031e">%53%</td>
    <td class="tg-031e">%54%</td>
    <td class="tg-031e">%55%</td>
    <td class="tg-031e">%56%</td>
    <td class="tg-031e">%57%</td>
    <td class="tg-031e">%58%</td>
    <td class="tg-031e">%59%</td>
    <td class="tg-031e">%60%</td>
    <td class="tg-031e">%61%</td>
    <td class="tg-031e">%62%</td>
    <td class="tg-031e">%63%</td>
    <td class="tg-031e">%64%</td>
  </tr>
  <tr>
    <td class="tg-031e">%65%</td>
    <td class="tg-031e">%66%</td>
    <td class="tg-031e">%67%</td>
    <td class="tg-031e">%68%</td>
    <td class="tg-031e">%69%</td>
    <td class="tg-031e">%70%</td>
    <td class="tg-031e">%71%</td>
    <td class="tg-031e">%72%</td>
    <td class="tg-031e">%73%</td>
    <td class="tg-031e">%74%</td>
    <td class="tg-031e">%75%</td>
    <td class="tg-031e">%76%</td>
    <td class="tg-031e">%77%</td>
    <td class="tg-031e">%78%</td>
    <td class="tg-031e">%79%</td>
    <td class="tg-031e">%80%</td>
  </tr>
  <tr>
    <td class="tg-031e">%81%</td>
    <td class="tg-031e">%82%</td>
    <td class="tg-031e">%83%</td>
    <td class="tg-031e">%84%</td>
    <td class="tg-031e">%85%</td>
    <td class="tg-031e">%86%</td>
    <td class="tg-031e">%87%</td>
    <td class="tg-031e">%88%</td>
    <td class="tg-031e">%89%</td>
    <td class="tg-031e">%90%</td>
    <td class="tg-031e">%91%</td>
    <td class="tg-031e">%92%</td>
    <td class="tg-031e">%93%</td>
    <td class="tg-031e">%94%</td>
    <td class="tg-031e">%95%</td>
    <td class="tg-031e">%96%</td>
  </tr>
  <tr>
    <td class="tg-031e">%97%</td>
    <td class="tg-031e">%98%</td>
    <td class="tg-031e">%99%</td>
    <td class="tg-031e">%100%</td>
    <td class="tg-031e">%101%</td>
    <td class="tg-031e">%102%</td>
    <td class="tg-031e">%103%</td>
    <td class="tg-031e">%104%</td>
    <td class="tg-031e">%105%</td>
    <td class="tg-031e">%106%</td>
    <td class="tg-031e">%107%</td>
    <td class="tg-031e">%108%</td>
    <td class="tg-031e">%109%</td>
    <td class="tg-031e">%110%</td>
    <td class="tg-031e">%111%</td>
    <td class="tg-031e">%112%</td>
  </tr>
  <tr>
    <td class="tg-031e">%113%</td>
    <td class="tg-031e">%114%</td>
    <td class="tg-031e">%115%</td>
    <td class="tg-031e">%116%</td>
    <td class="tg-031e">%117%</td>
    <td class="tg-031e">%118%</td>
    <td class="tg-031e">%119%</td>
    <td class="tg-031e">%120%</td>
    <td class="tg-031e">%121%</td>
    <td class="tg-031e">%122%</td>
    <td class="tg-031e">%123%</td>
    <td class="tg-031e">%124%</td>
    <td class="tg-031e">%125%</td>
    <td class="tg-031e">%126%</td>
    <td class="tg-031e">%127%</td>
    <td class="tg-031e">%128%</td>
  </tr>
  <tr>
    <td class="tg-031e">%129%</td>
    <td class="tg-031e">%130%</td>
    <td class="tg-031e">%131%</td>
    <td class="tg-031e">%132%</td>
    <td class="tg-031e">%133%</td>
    <td class="tg-031e">%134%</td>
    <td class="tg-031e">%135%</td>
    <td class="tg-031e">%136%</td>
    <td class="tg-031e">%137%</td>
    <td class="tg-031e">%138%</td>
    <td class="tg-031e">%139%</td>
    <td class="tg-031e">%140%</td>
    <td class="tg-031e">%141%</td>
    <td class="tg-031e">%142%</td>
    <td class="tg-031e">%143%</td>
    <td class="tg-031e">%144%</td>
  </tr>
  <tr>
    <td class="tg-031e">%145%</td>
    <td class="tg-031e">%146%</td>
    <td class="tg-031e">%147%</td>
    <td class="tg-031e">%148%</td>
    <td class="tg-031e">%149%</td>
    <td class="tg-031e">%150%</td>
    <td class="tg-031e">%151%</td>
    <td class="tg-031e">%152%</td>
    <td class="tg-031e">%153%</td>
    <td class="tg-031e">%154%</td>
    <td class="tg-031e">%155%</td>
    <td class="tg-031e">%156%</td>
    <td class="tg-031e">%157%</td>
    <td class="tg-031e">%158%</td>
    <td class="tg-031e">%159%</td>
    <td class="tg-031e">%160%</td>
  </tr>
</table>

<hr>

<h3>Team-Auswertung</h3>

<style type="text/css">
.tg  {border-collapse:collapse;border-spacing:0;}
.tg td{font-family:Arial, sans-serif;font-size:14px;padding:10px 5px;border-style:solid;border-width:1px;overflow:hidden;word-break:normal;}
.tg th{font-family:Arial, sans-serif;font-size:14px;font-weight:normal;padding:10px 5px;border-style:solid;border-width:1px;overflow:hidden;word-break:normal;}
</style>
<table class="tg">
  <tr>
    <th class="tg-031e" rowspan="2">Team</th>
    <th class="tg-031e" rowspan="2">Punktzahl</th>
  </tr>
  <tr>
  </tr>
  <tr>
    <td class="tg-031e">Rot</td>
    <td class="tg-031e">%161%</td>
  </tr>
  <tr>
    <td class="tg-031e">Grün</td>
    <td class="tg-031e">%162%</td>
  </tr>
  <tr>
    <td class="tg-031e">Blau</td>
    <td class="tg-031e">%163%</td>
  </tr>
  <tr>
    <td class="tg-031e">Gelb</td>
    <td class="tg-031e">%164%</td>
  </tr>
  <tr>
    <td class="tg-031e">Cyan</td>
    <td class="tg-031e">%165%</td>
  </tr>
  <tr>
    <td class="tg-031e">Magenta</td>
    <td class="tg-031e">%166%</td>
  </tr>
</table></body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request)   // Text wenn Website nicht gefunden wurde
{
  request->send(404, "text/plain", "Not found");
}


void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len)        // Verarbeitung empfangener Daten
{
  if(len > 8) 
  {
    memcpy(&y, incomingData, sizeof(y));
    pakete[y.num] = true;
    schuss[y.num] = y.Schuss;
    for(int i = 0; i < 10; i++)
    {
      deaths[y.num][i] = y.Deaths[i];
    }
  }
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)            // Verarbeitung Sendung
{

}

float quote[10];
int enemyKill[10];
int friendlyKill[10];
int kills[10];
int tode[10];
int score[10];
int flag[10];
String team[] = {"","Rot","Grün","Blau","Gelb","Cyan","Magenta"};

// Team Rot
int score_Rot = 0;

// Team Grün
int score_Gruen = 0;

// Team Blau
int score_Blau = 0;

// Team Cyan
int score_Cyan = 0;

// Team Magenta
int score_Magenta = 0;

// Team Gelb
int score_Gelb = 0;

void Auswerten()          // Berechnung aller Daten für das Spielergebnis
{
  for (int i = 0; i < Spieler_Anzahl; i++)
  {
    for (int j = 0; j < Spieler_Anzahl; j++)
    {
      if(i!=j)
      {
        if(x.Teamfarbe[i] == x.Teamfarbe[j]) friendlyKill[i] += deaths[j][i];
        else enemyKill[i] += deaths[j][i];

        kills[i] += deaths[j][i];
        tode[i] += deaths[i][j];
      }
      else if((x.Modus == 2)&&(i==j)) flag[i] = deaths[j][i];          
    }
    if((kills[i] == 0)||(schuss[i] == 0)) quote[i] = 0;
    else quote[i] = float(kills[i]) / float(schuss[i]) * 100;
    score[i] = 100 * enemyKill[i] - 200 * friendlyKill[i] - 50 * tode[i] + 200 * flag[i];
    switch(x.Teamfarbe[i])
    {
      case 1: score_Rot += score[i]; break;
      case 2: score_Gruen += score[i]; break;
      case 3: score_Blau += score[i]; break;
      case 4: score_Gelb += score[i]; break;
      case 5: score_Cyan += score[i]; break;
      case 6: score_Magenta += score[i]; break;
    }
  }
}

String processor(const String& var)     // String-Prozessor für Ergebnis-Darstellung in html2
{
  if(var == "1") return String(team[x.Teamfarbe[0]]);
  else if(var == "2") return String(x.name1);
  else if(var == "3") return String(kills[0]);
  else if(var == "4") return String(schuss[0]);
  else if(var == "5") return String(quote[0]);
  else if(var == "6") return String(deaths[0][0]);
  else if(var == "7") return String(deaths[0][1]);
  else if(var == "8") return String(deaths[0][2]);
  else if(var == "9") return String(deaths[0][3]);
  else if(var == "10") return String(deaths[0][4]);
  else if(var == "11") return String(deaths[0][5]);
  else if(var == "12") return String(deaths[0][6]);
  else if(var == "13") return String(deaths[0][7]);
  else if(var == "14") return String(deaths[0][8]);
  else if(var == "15") return String(deaths[0][9]);
  else if(var == "16") return String(score[0]);

  else if(var == "17") return String(team[x.Teamfarbe[1]]);
  else if(var == "18") return String(x.name2);
  else if(var == "19") return String(kills[1]);
  else if(var == "20") return String(schuss[1]);
  else if(var == "21") return String(quote[1]);
  else if(var == "22") return String(deaths[1][0]);
  else if(var == "23") return String(deaths[1][1]);
  else if(var == "24") return String(deaths[1][2]);
  else if(var == "25") return String(deaths[1][3]);
  else if(var == "26") return String(deaths[1][4]);
  else if(var == "27") return String(deaths[1][5]);
  else if(var == "28") return String(deaths[1][6]);
  else if(var == "29") return String(deaths[1][7]);
  else if(var == "30") return String(deaths[1][8]);
  else if(var == "31") return String(deaths[1][9]);
  else if(var == "32") return String(score[1]);

  else if(var == "33") return String(team[x.Teamfarbe[2]]);
  else if(var == "34") return String(x.name3);
  else if(var == "35") return String(kills[2]);
  else if(var == "36") return String(schuss[2]);
  else if(var == "37") return String(quote[2]);
  else if(var == "38") return String(deaths[2][0]);
  else if(var == "39") return String(deaths[2][1]);
  else if(var == "40") return String(deaths[2][2]);
  else if(var == "41") return String(deaths[2][3]);
  else if(var == "42") return String(deaths[2][4]);
  else if(var == "43") return String(deaths[2][5]);
  else if(var == "44") return String(deaths[2][6]);
  else if(var == "45") return String(deaths[2][7]);
  else if(var == "46") return String(deaths[2][8]);
  else if(var == "47") return String(deaths[2][9]);
  else if(var == "48") return String(score[2]);

  else if(var == "49") return String(team[x.Teamfarbe[3]]);
  else if(var == "50") return String(x.name4);
  else if(var == "51") return String(kills[3]);
  else if(var == "52") return String(schuss[3]);
  else if(var == "53") return String(quote[3]);
  else if(var == "54") return String(deaths[3][0]);
  else if(var == "55") return String(deaths[3][1]);
  else if(var == "56") return String(deaths[3][2]);
  else if(var == "57") return String(deaths[3][3]);
  else if(var == "58") return String(deaths[3][4]);
  else if(var == "59") return String(deaths[3][5]);
  else if(var == "60") return String(deaths[3][6]);
  else if(var == "61") return String(deaths[3][7]);
  else if(var == "62") return String(deaths[3][8]);
  else if(var == "63") return String(deaths[3][9]);
  else if(var == "64") return String(score[3]);

  else if(var == "65") return String(team[x.Teamfarbe[4]]);
  else if(var == "66") return String(x.name5);
  else if(var == "67") return String(kills[4]);
  else if(var == "68") return String(schuss[4]);
  else if(var == "69") return String(quote[4]);
  else if(var == "70") return String(deaths[4][0]);
  else if(var == "71") return String(deaths[4][1]);
  else if(var == "72") return String(deaths[4][2]);
  else if(var == "73") return String(deaths[4][3]);
  else if(var == "74") return String(deaths[4][4]);
  else if(var == "75") return String(deaths[4][5]);
  else if(var == "76") return String(deaths[4][6]);
  else if(var == "77") return String(deaths[4][7]);
  else if(var == "78") return String(deaths[4][8]);
  else if(var == "79") return String(deaths[4][9]);
  else if(var == "80") return String(score[4]);

  else if(var == "81") return String(team[x.Teamfarbe[5]]);
  else if(var == "82") return String(x.name6);
  else if(var == "83") return String(kills[5]);
  else if(var == "84") return String(schuss[5]);
  else if(var == "85") return String(quote[5]);
  else if(var == "86") return String(deaths[5][0]);
  else if(var == "87") return String(deaths[5][1]);
  else if(var == "88") return String(deaths[5][2]);
  else if(var == "89") return String(deaths[5][3]);
  else if(var == "90") return String(deaths[5][4]);
  else if(var == "91") return String(deaths[5][5]);
  else if(var == "92") return String(deaths[5][6]);
  else if(var == "93") return String(deaths[5][7]);
  else if(var == "94") return String(deaths[5][8]);
  else if(var == "95") return String(deaths[5][9]);
  else if(var == "96") return String(score[5]);

  else if(var == "97") return String(team[x.Teamfarbe[6]]);
  else if(var == "98") return String(x.name7);
  else if(var == "99") return String(kills[6]);
  else if(var == "100") return String(schuss[6]);
  else if(var == "101") return String(quote[6]);
  else if(var == "102") return String(deaths[6][0]);
  else if(var == "103") return String(deaths[6][1]);
  else if(var == "104") return String(deaths[6][2]);
  else if(var == "105") return String(deaths[6][3]);
  else if(var == "106") return String(deaths[6][4]);
  else if(var == "107") return String(deaths[6][5]);
  else if(var == "108") return String(deaths[6][6]);
  else if(var == "109") return String(deaths[6][7]);
  else if(var == "110") return String(deaths[6][8]);
  else if(var == "111") return String(deaths[6][9]);
  else if(var == "112") return String(score[6]);

  else if(var == "113") return String(team[x.Teamfarbe[7]]);
  else if(var == "114") return String(x.name8);
  else if(var == "115") return String(kills[7]);
  else if(var == "116") return String(schuss[7]);
  else if(var == "117") return String(quote[7]);
  else if(var == "118") return String(deaths[7][0]);
  else if(var == "119") return String(deaths[7][1]);
  else if(var == "120") return String(deaths[7][2]);
  else if(var == "121") return String(deaths[7][3]);
  else if(var == "122") return String(deaths[7][4]);
  else if(var == "123") return String(deaths[7][5]);
  else if(var == "124") return String(deaths[7][6]);
  else if(var == "125") return String(deaths[7][7]);
  else if(var == "126") return String(deaths[7][8]);
  else if(var == "127") return String(deaths[7][9]);
  else if(var == "128") return String(score[7]);

  else if(var == "129") return String(team[x.Teamfarbe[8]]);
  else if(var == "130") return String(x.name9);
  else if(var == "131") return String(kills[8]);
  else if(var == "132") return String(schuss[8]);
  else if(var == "133") return String(quote[8]);
  else if(var == "134") return String(deaths[8][0]);
  else if(var == "135") return String(deaths[8][1]);
  else if(var == "136") return String(deaths[8][2]);
  else if(var == "137") return String(deaths[8][3]);
  else if(var == "138") return String(deaths[8][4]);
  else if(var == "139") return String(deaths[8][5]);
  else if(var == "140") return String(deaths[8][6]);
  else if(var == "141") return String(deaths[8][7]);
  else if(var == "142") return String(deaths[8][8]);
  else if(var == "143") return String(deaths[8][9]);
  else if(var == "144") return String(score[8]);

  else if(var == "145") return String(team[x.Teamfarbe[9]]);
  else if(var == "146") return String(x.name10);
  else if(var == "147") return String(kills[9]);
  else if(var == "148") return String(schuss[9]);
  else if(var == "149") return String(quote[9]);
  else if(var == "150") return String(deaths[9][0]);
  else if(var == "151") return String(deaths[9][1]);
  else if(var == "152") return String(deaths[9][2]);
  else if(var == "153") return String(deaths[9][3]);
  else if(var == "154") return String(deaths[9][4]);
  else if(var == "155") return String(deaths[9][5]);
  else if(var == "156") return String(deaths[9][6]);
  else if(var == "157") return String(deaths[9][7]);
  else if(var == "158") return String(deaths[9][8]);
  else if(var == "159") return String(deaths[9][9]);
  else if(var == "160") return String(score[9]);
  else if(var == "161") return String(score_Rot);
  else if(var == "162") return String(score_Gruen);
  else if(var == "163") return String(score_Blau);
  else if(var == "164") return String(score_Gelb);
  else if(var == "165") return String(score_Cyan);
  else if(var == "166") return String(score_Magenta);
  return String();
}

void setup()
{
   // Definition der GPIOs \\
  pinMode(Start_Pin, INPUT);
  pinMode(Config_Pin, OUTPUT);
  pinMode(Spiel_Pin, OUTPUT);
  pinMode(Ergebnis_Pin, OUTPUT);

  WiFi.softAP("LaserTag", "123456789");                                         // Wlan-Access-Point aktivieren

  server.on("/Konfiguration", HTTP_GET, [](AsyncWebServerRequest *request)      // html1 hosten
  {
    request->send_P(200, "text/html", html1);
  });

  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request)               // Auswerten des Feedbacks von html1
  {
    if (request->hasParam(input_parameter1))
    {
      Spieler_Anzahl = request->getParam(input_parameter1)->value().toInt();
    }
    if (request->hasParam(input_parameter2))
    {
      x.Modus = request->getParam(input_parameter2)->value().toInt();
    }
    if (request->hasParam(input_parameter3))
    {
      x.Laenge = request->getParam(input_parameter3)->value().toInt();
    }
    if (request->hasParam(input_parameter4))
    {
      x.Aktiv_Magazin = request->getParam(input_parameter4)->value().toInt();
    }
    if (request->hasParam(input_parameter5))
    {
      x.Full_Auto = request->getParam(input_parameter5)->value().toInt();
    }
    if (request->hasParam(input_parameter6))
    {
      x.name1 = request->getParam(input_parameter6)->value();
    }
    if (request->hasParam(input_parameter7))
    {
      x.Teamfarbe[0] = request->getParam(input_parameter7)->value().toInt();
    }
    if (request->hasParam(input_parameter8))
    {
      x.name2 = request->getParam(input_parameter8)->value();
    }
    if (request->hasParam(input_parameter9))
    {
      x.Teamfarbe[1] = request->getParam(input_parameter9)->value().toInt();
    }
    if (request->hasParam(input_parameter10))
    {
      x.name3 = request->getParam(input_parameter10)->value();
    }
    if (request->hasParam(input_parameter11))
    {
      x.Teamfarbe[2] = request->getParam(input_parameter11)->value().toInt();
    }
    if (request->hasParam(input_parameter12))
    {
      x.name4 = request->getParam(input_parameter12)->value();
    }
    if (request->hasParam(input_parameter13))
    {
      x.Teamfarbe[3] = request->getParam(input_parameter13)->value().toInt();
    }
    if (request->hasParam(input_parameter14))
    {
      x.name5 = request->getParam(input_parameter14)->value();
    }
    if (request->hasParam(input_parameter15))
    {
      x.Teamfarbe[4] = request->getParam(input_parameter15)->value().toInt();
    }
    if (request->hasParam(input_parameter16))
    {
      x.name6 = request->getParam(input_parameter16)->value();
    }
    if (request->hasParam(input_parameter17))
    {
      x.Teamfarbe[5] = request->getParam(input_parameter17)->value().toInt();
    }
    if (request->hasParam(input_parameter18))
    {
      x.name7 = request->getParam(input_parameter18)->value();
    }
    if (request->hasParam(input_parameter19))
    {
      x.Teamfarbe[6] = request->getParam(input_parameter19)->value().toInt();
    }
    if (request->hasParam(input_parameter20))
    {
      x.name8 = request->getParam(input_parameter20)->value();
    }
    if (request->hasParam(input_parameter21))
    {
      x.Teamfarbe[7] = request->getParam(input_parameter21)->value().toInt();
    }
    if (request->hasParam(input_parameter22))
    {
      x.name9 = request->getParam(input_parameter22)->value();
    }
    if (request->hasParam(input_parameter23))
    {
      x.Teamfarbe[8] = request->getParam(input_parameter23)->value().toInt();
    }
    if (request->hasParam(input_parameter24))
    {
      x.name10 = request->getParam(input_parameter24)->value();
    }
    if (request->hasParam(input_parameter25))
    {
      x.Teamfarbe[9] = request->getParam(input_parameter25)->value().toInt();
      d++;
    }
  });
  server.onNotFound(notFound);                                        // notFound hosten
  server.begin();                                                     // Webserver starten
  digitalWrite(Config_Pin, HIGH);                                     // Aktivieren der Config-LED                                    
  while(d == 0) delay(1000);                                          // Warten bis alle Daten da sind
  server.end();                                                       // Beenden des Webservers
  digitalWrite(Config_Pin, LOW);                                      // Deaktivieren der Config-LED                                    
   
  
  WiFi.mode(WIFI_STA);                                                // Wifi im Station-Mode
  
  esp_now_init();                                                     // ESP-NOW initialisieren

  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo;                                       // Verbindung mit allen Clients aufbauen
  memcpy(peerInfo.peer_addr, mac, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  esp_now_register_recv_cb(OnDataRecv);                               // Empfang aktivieren
  
  while(digitalRead(Start_Pin) == LOW) delay(100);

  esp_now_send(mac, (uint8_t *) &x, sizeof(x));                       // Senden an alle Clients

  digitalWrite(Spiel_Pin, HIGH);                                      // Aktivieren der Spiel-LED
  d = 0;
  while(d < Spieler_Anzahl)
  {
    d = 0;
    for(int i = 0; i < Spieler_Anzahl; i++)
    {
      if(pakete[i] == true) d++;
    }
    delay(100);                                                       // Warten bis alle Pakete wieder angekommen sind
  }
  esp_now_deinit();
  digitalWrite(Spiel_Pin, LOW);                                       // Deaktivieren der Spiel-LED


  Auswerten();                                                        // Starten der Auswertung
  WiFi.mode(WIFI_AP);                                                 // Wlan-Access-Point starten
  WiFi.softAP("LaserTag", "123456789");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)         // html2 zu Webserver hinzufügen mit String-Prozessor
  {
    request->send_P(200, "text/html", html2, processor);
  });
  server.begin();                                                     // Webserver starten
  digitalWrite(Ergebnis_Pin, HIGH);                                   // Aktivieren der Ergebnis-LED
}
 
void loop()
{

}
