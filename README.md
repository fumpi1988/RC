# PS4 Controller gesteuerter Fahrregler

Dieses Projekt zeigt, wie ein ESP32 mithilfe der [Bluepad32](https://github.com/dreamquake/Bluepad32) Bibliothek einen PS4-Controller als Eingabegerät verwenden kann, um zwei Motoren anzusteuern. Die Pins für PWM, Bremsen und Fahrtrichtung sind im Sketch `controller.ino` definiert.

## Hardware

- ESP32-Board
- Zwei Motorentreiber (linke und rechte Seite)
- Jeweils ein PWM-, Brems- und Richtungs-Pin pro Motor

Die Standardbelegung im Code lautet:

| Funktion | Pin (links) | Pin (rechts) |
|----------|-------------|--------------|
| PWM      | 22          | 18           |
| Bremse   | 16          | 19           |
| Richtung | 17          | 21           |

Bei Bedarf können die Pins im Sketch angepasst werden.

## Installation

1. Arduino IDE oder eine andere ESP32 Entwicklungsumgebung einrichten.
2. Die Bluepad32-Bibliothek installieren.
3. Die Datei `controller.ino` öffnen und auf den ESP32 hochladen.

Nach dem Flashen meldet der ESP32 über die serielle Schnittstelle „Bereit für PS4-Controller...“.

## Steuerung

- **Linker Stick:** Geschwindigkeit und Richtung des linken Motors
- **Rechter Stick:** Geschwindigkeit und Richtung des rechten Motors
- **Steuerkreuz oben/unten:** Wechsel der Geschwindigkeitsstufe (1–5)
- **X-Taste:** Sofortiges Bremsen beider Motoren (Not-Stopp)

Beim Trennen des Controllers werden die Motoren automatisch gestoppt und die Bremsen angezogen【F:controller.ino†L54-L63】.

Die maximal mögliche PWM für jede Stufe ist in `maxSpeedLevels` hinterlegt. Gleichzeitig zeigt die Lightbar-Farbe des Controllers die aktuelle Stufe an.

## Weitere Hinweise

- Im Ruhezustand sind die Bremsen aktiv, sodass das Fahrzeug nicht wegrollt.
- Die Variable `DEADZONE` legt fest, ab welcher Joystick-Bewegung sich die Motoren bewegen.
- `BP32.forgetBluetoothKeys()` kann entfernt werden, wenn sich der Controller dauerhaft koppeln soll.

## Lizenz

Dieses Projekt steht ohne besondere Lizenz bereit. Verwenden auf eigene Gefahr.
