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

