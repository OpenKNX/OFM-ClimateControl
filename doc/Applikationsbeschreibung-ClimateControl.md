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
### Objekt

Umschaltung wird durch ein Gruppentelegram ausgelöst:
Sommer = 0, Winter = 1

<!-- DOC  -->
### Datum

Umschaltung anhand von einem Datumsbereich.

<!-- DOC -->
### Sommerbetrieb ab

Tag, an dem auf Sommerbetrieb umgeschalten wird

<!-- DOC -->
### Winterbetrieb ab

Tag, an dem auf Winterbetrieb umgeschalten wird

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
### Bei Sommerbetriebsstart Wechsel auf

Modus auf dem beim Beginn des Sommerbetriebes umgeschalten wird.

Auswahl:
- Deaktiviert
  Es erfolgt kein Moduswechsel
- Aus
  Das Heiz-/Kühlsystem wird ausgeschaltet
- Auto
  Es wird auf dem Automatikmodus gewechselt
- Heizen
  Es wird der Heizbetrieb aktiviert
- Kühlen
  Es wird der Kühlbetrieb aktiviert
- Ventilator
  Es wird der Ventilatorbetrieb aktiviert
- Entfeuchten
  Es wird der Entfeuchtungsbetrieb aktiviert


<!-- DOC -->
### Bei Winterbetriebsstart Wechsel auf

Modus auf dem beim Beginn des Winterbetriebes umgeschalten wird.

Auswahl:
- Deaktiviert
  Es erfolgt kein Moduswechsel
- Aus
  Das Heiz-/Kühlsystem wird ausgeschaltet
- Auto
  Es wird auf dem Automatikmodus gewechselt
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
### Kanal deaktivieren (zu Testzwecken)

Über diese Einstellung kann der Kanal deaktiviert werden, ohne das die Gruppenaddressenzuordnungen verloren gehen.

<!-- DOC -->
### Fenster offen Behandlung 

Ist diese Option aktiv, wird ein Reiter "Fenster offen" eingeblendet. 
Dort können Aktionen konfiguriert werden, die ausgelöst werden, wenn Fenster geöffnet werden.
Damit kann beispielsweise der Heizbetriebt reduziert werden oder ein Alarm bei lange geöffneten Fenster ausgelöst werden.

<!-- DOCEND -->
## Solltemperatur

In diesem Abschnitt wird festgelegt, wie die Solltemperatur vorgegeben wird.

<!-- DOC -->
### Nach Gerätestart

Über die Einstellung wird gesteuert, wie die Solltemperatur festgelegt werden soll.

- Gespeicherter Werte, sonst Initialwert
  Wurden vor dem Gerätestart der Wert gespeichert, wird diese verwendet. 
  Ansonsten werden der Initialwert geladen.
- Vom Bus lesen, sonst Initialwert
  Es wird ein Lesetelegram auf den Bus geschickt, wird keine Antwort empfangen wird der Initialwert geladen.
- Vom Bus lesen, sonst gespeicherter Wert, sonst Initialwert
  Es wird ein Lesetelegram auf den Bus geschickt, wird keine Antwort empfangen, wird zuerst versucht den gespeicherten Wert zu laden.
  Wurden kein gespeicherter Werte gefunden, wird der Initialwert verwendet.
      
<!-- DOC -->
### Bei ungültiger Solltemperaturvorgabe

Folgende Einstellmöglichkeiten können gewählt werden:

- Auf Minimal-/Maximalwert korrigieren
  Die Vorgabe wird auf den konfigurierten maximalen bzw. mininmalen Wert korriegiert

- Ignorieren
  Telegramme die ungültige Werte beinhalten werden ignoriert und die aktuelle Solltemperatur wird beibehalten

Hinweis: Werden ungültige Werte empfangen, wird ein Solltemperatur Status Telegram mit dem korrigierten Wert gesendet.

<!-- DOC -->
### Objekt für relative Solltemperaturänderung

Diese Option blendet ein Gruppenobjekt zur relativen Anpassung der Solltemperatur ein.
In den Einstellungen "Erhöhen / Verringern um" wird der Offset für die relative Änderung festgelegt.

<!-- DOC  -->
### Getrennte Solltemperatur für Heiz- und Kühlbetrieb

