#include <Servo.h>
#include <Wire.h>
#include <MPU6050_light.h>

#define IR_SENSOR_LEFT   A2
#define IR_SENSOR_RIGHT  A3

#define TRIG_PIN A0
#define ECHO_PIN A1

#define MOTOR_SPEED 203
#define GYRO_CORRECT 3

#define HYBRID_SAFE_DISTANCE 25

#define T_MIN_DISTANCE 15
#define T_MAX_DISTANCE 50

int enableLeftMotor  = 5;
int enableRightMotor = 6;

int leftMotorPin1  = 8;
int leftMotorPin2  = 9;
int rightMotorPin1 = 10;
int rightMotorPin2 = 11;

#define CLAW_PIN        4
#define SCAN_SERVO_PIN  3
#define BUZZER          12

Servo claw;
Servo scanServo;

MPU6050 mpu(Wire);

float targetYaw = 0;

char mode = 'M';

bool hybridAvoiding = false;

unsigned long tLastScan = 0;
int tDistance = 999;

struct Step {
  char cmd;
  unsigned long dur;
};

Step path[80];

int steps = 0;
bool recording = false;
bool playing = false;

int playIndex = 0;
unsigned long stepStart = 0;
unsigned long playStart = 0;

char lastCmd = 'S';

unsigned long lastScan = 0;
int frontDistance = 100;

void setup()
{
  pinMode(enableLeftMotor, OUTPUT);
  pinMode(enableRightMotor, OUTPUT);

  pinMode(leftMotorPin1, OUTPUT);
  pinMode(leftMotorPin2, OUTPUT);
  pinMode(rightMotorPin1, OUTPUT);
  pinMode(rightMotorPin2, OUTPUT);

  pinMode(IR_SENSOR_LEFT, INPUT);
  pinMode(IR_SENSOR_RIGHT, INPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(BUZZER, OUTPUT);

  claw.attach(CLAW_PIN);
  claw.write(90);

  scanServo.attach(SCAN_SERVO_PIN);
  scanServo.write(90);

  Serial.begin(9600);

  Wire.begin();
  delay(1000);

  mpu.begin();
  delay(1000);

  mpu.calcOffsets(true, true);

  stopMotors();
}

void loop()
{
  mpu.update();

  if (Serial.available() && !playing)
  {
    handleCommand(Serial.read());
  }

  if (playing)
  {
    playRoutine();
  }
  else if (mode == 'X')
  {
    lineFollower();
  }
  else if (mode == 'O')
  {
    obstacleAvoidMode();
  }
  else if (mode == 'H')
  {
    hybridMode();
  }
  else if (mode == 'T')
  {
    triggerMode();
  }
}

void handleCommand(char c)
{
  if (c == 'M' || c == 'X' || c == 'Y' || c == 'O' || c == 'H' || c == 'T')
  {
    mode = c;
    beep();
    stopMotors();
    targetYaw = mpu.getAngleZ();
    hybridAvoiding = false;
    return;
  }

  if (c == 'Z')
  {
    recording = true;
    steps = 0;
    stepStart = millis();
    lastCmd = 'S';
    beep();
    return;
  }

  if (c == 'E')
  {
    if (recording)
    {
      saveStep();
    }
    recording = false;
    beep();
    return;
  }

  if (c == 'P')
  {
    if (recording)
    {
      saveStep();
    }

    recording = false;
    playing = true;
    playIndex = 0;
    playStart = millis();
    beep();

    execute(path[0].cmd);
    return;
  }

  if (recording && c != lastCmd)
  {
    saveStep();
    lastCmd = c;
  }

  execute(c);
}

void triggerMode()
{
  if (millis() - tLastScan > 80)
  {
    tDistance = getDistance();
    tLastScan = millis();
  }

  if (tDistance > T_MIN_DISTANCE && tDistance < T_MAX_DISTANCE)
  {
    rotateMotor(MOTOR_SPEED, MOTOR_SPEED);
  }
  else if (tDistance <= T_MIN_DISTANCE && tDistance > 0)
  {
    rotateMotor(-MOTOR_SPEED, -MOTOR_SPEED);
  }
  else
  {
    stopMotors();
  }
}

void hybridMode()
{
  if (millis() - lastScan > 80)
  {
    frontDistance = getDistance();
    lastScan = millis();
  }

  if (frontDistance < HYBRID_SAFE_DISTANCE && frontDistance > 0)
  {
    hybridAvoiding = true;

    stopMotors();
    delay(40);

    rotateMotor(-MOTOR_SPEED, -MOTOR_SPEED);
    delay(180);

    rotateMotor(MOTOR_SPEED, -MOTOR_SPEED);
    delay(200);

    stopMotors();
    hybridAvoiding = false;
  }
}

void lineFollower()
{
  int L = digitalRead(IR_SENSOR_LEFT);
  int R = digitalRead(IR_SENSOR_RIGHT);

  if (R == LOW && L == LOW)
  {
    rotateMotor(MOTOR_SPEED, MOTOR_SPEED);
  }
  else if (R == HIGH && L == LOW)
  {
    rotateMotor(MOTOR_SPEED, -MOTOR_SPEED);
  }
  else if (R == LOW && L == HIGH)
  {
    rotateMotor(-MOTOR_SPEED, MOTOR_SPEED);
  }
  else
  {
    stopMotors();
  }
}

void obstacleAvoidMode()
{
  if (millis() - lastScan > 80)
  {
    frontDistance = getDistance();
    lastScan = millis();
  }

  if (frontDistance < 30 && frontDistance > 0)
  {
    stopMotors();
    delay(50);

    rotateMotor(-MOTOR_SPEED, -MOTOR_SPEED);
    delay(200);

    stopMotors();
    delay(50);

    scanServo.write(10);
    delay(300);
    int leftDist = getDistance();

    scanServo.write(170);
    delay(350);
    int rightDist = getDistance();

    scanServo.write(90);
    delay(150);

    if (rightDist > leftDist)
    {
      rotateMotor(MOTOR_SPEED, -MOTOR_SPEED);
    }
    else
    {
      rotateMotor(-MOTOR_SPEED, MOTOR_SPEED);
    }

    delay(220);
    stopMotors();
  }
  else
  {
    rotateMotor(MOTOR_SPEED, MOTOR_SPEED);
  }
}

int getDistance()
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 25000);

  if (duration == 0)
  {
    return 999;
  }

  int d = duration * 0.034 / 2;

  if (d > 200)
  {
    return 999;
  }

  return d;
}

