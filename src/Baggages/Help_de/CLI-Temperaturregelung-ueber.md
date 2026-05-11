### Temperaturregelung über

Legt fest, wie die Temperaturregelung erfolgt.

- Solltemperaturvorgabe (Regelung über Aktor)
  Die Temperaturregelung wird durch den Aktor vorgenommen.
  Dieser bekommt die Solltemperatur vorgegeben.
- Stellwertvorgabe (Motorventil, Aktor)
  Die Regelung erfolgt durch die HVAC Klimasteuerung.
  Der Aktor oder das Motorventil bekommt direkt den Stellwert vorgegeben.
  Mit der Einstellung "Heizungstyp" kann der HVAC-interne Regler konfiguriert werden.
- Pulsweiten Modulation (thermoelektrisches Ventil)
  Die Regelung erfolgt durch die HVAC Klimasteuerung.
  Ein thermoelektrisches Ventil kann über einen Schaltaktor in diesem Modus entsprechend gesteuert werden.
  Über die Einstellung "Pulsweitenmodulation Periode" können die Ein- bzw. Ausschaltintervalle, die anhand des intern berechneten Stellwertes festgelegt werden.
  Mit der Einstellung "Heizungstyp" kann der HVAC-interne Regler und somit die interne Stellwertberechnung konfiguriert werden.
- Regelung durch OpenKNX über angepasste Solltemperaturvorgabe
  Dieser Modus kann verwendet werden, wenn das Kühl-/Heizgerät keinen externen KNX-Raumtemperatursensor verwenden kann, dieser aber verwendet werden soll.
  Beispielsweise erlauben Klimaanlagen häufig keine externe Raumtemperaturvorgabe. 
  In diesem Modus wird die Abweichung zwischen dem KNX-Raumtemperatursensor und dem Raumtemperatursensor des Kühl-/Heizgerätes verwendet, um den Sollwert für Kühl-/Heizgeräte zu errechnen, der die Raumtemperatur am KNX-Raumtemperatursensor erreichen lässt.
  WICHTIG: Die Sollwerttemperaturvorgabe direkt am Kühl-/Heizgerät darf nicht mehr verwendet werden und die Raumtemperaturmessung des Kühl-/Heizgerät muss an das Gruppenobjekt "Kühl-/Heizsystem X: Solltemperatur" angeschlossen werden.


