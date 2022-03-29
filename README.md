# ESP32-LaserTag
Easy Open Source LaserTag System
### Überblick
Ziel dieses Projektes war der Bau eines einfachen LaserTag-Systems. Es sollte schnell und mit wenig Hardware an beliebigen Orten aufbaubar sein und  und über ein Handy gesteuert werden können.

Weitere Features sind:

- Soundausgabe für Schuss, Treffer und Nachladen (insgesamt 5 Verschiedene)
- Display in jedem Tagger
- lange Akkulaufzeit des gesamten Systems
- 3 Spielmodi
- Taschenlampe
- rote Laserdiode für Schuss-Simulation
- 5 in den Tagger integriete Infrarot-Empfänger
    
Das LaserTag-System besteht aus zwei elementaren Bestandteilen. Die Basisstation dient als Verbindungsglied zwischen den Taggern, der einzelnen Spieler, und einer HTTP-Website, welche der Konfiguration und Auswertung dient. Der Datenaustausch mit den Spielern erfolgt drahtlos mit Hilfe des EspNow-Protokolls der ESP32 S2 Mikrocontroller. Zum Spielen sind zwischen 2 und 10 Tagger notwendig. Für den Capture the Flag Spielmodus werden zusätzlich noch Flags benötigt, die die Basis und Spawnpunkt eines Team markieren.

### Basisstation
![Basisstation](/Basisstation/Basisstation_Beispiel.JPG)
Nach dem Einschalten der Basisstation wird ein WLAN-Netzwerk erzeugt und die Konfigurationswebsite gehostet. Wenn man sich hier mit einem Smartphone einloggt, kann man die Anzahl der Spieler, Dauer des Spiels, Modus, Namen der Spieler und Teamzugehörigkeit einstellen. Die maximal 10 Spieler können auf 6 Teams verteilt werden. Vor dem Starten des Spiels mit dem Start-Taster müssen unbedingt alle Tagger, die verwendet werden sollen, eingeschaltet werden, damit diese von der Basisstation ihre Informationen erhalten können. Nach Ablauf der Spieldauer übermitteln die Tagger automatisch die Spielstände an die Basisstation und schalten sich ab. Die Basisstation berechnet die Ergebnisse und hostet diese in Form einer Website. Das hat den Vorteil, dass sich theoretisch mehrere Spieler die Ergebnisse auf ihren Smartphones anschauen können.

### Tagger
![Tagger1](/Tagger/Tagger_Beispiel1.JPG)
Bei einem Testsystem wurde festgestellt, dass es dem Spieler nicht möglich sein darf, seinen Tagger aus Versehen auszuschalten. Deshalb wird die Schaltung durch einen Taster aktiviert und nur durch den Mikrocontroller wieder deaktiviert. Außerdem soll der Spieler immer beide Hände am Tagger haben. So soll verhindert werden, dass man mit einer Hand Sensoren abdecken könnte. Dies wird durch einen Taster in Reihe zum eigentlichen Abzug realisiert. Gleichzeitig schaltet dieser auch die Taschenlampe an. In meinem Fall dient ein kleiner 12V Bleiakku aus einer Brandmeldezentrale als Energieversorgung. Mit Hilfe des Step Down Converters wird eine hohe Effizienz und Akkulaufzeit erreicht. Das Display hat 20 alphanumerische Stellen, welche die Namen der Spieler und die Anzahl der Leben anzeigen. Eine Hintergrundbeleuchtung ist nicht vorhanden, da diese zu viel Strom benötigen würde. Deshalb dient die RGB-LED für die Teamzugehörigkeit als Beleuchtung.

![Tagger2](/Tagger/Tagger_Beispiel2.JPG)

Der Schuss wird durch eine Laserdiode visualisiert. Die eigentliche Information findet durch eine IR-Übertragung ihren Weg zu anderen Spielern. Bei Tests in der Nacht konnte eine Reichweite von bis zu 60 Meter erzielt werden. Der kurze Alu-Lauf dient als Reflektor für die Infrarot-LED und auch als Kühlkörper, da bei aktiviertem Schnellfeuermodus die LED sehr belastet wird.

Die verfügbaren Spielmodi sind:

- Standard (Spielen in Teams oder Einzeln, Reaktivierung nach ca. 20 Sekunden)
- Battle Royal (Spielen in Teams oder Einzeln, Leben werden bei Treffer weniger, kein Treffer in gewisser Zeit fügt ein Leben hinzu, lange Reaktivierungszeit ca. 2 Minuten)
- Capture the Flag (2 Teams rot & blau, Flags markieren Basis oder Spawnpunkt des jeweiligen Teams, Reaktivierung nur im Bereich des eigenen Spawnpunkts, Punkte bei Aufenthalt in gegnerischen Basis)

### Flag
![Flag](/Flag/Flag_Beispiel.JPG)
Ein Flag sendet in regelmäßigen Abständen einen Code der von den Taggern empfangen wird. Bereich, in dem das Signal empfangbar ist, beträgt in etwa 5-8 Meter im Radius. Diese Strecke ergibt sich durch den Einsatz von ESP32 S2 Mikrocontroller ohne integriete Antenne in den Taggern.

### Verbesserungsmöglichkeiten

- Reichweite bei starkem Sonnenschein eingeschränkt
- Mikrocontroller überträgt kurzes Störgeräusch über I2S nach Ende einer Soundausgabe (Software Problem)
- Skalierbarkeit (mehr als 10 Spieler)
