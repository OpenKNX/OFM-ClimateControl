### Ist Aktiv Rückmeldung

Erfolgt die Temperaturregelung nicht über die HVAC Klimasteuerung, wird für die Rückmeldung ob der Heiz- oder Kühlbetrieb aufgrund der Temperatur gerade aktiv ist folgende Einstellung benutzt:

- Intern berechnet
  Die Raumtemperatur wird mit der Solltemperatur verglichen, daraus wird abgeleitet ob das Kühl-/Heizgerät gerade aktiv ist.

- Rückmeldung Objekt EIN/AUS
  Es wird ein Gruppenobjekt eingeblendet, das die Rückmeldung des Aktors über ein Ein/Aus-Telegramm ermöglicht.

- Rückmeldung Objekt Prozent" Value="2" Id="%ENID%" op:headerName="FeedbackPercent" - 
  Es wird ein Gruppenobjekt eingeblendet, das den Aktorstellwert empfängt. 
  Ist der Stellwert ungleich 0 % wird dies als 'Aktiv' interpretiert.

