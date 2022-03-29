# ESP32-LaserTag
Easy Open Source LaserTag System
### Überblick
Ziel dieses Projektes war der Bau eines einfachen LaserTag-Systems. Es sollte schnell und mit wenig Hardware an beliebigen Orten aufbaubar sein und  und über ein Handy jeglicher Betriebssysteme gesteuert werden können.

Weitere Features sind:

- Soundausgabe für Schuss, Treffer und Nachladen (insgesamt 5 verschiedene)
- Display in jedem Tagger
- lange Akkulaufzeit des gesamten Systems
- 3 Spielmodi
- Taschenlampe
- rote Laserdiode für Schuss-Simulation
- 5 in den Tagger integriete Infrarot-Empfänger
    
Das LaserTag-System besteht aus zwei elementaren Bestandteilen. Die Basisstation dient als Verbindungsglied zwischen den Taggern, der einzelnen Spieler, und einer Http-Website, welche der Konfiguration und Auswertung dient. Der Datenaustausch mit den Spielern erfolgt drahtlos mit Hilfe des EspNow-Protokolls der ESP32 S2 Mikrocontroller. Zum Spielen sind zwischen 2 und 10 Tagger notwendig. Für den Capture the Flag Spielmodus werden zusätzlich noch Flags benötigt, die die Basis und Spawnpunkt eines Team markieren.

### Basisstation
![Basisstation](/Basisstation/Basisstation_Beispiel.JPG)
Nach dem Einschalten der Basisstation wird en Wlan Netzwerk erzeugt und die Konfigurationswebsite gehostet. Wenn man sich hier mit einem Smartphone einloggt, kann man die Anzahl der Spieler, Dauer des Spiels, Modus, Namen der Spieler und Teamzugehörigkeit. Die maximal 10 Spieler können auf 6 Teams verteilt werden. Vor dem Starten des Spiels mit dem Start-Taster müssen unbedingt alle Tagger, die verwendet werden sollen, eingeschaltet werden, damit diese von der Basisstation ihre Informationen erhalten können. Nach Ablauf der Spieldauer übermitteln die Tagger automatisch die Spielstände an die Basisstation und schalten sich ab. Die Basisstation berechnet die Ergebnisse und hostet diese in Form einer Website. Das hat den Vorteil, dass sich theoretisch mehrere Spieler die Ergebnisse auf ihren Smartphones anschauen können.

### Tagger
![Tagger](/Tagger/Tagger_Beispiel.JPG)
