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
![This is an image](/Basisstation/Beispiel_Basisstation.jpg)
