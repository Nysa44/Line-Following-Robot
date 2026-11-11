/*
  Line-Following Robot
  Board: Arduino Uno / Nano (ATmega328P)
  Sensors: 5-channel ANALOG IR array, connected to A0-A4
  Motor driver: L298N

  See ../docs/WIRING.md before connecting hardware.
*/

#include <Arduino.h>

// -------------------- USER CONFIGURATION --------------------
const bool LINE_IS_DARK = true;       // true: black line on light floor
const bool MOTOR_LEFT_REVERSED = false;
const bool MOTOR_RIGHT_REVERSED = false;

const int BASE_SPEED = 125;           // 0..255; start slowly
const int MAX_SPEED = 210;            // 0..255
const int MIN_SPEED = 0;

// PID gains: tune for your chassis and track.
float KP = 0.085f;
float KI = 0.000f;
float KD = 0.42f;

// Sensor pins ordered LEFT to RIGHT.
const uint8_t SENSOR_PINS[5] = {A0, A1, A2, A3, A4};

// L298N pins. Remove the ENA/ENB jumpers on the module if using PWM.
const uint8_t ENA = 5;   // Left motor PWM
const uint8_t IN1 = 7;
const uint8_t IN2 = 8;
const uint8_t ENB = 6;   // Right motor PWM
const uint8_t IN3 = 9;
const uint8_t IN4 = 10;

const unsigned long CALIBRATION_TIME_MS = 3000;
const int CALIBRATION_SAMPLES = 100;
const int LINE_DETECT_THRESHOLD = 180; // normalized strength, 0..1000
const unsigned long LOOP_DELAY_MS = 5;

// -------------------- STATE --------------------
int sensorMin[5];
int sensorMax[5];
int sensorRaw[5];
int sensorStrength[5];

float lastError = 0.0f;
float integral = 0.0f;
unsigned long lastControlTime = 0;
int lastSeenDirection = 1; // -1 = line last seen left, +1 = right

// -------------------- MOTOR CONTROL --------------------
void setOneMotor(int speedValue, uint8_t enablePin,
                 uint8_t inputA, uint8_t inputB, bool reversed) {
  speedValue = constrain(speedValue, -255, 255);
  if (reversed) speedValue = -speedValue;

  if (speedValue > 0) {
    digitalWrite(inputA, HIGH);
    digitalWrite(inputB, LOW);
    analogWrite(enablePin, speedValue);
  } else if (speedValue < 0) {
    digitalWrite(inputA, LOW);
    digitalWrite(inputB, HIGH);
    analogWrite(enablePin, -speedValue);
  } else {
    analogWrite(enablePin, 0);
    digitalWrite(inputA, LOW);
    digitalWrite(inputB, LOW);
  }
}

void setMotors(int leftSpeed, int rightSpeed) {
  setOneMotor(leftSpeed, ENA, IN1, IN2, MOTOR_LEFT_REVERSED);
  setOneMotor(rightSpeed, ENB, IN3, IN4, MOTOR_RIGHT_REVERSED);
}

void stopMotors() {
  setMotors(0, 0);
}

// -------------------- SENSOR CALIBRATION --------------------
void calibrateSensors() {
  for (int i = 0; i < 5; i++) {
    sensorMin[i] = 1023;
    sensorMax[i] = 0;
  }

  Serial.println(F("Calibration: move sensors across line and floor."));
  const unsigned long start = millis();
  int samples = 0;

  while ((millis() - start < CALIBRATION_TIME_MS) &&
         samples < CALIBRATION_SAMPLES) {
    for (int i = 0; i < 5; i++) {
      int value = analogRead(SENSOR_PINS[i]);
      if (value < sensorMin[i]) sensorMin[i] = value;
      if (value > sensorMax[i]) sensorMax[i] = value;
    }
    samples++;
    delay(CALIBRATION_TIME_MS / CALIBRATION_SAMPLES);
  }

  // Avoid division by zero if a sensor did not vary during calibration.
  for (int i = 0; i < 5; i++) {
    if (sensorMax[i] - sensorMin[i] < 10) {
      sensorMin[i] = max(0, sensorMin[i] - 5);
      sensorMax[i] = min(1023, sensorMax[i] + 5);
    }
  }
  Serial.println(F("Calibration complete."));
}

