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
      