Ist diese Option aktiv, wird die Solltemperatur im Heiz- und Kühlbetrieb getrennt gespeichert und verwaltet.
Die Werte für "Initialwert", "Minimal", "Maximal", "Erhöhen / Verringern um" können in diesem Fall für den Heiz- bzw. Kühlbetrieb unterschiedlich festgelegt werden.

<!-- DOC -->
### Initialwert 

Legt die Initiale Solltemperatur fest, wenn diese beim Gerätstart nicht über eine andere Option festgelegt wird.
Siehe Einstellung "Nach Gerätestart".

<!-- DOC -->
### Minimal 

Minimale Solltemperatur.

<!-- DOC -->
### Maximal 

Maximale Solltemperatur.

<!-- DOC -->
### Erhöhren / Verrringern um

Änderung des Sollwerts über den Gruppenobjekt-Eingang "Solltemperatur erhöhen/verringern".
Optionen:

- 0,5 K
- 1 K
- 2 K
- 3 K
- 4 K
- 5 K 

<!-- DOCEND -->
## Modus

In diesem Abschnitt wird das Verhalten des HVAC Modus Auswahl Gruppenobjekt-Eingangs konfiguriert.

<!-- DOC -->
### Modus Auswahl schaltet EIN

Ist diese Auswahl auf "Ja", wird beim Empfang eines HVAC Modus Telegrams das Gerät eingeschaltet.
ISt diese Auswahl auf "Nein", wird beim Empfang eines HVAC Modus Telegrams das Gerät im ausgeschalten Zustandes nicht aktivert, jedoch der Modus vorgewählt damit ein späteres Einschalten des Geräts diesen startet.

<!-- DOC -->
### 'Modus Aktueller Status' bei AUS

Über diese Option wird das Verhalten des Gruppenobjekt-Ausgangs 'Modus Auswahl Aktueller Status' beim Ausschalten des Gerätes festgelegt.

- Gewählten Modus für eingeschaltetes Gerät
  Sendet den Modus, der beim Wiedereinschalten ausgewählt wird

- Sendet AUS
  Sendet das HVAC Telegram "AUS" (Wert 6)


<!-- DOC HelpContext="StartMode" -->
### Nach Gerätestart

Über die Einstellung wird gesteuert, wie der aktuelle Modus festgelegt werden soll.

- Gespeicherter Werte, sonst Initialwert
  Wurden vor dem Gerätestart der Wert gespeichert, wird diese verwendet. 
  Ansonsten werden der Initialwert geladen.
- Vom Bus lesen, sonst Initialwert
  Es wird ein Lesetelegram auf den Bus geschickt, wird keine Antwort empfangen wird der Initialwert geladen.
- Vom Bus lesen, sonst gespeicherter Wert, sonst Initialwert
  Es wird ein Lesetelegram auf den Bus geschickt, wird keine Antwort empfangen, wird zuerst versucht den gespeicherten Wert zu laden.
  Wurden kein gespeicherter Werte gefunden, wird der Initialwert verwendet.

<!-- DOC HelpContext="DefaultMode" -->

- Abhängig Sommer/Winterbetrieb (Einstellung aus Sommer- Winterbetriebstart)
  Abhängig ob der Sommer- oder Winterbetrieb aktiv ist, wird die Einstellung aus der Betriebart für den jeweiligen Sommer- oder Winterbetrieb verwendet.
- Auto
  Es wird der Automatikmodus aktiviert
- Heizen
  Es wird der Heizungsmodus aktiviert
- Kühlen
  Es wird der Kühlmdus aktiviert
- Ventilator
  Es wird der Ventilatormodus aktiviert
- Entfeuchten
  Es wird der Entfeuchtungsmodus aktiviert

<!-- DOCEND  -->
## Kühl-/Heizsystemname 1/2

In diesem Reiter wird die konfiguration für das angeschlossen Kühl-/Heizsystem vorgenommen.

<!-- DOC HelpContext="Kuehl-Heizsystemname" -->
### Kühl-/Heizsystemname

Der Name des Kühl-/Heizsystems.
Dieser wird in der Anwendung für die Bennenung der Gruppenobjekte verwendet.

