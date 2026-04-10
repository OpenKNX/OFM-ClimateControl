<!-- SPDX-License-Identifier: AGPL-3.0-only -->
<!-- Copyright (C) 2026 Michael Geramb -->

# Applikationsbeschreibung Klimasteuerung

Die OpenKNX Klimasteuerung ermöglicht die Steuerung / Regelung von 2 unabhängigen Heiz-/Kühlsystemen über einen HVAC Eingang.

Features:

- Fenster offen Behandlung
- Sommer- / Winterbetriebumschaltung
- 24h Durchschnittstemperature Berechnung
- Wiederherstellen des eingestellten Modus, der Solltemperatur und der letzten Außentemperaturwerte nach Busspannungsausfall und Gerätestart

<!-- DOC HelpContext="KlimaSteuerung" -->
## Klimasteuerung (HVAC)

In diesem Abschnitt werden die Basiseinstellungen der Klimasteuerung vorgenommen.

<!-- DOCEND  -->
## Kanalauswahl

Hier wird die Anzahl der verfügbaren Kanäle ausgewählt.
Üblicherweise wird pro Raum ein Kanal verwendet.

<!-- DOC  -->
### Anzahl der Räume

Anzahl der Räume für die ein Kanal angezeigt wird.

<!-- DOCEND  -->
## Sommer- / Winterbetriebsumschaltung

Über die Sommer- und Winterbetriebsumstellung können Heiz- bzw. Kühlsystem deaktiviert werden.

<!-- DOCEND -->
Die Umschaltung erfolgt anhand folgender Kriterien:

<!-- DOC  -->
### Gruppenobjekt

Umschaltung wird durch ein Gruppentelegram ausgelöst

<!-- DOC  -->
### Datum

Umschaltung anhand von einem Datumsbereich

<!-- DOC -->
### Sommerbetrieb ab

Tag an dem auf Sommerbetrieb umgeschalten wird

<!-- DOC -->
### Winterbetrieb ab

Tag an dem auf Winterbetrieb umgeschalten wird

<!-- DOC  -->
### 24h-Temperaturdurchschnitt 

Umschaltung auf Basis der Durschnittstemperature der letzten 24h

<!-- DOC  -->
### Sommer bei ≥

Temperatur ab der auf Sommer geschalten wird.

<!-- DOC  -->
### Winter bei ≤

Temperatur unter der auf Winter geschalten wird.

<!-- DOC  -->
#### für

Anzahl der Tage mit eingestellten Temperaturgrenze nach der die Umschaltung erfolgt.

<!-- DOC  -->
### Bei-Sommerbetriebsstart Wechsel auf

Modus auf dem beim Beginn des Sommerbetriebes umgeschalten wird.

Auswahl:
- Deaktiviert
  Es erfolgt kein Moduswechsel
- Aus
  Das Heiz-/Kühlsystem wird ausgeschaltet
- Auto
  Es wird auf dem Automodus gewechselt
- Heizen
  Es wird der Heizbetrieb aktiviert
- Kühlen
  Es wird der Kühlbetrieb aktiviert
- Ventilator
  Es wird der Ventilatorbetrieb aktiviert
- Entfeuchten
  Es wird der Entfeuchtungsbetrieb aktiviert


<!-- DOC -->
### Bei-Winterbetriebsstart Wechsel auf

Modus auf dem beim Beginn des Winterbetriebes umgeschalten wird.

Auswahl:
- Deaktiviert
  Es erfolgt kein Moduswechsel
- Aus
  Das Heiz-/Kühlsystem wird ausgeschaltet
- Auto
  Es wird auf dem Automodus gewechselt
- Heizen
  Es wird der Heizbetrieb aktiviert
- Kühlen
  Es wird der Kühlbetrieb aktiviert
- Ventilator
  Es wird der Ventilatorbetrieb aktiviert
- Entfeuchten
  Es wird der Entfeuchtungsbetrieb aktiviert

<!-- DOC HelpContext="Raum"  -->
## Raum 1-n

Für jeden Raum wird eine Konfigurationslasche angezeigt.
Ist der Name des Raums festgelegt, wird dieser anstatt "Raum 1-n" angezeigt.

Alle für den Raum notwendigen Einstellungen werden in diesem Abschnitt vorgenommen.

<!-- DOC -->
### Nach Gerätestart

Über die Einstellung wird gesteuert, wie der aktuelle Modus und die Solltemperatur festgelegt werden soll.

- Gespeicherte Werte, sonst Initialwerte
  Wurden vor dem Gerätestart die Werte gespeichert, werden diese verwendet. 
  Ansonsten werden die Initialwerte geladen.
- Vom Bus lesen, sonst Initialwerte
  Es wird ein Lesetelegram auf den Bus geschickt, wird keine Antwort empfangen werden die Initialwerte geladen.
- Vom Bus lesen, sonst gespeicherter Wert, sonst Initialwerte
  Es wird ein Lesetelegram auf den Bus geschickt, wird keine Antwort empfangen, wird zuerst versucht die gespeicherten Werte zu laden.
  Wurden keine gespeicherten Werte gefunden, werden die Initialwerte verwendet.
      
