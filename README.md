ESP32 Smart Parking System

An embedded firmware solution for automated vehicle entry/exit management, built on the ESP32 microcontroller. Features dual ultrasonic sensors, servo-controlled gate, flame/smoke detection, and ambient light-based LED control.


Features


Directional vehicle detection via dual HC-SR04 ultrasonic sensors
Servo-driven gate with open/close state management
Real-time occupancy tracking — up to 20 parking slots
Flame & smoke alarm with non-blocking variable-rate buzzer
Smart LED lighting — auto ON/OFF based on ambient light (LDR)
FSM-based gate logic — prevents race conditions and double-counting
Serial debug output at 115200 baud for live monitoring



Hardware

Pin Configuration

PinGPIOTypeDescriptionFLAME_PIN34Analog InputFlame/IR sensorSMOKE_PIN35Analog InputMQ smoke/gas sensorBUZZER_PIN23Digital OutputActive buzzerLDR_PIN32Analog InputAmbient light sensorLED_PIN2Digital OutputSmart LEDTRIG1 / ECHO15 / 18DigitalUltrasonic sensor 1 (entry side)TRIG2 / ECHO217 / 16DigitalUltrasonic sensor 2 (exit side)SERVO_PIN13PWM OutputGate servo

Wiring Diagram

ESP32
├── GPIO 34  ←── Flame Sensor (AO)
├── GPIO 35  ←── Smoke Sensor (AO)
├── GPIO 32  ←── LDR
├── GPIO 23  ───► Buzzer
├── GPIO 2   ───► LED
├── GPIO 5   ───► Ultrasonic 1 TRIG
├── GPIO 18  ←── Ultrasonic 1 ECHO
├── GPIO 17  ───► Ultrasonic 2 TRIG
├── GPIO 16  ←── Ultrasonic 2 ECHO
└── GPIO 13  ───► Servo Signal


Software

Dependencies

LibraryInstallESP32ServoArduino Library Manager → search ESP32Servo

Board Setup (Arduino IDE)


Board: ESP32 Dev Module
Upload Speed: 115200
Flash Frequency: 80 MHz
Partition Scheme: Default 4MB with spiffs



How It Works

Finite State Machine (Gate Logic)

         S1 triggered          S2 triggered
IDLE ──────────────────► ENTRY_WAIT ──────────────► WAIT_CLEAR
  │                                                      │
  │   S2 triggered          S1 triggered                 │ both clear
  └────────────────► EXIT_WAIT ───────────────────►      │
                                                         ▼
                                                   update count
                                                   close gate
                                                      IDLE


Entry: S1 triggers → gate opens → vehicle passes S2 → gate closes → occupied++
Exit: S2 triggers → gate opens → vehicle passes S1 → gate closes → occupied--
Gate stays open until both sensors are clear, preventing premature close


Buzzer Alarm

ConditionBehaviorNo hazardSilentFlame or SmokeSlow beep (500ms interval)Flame and SmokeRapid beep (100ms interval)


Configuration

All settings are defined as constants at the top of the sketch:

cppconst int FLAME_THRESHOLD    = 50;    // ADC below this = fire detected
const int SMOKE_THRESHOLD    = 3000;  // ADC above this = smoke detected
const int LDR_THRESHOLD      = 2500;  // ADC above this = dark, LED ON
const float DETECTION_THRESHOLD = 15.0; // cm — object within range
const int TOTAL_SLOTS        = 20;    // parking capacity
const int SERVO_OPEN         = 90;    // gate open angle (degrees)
const int SERVO_CLOSE        = 0;     // gate closed angle (degrees)

Calibrate thresholds using Serial Monitor output:

Flame:2048 Smoke:1100 LDR:3200 Occupied:5


Serial Output

Connect at 115200 baud. Each loop prints:

Flame:<value> Smoke:<value> LDR:<value> Occupied:<count>


Known Limitations


Occupancy count resets to 0 on power cycle (no persistent storage)
Single-lane design — does not handle simultaneous entry/exit on separate lanes
No timeout if a vehicle stalls mid-transit (FSM stays in ENTRY_WAIT / EXIT_WAIT)
Ultrasonic sensors may give false readings in rain, fog, or with soft surfaces



License

MIT License — free to use, modify, and distribute