Beispiel: "Fußbodenheizung Wohnzimmer"

<!-- DOC  -->
### Modusauswahl über

Legt fest, wie der Aktor für das Kühl-/Heizsystem angesteuert werden soll umd die Betriebsart auszuwählen:

Optionen:

- HVAC 
  HVAC mit Auto=0 / Heizen=1 / Kühlen=3 / Aus=6 / Lüfter=9 / Entfeuchten=14
- HVAC und Power
  HVAC mit Auto=0 / Heizen=1 / Kühlen=3 / Lüfter=9 / Entfeuchten=14 und ein Zusätzliche Schalt KO
- Ein Objekt pro Modus
  Für jeden Modus wird ein Objekt bereit gestellt

<!-- DOC  -->
### Temperaturregelung über

Legt fest, wie die Temperaturregelung erfolgt.

- Solltemperaturvorgabe (Regelung über Aktor)
  Die Temperaturregelung wird durch den Aktor vorgenommen.
  Dieser bekommt die Solltemperatur vorgegeben.
- Stellwertvorgabe (Motorventil, Aktor)
  Die Regelung erfolgt durch die HVAC Klimasteuerung.
  Der Aktor oder das Motorventil bekommt direkt den Stellwert vorgegeben.
  Mit der Einstellung "Heizungstype" kann der HVAC interne Regler konfiguriert werden.
- Pulsweiten Modulation (thermoelektrisches Ventil)
  Die Regelung erfolgt durch die HVAC Klimasteuerung.
  Ein thermoelektrisches Ventil kann über einen Schaltaktor in diesem Modus entsprechend gesteuert werden.
  Über die Einstellung "Pulsweitenmodulation Periode" können die Ein- bzw. Ausschaltintervalle die anhand des intern berechneten Stellwertes festgelegt werden.
  Mit der Einstellung "Heizungstype" kann der HVAC interne Regler und somit die interne Stellwertberechnung konfiguriert werden.
- Regelung durch OpenKNX über angepasste Solltemperaturvorgabe
  Dieser Modus kann verwendet werden, wenn das Kühl-/Heizgerät keinen externen KNX-Raumtemperatursensor verwenden kann, die aber verwendet werden soll. 
  Beispielsweise erlauben Klimanalagen häufig keine externe Raumtemperaturvorgabe. 
  In diesem Modus wird die Abweichung zwischen dem KNX-Raumtemperatursensor und dem Raumtemperatursensor des Kühl-/Heizgerätes verwendet, um den Sollwert für Kühl-/Heizgeräte zu errechnen, der die Raumtemperatur am KNX-Raumtemperatursensor erreichen lässt.
  WICHTIG: Die Sollwerttemperaturvorgabe direkt am Kühl-/Heizgerät darf nicht mehr verwendet werden und die Raumtemperaturmessung des Kühl-/Heizgerät muss an das Gruppenobjekt "Kühl-/Heizsystem X: Solltemperatur" angeschlossen werden.


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
### Heizungstype 

Legt fest, welche Parameter für den internen PI-Regler verwendet werden.
Die auswählbaren Standardwerte sollten bei den meisten Heiz-/Kühlsystemen funktionieren ohne das es zu Schwingungen in der Regelung kommt. 
In Einzelfällen kann es jedoch notwendig sein den Auswahl "Benutzerdefiniert" zu aktivieren um die Standardwerte anzupassen. 

- Fußbodenheizung (5K / 160min)
- Radiator (3K / 80min)
- Luftheizung (2K / 30min)
- Benutzerdefiniert 
  Ermöglich die Vorgabe der PI-Reglerparameter

<!-- DOC -->
### Proportional 

Der Proportionalband-Wert (Xp) des PI-Reglers in Kelvin.

Er gibt an, wie groß die Temperaturdifferenz zwischen Soll- und Istwert sein muss, damit das Stellventil vollständig geöffnet wird (100% Stellwert).

Beispiel bei Proportional = 5 K:
- Abweichung 0 K → Stellwert 0 %
- Abweichung 2,5 K → Stellwert 50 %
- Abweichung 5 K → Stellwert 100 %