<!-- DOC -->
### Modus Auswahl schaltet EIN

<!-- DOC -->
### Modus Auswahl Aktueller Status bei AUS

<!-- DOC HelpContext="Kuehl-Heizsystemname" -->
### Kühl-/Heizsystemname

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

Mithilfe dieses Gruppenobjektes kann ein Schaltaktor angesteuert werden, der das KlimaKühl-/Heizsystem vollständig vom Strom trennt, wenn es nicht benötigt wird.

<!-- DOC  -->
### Wartezeit nach Einschalten

Zeit die nach dem Einschalten des Stromes gewartet wird, bis das Kühl-/Heizsystem angesprochen wird


<!-- DOC  -->
### Getrennte Solltemperatur für Heiz- und Kühlbetrieb


<!-- DOC  -->
### Berechnung

- Jede Stunde
  Jede volle Stunde wird der Messwert genommen und daraus über 24h der Durchschnitt gebildet.
  Ist die Uhrzeit nicht bekannt, wird jede volle Stunde ab Kühl-/Heizsystemstart verwendet.
- Mannheimer Stunden (T7+T14+T21*2)/4
  Frühere in Deutschland übliche Berechnungsformel.
  Um 7:00, 14:00, 21:00 wird die Temperatur gemessen und über die Formel (T7+T14+T21*2)/4 der Durchschnitt berechnet.
  Ist die Uhrzeit nicht bekannt, wird die Berechnung nicht durchgeführt.
- Minimum-Maximum-Mittel
  Mittelwert aus der täglichen Tiefst- und Höchsttemperatur.
- 24h Durchschnittstemperatur Gruppenobjekt
  Der aktuelle Temperaturdurchschnitt wird über ein Gruppenobjekt eingelesen.

<!-- DOC  -->
#### Leseanfrage zum Berechnungszeitpunkt senden  

Bei den Berechnungsarten "Jede Stunde" und "Mannheimer Stunden (T7+T14+T21*2)/4" wird zu den jeweils benötigen Uhrzeiten ein Lesetelegram gesendet.

<!-- DOC  -->

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

<!-- DOC HelpContext="Manuelle-Aenderung-am-Kuehl-Heizsystem" -->
### Manuelle Änderung am Kühl-/Heizsystem


<!-- DOC -->
### Fenster-offen-Behandlung 

<!-- DOC -->
### Fenster-zu-Behandlung

<!-- DOC -->
### Kühlen im Winterbetrieb gesperrt

<!-- DOC -->
### Heizen im Sommerbetrieb gesperrt

<!-- DOC -->
### Kühlen im Winterbetrieb erlaubt

<!-- DOC -->
### Heizen im Sommerbetrieb erlaubt


<!-- DOC -->
### Initialwert 

<!-- DOC -->
### Minimal 

<!-- DOC -->
### Maximal 

<!-- DOC HelpContext="FensterOffen" -->
### Fenster Offen 

<!-- DOC -->
### Ausführen

<!-- DOC -->
### Aktion

<!-- DOC HelpContext="Aktionnach" -->
### nach


<!-- DOC -->
### Objekt für relative Solltemperaturänderung

<!-- DOC -->
### Erhöhren / Verrringern um

<!-- DOC -->
### Kühlbetrieb plus Heizbetrieb minus

<!-- DOC -->
### Aktionen rückgängig

<!-- DOC -->
### Rükfall auf Automatik nach

<!-- DOC -->
### Maximale Genauigkeit der Solltemperatur

<!-- DOC -->
### Bei falscher Zieltemperatur

<!-- DOC -->
### Heizungstype 

- Fußbodenheizung (5K / 160min)
- Radiator (3K / 80min)
- Luftheizung (2K / 30min)
- Benutzerdefiniert 

<!-- DOC -->
### Proportional 

<!-- DOC -->
### Nachstellzeit

<!-- DOC -->
### Pulsweitenmodulation Periode

Die Einstellung ist abhängig davon, wieviele thermoelektrische Stellventile am Aktorkanal parallel angeschlossen sind.

- ein Ventil
  Der Werte sollte ca. 1/4 der Öffnungs/Schließzeit betragen. 
  Das bewirkt, dass das Ventil sich je nach Stellwert in einer Mittelstellung einpendelt und somit nur entsprechend des Stellwertes geöffnet ist.
  Hinweis: Bei sehr kurzen Zeiten, werden die Schaltkontakte des Aktors stark beansprucht.

- mehrere Ventile
  Der Wert sollte so eingestellt werden, dass ein vollständiges öffnen und schließen der Ventile erreicht wird.
  Damit wird verhindert das durch Bauteiltoleranzen einzelne Ventile mehr oder weniger öffnen.
  Die Stellwertvorgabe wird in dieser Einstellung durch die Trägheit des Heizkreises erreicht.
  Empfohlene Einstellung ist 2x die Öffnung/Schließzeit des Ventiles.

<!-- DOC -->
### Hysterese Heizen

<!-- DOC -->
### Hysterese Kühlen

