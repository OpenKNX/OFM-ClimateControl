### Nach Gerätestart

Über die Einstellung wird gesteuert, wie die Solltemperatur festgelegt werden soll.

- Gespeicherter Wert, sonst Initialwert
  Wurde vor dem Gerätestart ein Wert gespeichert, wird dieser verwendet. 
  Ansonsten wird der Initialwert geladen.
- Vom Bus lesen, sonst Initialwert
  Es wird ein Lesetelegramm auf den Bus geschickt, wird keine Antwort empfangen, wird der Initialwert geladen.
- Vom Bus lesen, sonst gespeicherter Wert, sonst Initialwert
  Es wird ein Lesetelegramm auf den Bus geschickt, wird keine Antwort empfangen, wird zuerst versucht den gespeicherten Wert zu laden.
  Wurde kein gespeicherter Wert gefunden, wird der Initialwert verwendet.
      
