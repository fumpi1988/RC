#include <Bluepad32.h>

// —— Pinbelegung (anpassen bei Bedarf) ——
const int PWM_LEFT    = 22;
const int BRAKE_LEFT  = 16;
const int DIR_LEFT    = 17;

const int PWM_RIGHT   = 18;
const int BRAKE_RIGHT = 19;
const int DIR_RIGHT   = 21;

// —— Steuerparameter ——
const int DEADZONE = 25;
const int MIN_PWM  = 20;

// —— Geschwindigkeitsstufen ——
const int SPEED_STEPS = 5;
const uint8_t maxSpeedLevels[SPEED_STEPS] = {
  51, 102, 153, 204, 255  // 20 %, 40 %, ..., 100 %
};
const uint8_t lightColors[SPEED_STEPS][3] = {
  {0, 255, 0},     // Grün
  {0, 0, 255},     // Blau
  {255, 255, 0},   // Gelb
  {255, 165, 0},   // Orange
  {255, 0, 0}      // Rot
};

// —— Controller und Geschwindigkeitsstufe ——
ControllerPtr ps4Controller = nullptr;
int speedStage = 1;
bool prevDpadUp = false;
bool prevDpadDown = false;

// —— Callback bei Verbindung ——
void onConnectedController(ControllerPtr ctl) {
  Serial.println("PS4-Controller verbunden.");
  ps4Controller = ctl;

  // PWM erst hier initialisieren
  ledcSetup(0, 1000, 8);        // Kanal 0 – links
  ledcSetup(1, 1000, 8);        // Kanal 1 – rechts
  ledcAttachPin(PWM_LEFT, 0);
  ledcAttachPin(PWM_RIGHT, 1);
  ledcWrite(0, 0);
  ledcWrite(1, 0);

  auto& c = lightColors[speedStage - 1];
  ctl->setColorLED(c[0], c[1], c[2]);
}


// —— Callback bei Trennung ——
void onDisconnectedController(ControllerPtr ctl) {
  Serial.println("PS4-Controller getrennt.");
  if (ps4Controller == ctl) {
    // Motoren stoppen und Bremsen aktivieren
    ledcWrite(0, 0);
    ledcWrite(1, 0);
    digitalWrite(BRAKE_LEFT, HIGH);
    digitalWrite(BRAKE_RIGHT, HIGH);
    ps4Controller = nullptr;
  }
}

// —— LED aktualisieren ——
void updateLightbar() {
  if (!ps4Controller) return;
  auto& c = lightColors[speedStage - 1];
  ps4Controller->setColorLED(c[0], c[1], c[2]);
}

// —— D-Pad Auswertung ——
void handleDpad() {
  if (!ps4Controller) return;
  uint8_t dpadState = ps4Controller->dpad();

  bool up = dpadState == DPAD_UP;
  bool down = dpadState == DPAD_DOWN;

  if (up && !prevDpadUp) {
    if (speedStage < SPEED_STEPS) {
      speedStage++;
      updateLightbar();
      Serial.printf("Stufe erhöht: %d (max %d%%)\n", speedStage, speedStage * 20);
    }
  }
  if (down && !prevDpadDown) {
    if (speedStage > 1) {
      speedStage--;
      updateLightbar();
      Serial.printf("Stufe reduziert: %d (max %d%%)\n", speedStage, speedStage * 20);
    }
  }

  prevDpadUp = up;
  prevDpadDown = down;
}

void setup() {
  Serial.begin(115200);

  pinMode(PWM_LEFT, OUTPUT);
  pinMode(BRAKE_LEFT, OUTPUT);
  pinMode(DIR_LEFT, OUTPUT);
  pinMode(PWM_RIGHT, OUTPUT);
  pinMode(BRAKE_RIGHT, OUTPUT);
  pinMode(DIR_RIGHT, OUTPUT);

  digitalWrite(BRAKE_LEFT, HIGH);   // Zu Beginn Bremsen aktiv
  digitalWrite(BRAKE_RIGHT, HIGH);

  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.forgetBluetoothKeys();  // Optional: alte Verbindungen löschen

  Serial.println("Bereit für PS4-Controller...");
}

void loop() {
  BP32.update();

  if (ps4Controller && ps4Controller->isConnected()) {
    handleDpad();

    // Wenn X gedrückt wird: Beide Motoren bremsen (Not-Stopp)
    if (ps4Controller->x()) {
      ledcWrite(0, 0);
      ledcWrite(1, 0);
      digitalWrite(BRAKE_LEFT, HIGH);
      digitalWrite(BRAKE_RIGHT, HIGH);
      return;
    }

    int maxPWM = maxSpeedLevels[speedStage - 1];
    int minPWM = MIN_PWM < maxPWM ? MIN_PWM : maxPWM;

    // Linker Stick → linker Motor
    int yL = ps4Controller->axisY();
    if (abs(yL) <= DEADZONE) {
      ledcWrite(0, 0);
      digitalWrite(BRAKE_LEFT, HIGH);  // Bremse aktiv in Nullstellung
    } else {
      digitalWrite(BRAKE_LEFT, LOW);   // Bremse lösen beim Fahren
      bool reverse = (yL > 0);
      digitalWrite(DIR_LEFT, reverse ? HIGH : LOW);
      int pwm = map(abs(yL), DEADZONE, 512, minPWM, maxPWM);
      pwm = constrain(pwm, 0, maxPWM);
      ledcWrite(0, pwm);
    }

    // Rechter Stick → rechter Motor
    int yR = ps4Controller->axisRY();
    if (abs(yR) <= DEADZONE) {
      ledcWrite(1, 0);
      digitalWrite(BRAKE_RIGHT, HIGH);  // Bremse aktiv in Nullstellung
    } else {
      digitalWrite(BRAKE_RIGHT, LOW);   // Bremse lösen beim Fahren
      bool reverse = (yR > 0);
      digitalWrite(DIR_RIGHT, reverse ? HIGH : LOW);
      int pwm = map(abs(yR), DEADZONE, 512, minPWM, maxPWM);
      pwm = constrain(pwm, 0, maxPWM);
      ledcWrite(1, pwm);
    }
  }

  delay(10);
}
