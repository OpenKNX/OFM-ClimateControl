<!-- SPDX-License-Identifier: AGPL-3.0-only -->
<!-- Copyright (C) 2026 Michael Geramb -->

# Applikationsbeschreibung Klimasteuerung



<!-- DOC HelpContext="KlimaSteuerung" -->
## Klima Steuerung

<!-- DOC  -->
### Anzahl der Kanäle

<!-- DOC  -->
### Raum

<!-- DOC  -->
### Gerätname

<!-- DOC  -->
### Durch

<!-- DOC  -->
### Kanal deaktivieren zu Testzwecken

<!-- DOC  -->
### Mehr Kanäle 

Ermöglicht weitere Kanäle hinzuzufügen.

<!-- DOC  -->
### Modusauswahl über

- HVAC 
  HVAC mit Auto=0 / Heizen=1 / Kühlen=3 / Aus=6 / Lüfter=9 / Entfeuchten=14
- HVAC und Power
  HVAC mit Auto=0 / Heizen=1 / Kühlen=3 / Lüfter=9 / Entfeuchten=14 und ein Zusätzliche Schalt KO
- Ein Objekt pro Modus
  Für jeden Modus wird ein Objekt bereit gestellt

<!-- DOC  -->
### Temperaturregelung über

<!-- DOC  -->
### Objekt zum vollständigen Ein/-Ausschalten

Mithilfe dieses Gruppenobjektes kann ein Schaltaktor angesteuert werden, der das Klimagerät vollständig vom Strom trennt, wenn es nicht benötigt wird.

<!-- DOC  -->
### Wartezeit nach Einschalten

Zeit die nach dem Einschalten des Stromes gewartet wird, bis das Gerät angesprochen wird

<!-- DOCEND -->
## Sommer- / Winterbetriebsumschaltung

Die Umschaltung erfolgt anhand folgender Kriterien:

<!-- DOC  -->
### Gruppenobjekt

Umschaltung durch ein Gruppentelegram ausgelöst.

<!-- DOC  -->
### Datum

Umschaltung anhand von einem Datumsbereich.

<!-- DOC -->
### Sommerbetrieb ab

<!-- DOC -->
### Winterbetrieb ab

<!-- DOC  -->
### Tagesdurchschnittstemperatur

Umschaltung auf basis der Tagesdurchschnittstemperatur

<!-- DOC  -->
### Getrennte Solltemperaturspeicherung für Sommer und Winterbetrieb


<!-- DOC  -->
### Berechnung

- Jede Stunde
  Jede volle Stunde wird der Messwert genommen und daraus über 24h der Mittelwert gebildet.
  Ist die Uhrzeit nicht bekannt, wird jede volle Stunde ab Gerätstart verwendet.
- Mannheimer Stunden (T7+T14+T21*2)/4
  Frühere in Deutschland übliche Berechnungsformel.
  Um 7:00, 14:00, 21:00 wird die Temperatur gemessen und über die Formel (T7+T14+T21*2)/4 der Mittelwert berechnet.
  Ist die Uhrzeit nicht bekannt, wird die Berechnung nicht durchgeführt.
- Minimum-Maximum-Mittel
  Mittelwert aus der täglichen Tiefst- und Höchsttemperatur.
- Mittelwert Temperatur Gruppenobjekt
  Der aktuelle Mittelwert wird über ein Gruppenobjekt eingelesen.

<!-- DOC  -->
#### Leseanfrage zum Berechnungszeitpunkt senden  

Bei den Berechnungsarten "Jede Stunde" und "Mannheimer Stunden (T7+T14+T21*2)/4" wird zu den jeweils benötigen Uhrzeiten ein Lesetelegram gesendet.

<!-- DOC  -->
### Sommer bei

Temperartur ab der auf Sommer geschalten wird.

<!-- DOC  -->
#### für

Anzahl der Tage bis die Umschaltung erfolgt.

<!-- DOC  -->
### Winter bei

Temperartur ab der auf Winter geschalten wird.

<!-- DOCEND  -->
#### für

Anzahl der Tage bis die Umschaltung erfolgt.

<!-- DOC  -->
### Kühlen Auswahl durch


<!-- DOC  -->
### Heizen Auswahl durch

<!-- DOC  -->
### Entfeuchten Auswahl durch

<!-- DOC  -->
### Ventilator Auswahl durch

<!-- DOC  -->
### Manuelle Änderung am Gerät



