#include <Servo.h>
#include <NewPing.h>

// ================= CONFIG =================

// ---- Enable Features ----
#define ENABLE_PIR            1
#define ENABLE_LASER          1
#define ENABLE_DEBUG          1
#define ENABLE_CONFIRMATION   1

// ---- Pins ----
#define TRIG_PIN   12
#define ECHO_PIN   11
#define PAN_PIN    9
#define PIR_PIN    2
#define LASER_PIN  13

// ---- Servo ----
#define PAN_MIN    20
#define PAN_MAX    160
#define PAN_CENTER 90

// ---- Scan ----
#define SCAN_STEP     8
#define LOCAL_STEP    5
#define LOCAL_RANGE   60

// ---- Timing ----
#define SERVO_SETTLE 120   // ms

// ---- Distance ----
#define MAX_DISTANCE 200
#define TRACK_THRESHOLD 10

// ---- Confirmation ----
#define CONFIRM_COUNT 2

// ==========================================

Servo panServo;
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);

int lockedAngle = 90;
int lockedDist = 0;

#if ENABLE_PIR
volatile bool pirTriggered = false;
#endif

// ================= PIR ISR =================
#if ENABLE_PIR
void pirISR()
{
  pirTriggered = true;
}
#endif

// ================= MEASURE =================
int measure()
{
  delay(SERVO_SETTLE);

  int raw = sonar.ping_cm();

#if ENABLE_DEBUG
  Serial.print("RAW: ");
  Serial.print(raw);
#endif

  if (raw == 0)
  {
#if ENABLE_DEBUG
    Serial.println(" -> INVALID");
#endif
    return MAX_DISTANCE;
  }

#if ENABLE_DEBUG
  Serial.print(" -> VALID: ");
  Serial.println(raw);
#endif

  return raw;
}

// ================= MOVE =================
int moveAndMeasure(int angle)
{
  panServo.write(angle);
  return measure();
}

// ================= FULL SCAN =================
int scanWide()
{
  int bestAngle = PAN_CENTER;
  int bestDist = MAX_DISTANCE;

#if ENABLE_DEBUG
  Serial.println("=== SCAN START ===");
#endif

  for (int a = PAN_MIN; a <= PAN_MAX; a += SCAN_STEP)
  {
    int d = moveAndMeasure(a);

#if ENABLE_DEBUG
    Serial.print("ANGLE: ");
    Serial.print(a);
    Serial.print(" | D:");
    Serial.println(d);
#endif

    if (d < bestDist)
    {
      bestDist = d;
      bestAngle = a;
    }
  }

  lockedDist = bestDist;
  return bestAngle;
}

// ================= LOCAL SCAN =================
int scanLocal(int center)
{
  int start = max(PAN_MIN, center - LOCAL_RANGE);
  int end   = min(PAN_MAX, center + LOCAL_RANGE);

  int bestAngle = center;
  int bestDist = MAX_DISTANCE;

  for (int a = start; a <= end; a += LOCAL_STEP)
  {
    int d = moveAndMeasure(a);

    if (d < bestDist)
    {
      bestDist = d;
      bestAngle = a;
    }
  }

  lockedDist = bestDist;
  return bestAngle;
}

// ================= SETUP =================
void setup()
{
  Serial.begin(115200);

  panServo.attach(PAN_PIN);
  panServo.write(PAN_CENTER);

#if ENABLE_LASER
  pinMode(LASER_PIN, OUTPUT);
  digitalWrite(LASER_PIN, LOW);
#endif

#if ENABLE_PIR
  pinMode(PIR_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), pirISR, RISING);
#endif

  delay(1000);
}

// ================= LOOP =================
void loop()
{
  // -------- FULL SCAN --------
  lockedAngle = scanWide();

#if ENABLE_DEBUG
  Serial.print("LOCKED ANGLE: ");
  Serial.print(lockedAngle);
  Serial.print(" DIST: ");
  Serial.println(lockedDist);
#endif

  panServo.write(lockedAngle);

#if ENABLE_LASER
  digitalWrite(LASER_PIN, HIGH);
#endif

  int confirm = 0;

  // -------- TRACK LOOP --------
  while (true)
  {
    int d = measure();

#if ENABLE_DEBUG
    Serial.print("TRACK D: ");
    Serial.println(d);
#endif

    int diff = abs(d - lockedDist);

    // ---- PIR TRIGGER ----
#if ENABLE_PIR
    if (pirTriggered)
    {
      pirTriggered = false;

#if ENABLE_DEBUG
      Serial.println("PIR TRIGGER → RESCAN");
#endif

#if ENABLE_LASER
      digitalWrite(LASER_PIN, LOW);
#endif
      break; // go to full scan
    }
#endif

#if ENABLE_CONFIRMATION
    if (diff > TRACK_THRESHOLD)
    {
      confirm++;

      if (confirm >= CONFIRM_COUNT)
      {
        confirm = 0;

#if ENABLE_DEBUG
        Serial.println("LOCAL RESCAN");
#endif

#if ENABLE_LASER
        digitalWrite(LASER_PIN, LOW);
#endif

        lockedAngle = scanLocal(lockedAngle);
        panServo.write(lockedAngle);

#if ENABLE_LASER
        digitalWrite(LASER_PIN, HIGH);
#endif

#if ENABLE_DEBUG
        Serial.print("NEW LOCK: ");
        Serial.println(lockedAngle);
#endif
      }
    }
    else
    {
      confirm = 0;
    }
#else
    if (diff > TRACK_THRESHOLD)
    {
      lockedAngle = scanLocal(lockedAngle);
      panServo.write(lockedAngle);
    }
#endif
  }
}