void readSensors() {
  for (int i = 0; i < 5; i++) {
    sensorRaw[i] = analogRead(SENSOR_PINS[i]);
    int range = sensorMax[i] - sensorMin[i];
    int normalized = 0;

    if (range > 0) {
      normalized = (sensorRaw[i] - sensorMin[i]) * 1000L / range;
    }
    normalized = constrain(normalized, 0, 1000);

    // On most analog reflectance boards, black may read either higher or lower.
    // Calibration only establishes the range, so choose the stronger side by
    // comparing against the calibrated midpoint and LINE_IS_DARK.
    int midpoint = (sensorMin[i] + sensorMax[i]) / 2;
    bool rawIsHigh = sensorRaw[i] > midpoint;
    bool lineDetected = LINE_IS_DARK ? !rawIsHigh : rawIsHigh;
    sensorStrength[i] = lineDetected ? (1000 - normalized) : normalized;

    // Correct normalized polarity so line strength is high for the configured
    // line color. If your sensor board has opposite analog behavior, see README.
    if (!LINE_IS_DARK) sensorStrength[i] = 1000 - sensorStrength[i];
  }
}

// Returns true if a line was detected; position ranges roughly -2000..+2000.
bool calculateLinePosition(long &position) {
  const int weights[5] = {-2000, -1000, 0, 1000, 2000};
  long weightedTotal = 0;
  long strengthTotal = 0;

  for (int i = 0; i < 5; i++) {
    int strength = sensorStrength[i];
    if (strength >= LINE_DETECT_THRESHOLD) {
      weightedTotal += (long)strength * weights[i];
      strengthTotal += strength;
    }
  }

  if (strengthTotal == 0) return false;
  position = weightedTotal / strengthTotal;
  if (position < 0) lastSeenDirection = -1;
  if (position > 0) lastSeenDirection = 1;
  return true;
}

// -------------------- SETUP / MAIN LOOP --------------------
void setup() {
  Serial.begin(115200);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  for (int i = 0; i < 5; i++) pinMode(SENSOR_PINS[i], INPUT);

  stopMotors();
  delay(500);
  calibrateSensors();
  lastControlTime = micros();

  Serial.println(F("Line follower ready."));
}

void loop() {
  readSensors();

  long position = 0;
  bool lineFound = calculateLinePosition(position);

  if (!lineFound) {
    // Search in the direction where the line was last observed.
    int searchSpeed = 95;
    if (lastSeenDirection < 0) {
      setMotors(-searchSpeed, searchSpeed);
    } else {
      setMotors(searchSpeed, -searchSpeed);
    }
    delay(LOOP_DELAY_MS);
    return;
  }

  // Position is scaled from -2000 (far left) to +2000 (far right).
  float error = (float)position;
  unsigned long now = micros();
  float dt = (now - lastControlTime) / 1000000.0f;
  if (dt <= 0.0f || dt > 0.1f) dt = 0.005f;
  lastControlTime = now;

  integral += error * dt;
  integral = constrain(integral, -5000.0f, 5000.0f);
  float derivative = (error - lastError) / dt;
  float correction = KP * error + KI * integral + KD * derivative;
  lastError = error;

  int leftSpeed = (int)(BASE_SPEED + correction);
  int rightSpeed = (int)(BASE_SPEED - correction);

  leftSpeed = constrain(leftSpeed, MIN_SPEED, MAX_SPEED);
  rightSpeed = constrain(rightSpeed, MIN_SPEED, MAX_SPEED);

  setMotors(leftSpeed, rightSpeed);

  // Optional serial diagnostics for tuning.
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= 100) {
    lastPrint = millis();
    Serial.print(F("Position: "));
    Serial.print(position);
    Serial.print(F(" | L: "));
    Serial.print(leftSpeed);
    Serial.print(F(" | R: "));
    Serial.println(rightSpeed);
  }

  delay(LOOP_DELAY_MS);
}
