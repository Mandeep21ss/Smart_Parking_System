#include <ESP32Servo.h>

// ================= PINS =================
#define FLAME_PIN   34
#define SMOKE_PIN   35
#define BUZZER_PIN  23
#define LDR_PIN     32
#define LED_PIN     2

#define TRIG1 5
#define ECHO1 18
#define TRIG2 17
#define ECHO2 16

#define SERVO_PIN 13

// ================= SETTINGS =================
const int FLAME_THRESHOLD = 50;
const int SMOKE_THRESHOLD = 3000;
const int LDR_THRESHOLD = 2500;

const float DETECTION_THRESHOLD = 15.0;
const int TOTAL_SLOTS = 20;

const int SERVO_OPEN = 90;
const int SERVO_CLOSE = 0;

// ================= OBJECTS =================
Servo gateServo;

// ================= STATE =================
enum State {
  IDLE,
  ENTRY_WAIT,
  EXIT_WAIT,
  WAIT_CLEAR
};

State state = IDLE;

// ================= VARIABLES =================
int occupied = 0;
bool entryMode = false;
bool exitMode = false;

bool gateOpen = false;

int s1Count = 0;
int s2Count = 0;

unsigned long lastBuzzerTime = 0;
bool buzzerState = false;

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);

  gateServo.attach(SERVO_PIN);
  closeGate();

  Serial.println("SYSTEM STARTED");
}

// ================= LOOP =================
void loop() {

  // ===== READ SENSORS =====
  int flame = analogRead(FLAME_PIN);
  int smoke = analogRead(SMOKE_PIN);
  int ldr   = analogRead(LDR_PIN);

  float d1 = getDistance(TRIG1, ECHO1);
  float d2 = getDistance(TRIG2, ECHO2);

  bool flameDetected = flame < FLAME_THRESHOLD;
  bool smokeDetected = smoke > SMOKE_THRESHOLD;

  bool s1 = (d1 < DETECTION_THRESHOLD);
  bool s2 = (d2 < DETECTION_THRESHOLD);

  // ===== SMART LIGHT =====
  digitalWrite(LED_PIN, ldr > LDR_THRESHOLD);

  // ===== FIRE + SMOKE BUZZER (NON-BLOCKING) =====
  handleBuzzer(flameDetected, smokeDetected);

  // ===== PARKING FSM =====
  switch (state) {

    case IDLE:

      if (s1 && !s2) {
        if (occupied < TOTAL_SLOTS) {
          openGate();
          entryMode = true;
          state = ENTRY_WAIT;
        }
      }

      else if (s2 && !s1) {
        if (occupied > 0) {
          openGate();
          exitMode = true;
          state = EXIT_WAIT;
        }
      }
      break;

    case ENTRY_WAIT:
      if (s2) state = WAIT_CLEAR;
      break;

    case EXIT_WAIT:
      if (s1) state = WAIT_CLEAR;
      break;

    case WAIT_CLEAR:
      if (!s1 && !s2) {

        if (entryMode) occupied++;
        if (exitMode) occupied--;

        if (occupied < 0) occupied = 0;
        if (occupied > TOTAL_SLOTS) occupied = TOTAL_SLOTS;

        closeGate();
        resetSystem();
      }
      break;
  }

  // ===== DEBUG =====
  Serial.print("Flame:");
  Serial.print(flame);
  Serial.print(" Smoke:");
  Serial.print(smoke);
  Serial.print(" LDR:");
  Serial.print(ldr);
  Serial.print(" Occupied:");
  Serial.println(occupied);

  delay(50); // small stability delay only
}

// ================= FUNCTIONS =================

// ultrasonic
float getDistance(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  long duration = pulseIn(echo, HIGH, 30000);
  if (duration == 0) return 999;

  return duration * 0.0343 / 2;
}

// buzzer logic
void handleBuzzer(bool flame, bool smoke) {

  unsigned long now = millis();

  int interval = 0;

  if (flame && smoke) interval = 100;
  else if (flame || smoke) interval = 500;
  else {
    digitalWrite(BUZZER_PIN, LOW);
    return;
  }

  if (now - lastBuzzerTime >= interval) {
    lastBuzzerTime = now;
    buzzerState = !buzzerState;
    digitalWrite(BUZZER_PIN, buzzerState);
  }
}

// servo
void openGate() {
  if (!gateOpen) {
    gateServo.write(SERVO_OPEN);
    gateOpen = true;
  }
}

void closeGate() {
  if (gateOpen) {
    gateServo.write(SERVO_CLOSE);
    gateOpen = false;
  }
}

void resetSystem() {
  state = IDLE;
  entryMode = false;
  exitMode = false;
}