void execute(char c)
{
  if (mode == 'X' || mode == 'O') return;
  if (mode == 'H' && hybridAvoiding) return;
  if (mode == 'T') return;

  if (mode == 'Y')
  {
    if (c == 'F')
    {
      gyroForward();
    }
    else if (c == 'L')
    {
      rotateMotor(-MOTOR_SPEED, MOTOR_SPEED);
      targetYaw = mpu.getAngleZ();
    }
    else if (c == 'R')
    {
      rotateMotor(MOTOR_SPEED, -MOTOR_SPEED);
      targetYaw = mpu.getAngleZ();
    }
    else if (c == 'B')
    {
      rotateMotor(-MOTOR_SPEED, -MOTOR_SPEED);
    }
    else
    {
      stopMotors();
    }
    return;
  }

  if (c == 'F')
  {
    rotateMotor(MOTOR_SPEED, MOTOR_SPEED);
  }
  else if (c == 'B')
  {
    rotateMotor(-MOTOR_SPEED, -MOTOR_SPEED);
  }
  else if (c == 'L')
  {
    rotateMotor(-MOTOR_SPEED, MOTOR_SPEED);
  }
  else if (c == 'R')
  {
    rotateMotor(MOTOR_SPEED, -MOTOR_SPEED);
  }
  else if (c == 'G')
  {
    claw.write(90);
  }
  else if (c == 'g')
  {
    claw.write(145);
  }
  else
  {
    stopMotors();
  }
}

void gyroForward()
{
  float error = mpu.getAngleZ() - targetYaw;

  int correction = constrain(error * GYRO_CORRECT, -30, 30);

  rotateMotor(
    MOTOR_SPEED - correction,
    MOTOR_SPEED + correction
  );
}

void saveStep()
{
  if (steps >= 80) return;

  path[steps++] = {
    lastCmd,
    millis() - stepStart
  };

  stepStart = millis();
}

void playRoutine()
{
  if (playIndex >= steps)
  {
    playing = false;
    stopMotors();
    return;
  }

  if (millis() - playStart >= path[playIndex].dur)
  {
    playIndex++;
    playStart = millis();

    if (playIndex < steps)
    {
      execute(path[playIndex].cmd);
    }
  }
}

void rotateMotor(int l, int r)
{
  digitalWrite(leftMotorPin1,  l > 0);
  digitalWrite(leftMotorPin2,  l < 0);

  digitalWrite(rightMotorPin1, r > 0);
  digitalWrite(rightMotorPin2, r < 0);

  analogWrite(enableLeftMotor,  abs(l));
  analogWrite(enableRightMotor, abs(r));
}

void stopMotors()
{
  rotateMotor(0, 0);
}

void beep()
{
  digitalWrite(BUZZER, HIGH);
  delay(10);
  digitalWrite(BUZZER, LOW);
}