Ein kleinerer Wert führt zu einer stärkeren (aggressiveren) Reaktion auf Temperaturabweichungen, birgt aber die Gefahr von Schwingungen.
Ein größerer Wert macht den Regler träger und stabiler, führt aber zu einer größeren bleibenden Regelabweichung (die durch den I-Anteil ausgeglichen wird).

<!-- DOC -->
### Nachstellzeit

Die Nachstellzeit (Ti) des PI-Reglers in Minuten.

Der I-Anteil (Integralanteil) des Reglers gleicht die bleibende Regelabweichung aus, die der P-Anteil allein nicht vollständig beseitigen kann. Die Nachstellzeit legt fest, wie schnell dieser Ausgleich erfolgt.

Eine kurze Nachstellzeit bedeutet, dass der I-Anteil die Abweichung schnell ausregelt, erhöht jedoch das Risiko von Schwingungen.
Eine lange Nachstellzeit bewirkt einen langsameren, dafür aber stabileren Ausgleich.

Die Nachstellzeit sollte an die thermische Trägheit des Heizsystems angepasst werden:
- Fußbodenheizung: ca. 160 min (sehr träges System)
- Radiator: ca. 80 min (mittlere Trägheit)
- Luftheizung: ca. 30 min (schnelles System)


<!-- DOC -->
### Maximale Genauigkeit der Solltemperatur

Gibt die maximale Genauigkeit der Solltemperatur für das Kühl-/Heizsystem vor.

- Deaktiviert
  Die Solltemperatur wird mit der vollen Genauigkeit weitergegeben.
- 0,1°C
  Die Solltemperatur wird auf 0,1 Grad Celisus gerundet.
- 0,5°C
  Die Solltemperatur wird auf 0,5 Grad Celisus gerundet.
- 1°C
  Die Solltemperatur wird auf 1 Grad Celisus gerundet.

<!-- DOC -->
### Ist Aktiv Rückmeldung

Erfolgt die Temperaturregelung nicht über die HVAC Klimasteuerung, wird für die Rückmeldung ob der Heiz- oder Kühlbetrieb aufgrund der Temperatur gerade aktiv ist folgende Einstellung benutzt:

- Intern berechnet
  Die Raumtemperatur wird mit der Solltemperatur verglichen, daraus wird abgeleitet ob das Kühl-/Heizgerät gerade aktiv ist.

- Rückmeldung Objekt EIN/AUS
  Es wird ein Gruppenobjekt eingeblendet, das die Rückmeldung des Aktors über ein Ein/Aus Telegrams ermöglicht.

- Rückmeldung Objekt Prozent" Value="2" Id="%ENID%" op:headerName="FeedbackPercent" - 
  Es wird ein Gruppenobjekt eingeblendet, das den Aktorstellwert empfängt. 
  Ist der Stellwert ungleich 0% wird dies als 'Aktiv' intepretiert.

<!-- DOCEND  -->
### Modusauswahl

In diesem Abschnitt wird konfiguriert welche Betriebsarten über die HVAC Modusauswahl verwendet werden können.

<!-- DOC  -->
### Durch



<!-- DOCEND  -->
### Modusauswahl

<!-- DOC  -->
### Mehr Kanäle 

Ermöglicht weitere Kanäle hinzuzufügen.



<!-- DOC  -->
### Objekt zum vollständigen Ein/-Ausschalten

Mithilfe dieses Gruppenobjektes kann ein Schaltaktor angesteuert werden, der das KlimaKühl-/Heizsystem vollständig vom Strom trennt, wenn es nicht benötigt wird.

<!-- DOC  -->
### Wartezeit nach Einschalten

Zeit die nach dem Einschalten des Stromes gewartet wird, bis das Kühl-/Heizsystem angesprochen wird





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



<!-- DOC HelpContext="FensterOffen" -->
### Fenster Offen 

<!-- DOC -->
### Ausführen

<!-- DOC -->
### Aktion

<!-- DOC HelpContext="Aktionnach" -->
### nach





<!-- DOC -->
### Kühlbetrieb plus Heizbetrieb minus

<!-- DOC -->
### Aktionen rückgängig

<!-- DOC -->
### Rükfall auf Automatik nach





<!-- DOC -->
### Hysterese Heizen

<!-- DOC -->
### Hysterese Kühlen



