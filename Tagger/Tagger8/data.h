struct Daten
{
	uint8_t num;			              // Dient der Identifikation

  uint8_t Magazin_Reload; 	      // Nachlade-Wert für das Magazin
	uint8_t Modus;                  // Spielmodus 0: Standard, 1: Battle Royal, 2: Capture the Flag
	bool Aktiv_Magazin; 		        // Magazin-Feature Aktivierung
  bool Aktiv_Friendlyfire;        // Ermöglicht es Teammitglieder abzuschießen
	bool Full_Auto; 		            // Schnellfeuer Aktivierung
	uint8_t Laenge; 		            // Dauer des Spiels in Minuten
	uint8_t Leben_Reload;           // Leben für Battle Royal Spielmodus
  short Reload_Zeit;              // Respawn-Zeit für Battle Royal Spielmodus
  short Spawn_Zeit;               // Respawn-Zeit allgemein

	uint8_t Teamfarbe[10]; 		      // 0 = Aus, 1 = Rot, 2 = Grün, 3 = Blau, 4 = Gelb, 5 = Cyan, 6 = Magent, 7 = Weiß 
	short Schuss; 			            // Anzahl der abgegebenen Schüsse
	uint8_t Deaths[10]; 		        // Anzahl der erhaltenen Treffer pro Spieler
	String name1; 			            // Namen aller Spieler, max. 10 Zeichen
	String name2;
	String name3;
	String name4;
	String name5;
	String name6;
	String name7;
	String name8;
	String name9;
	String name10;
};
	
