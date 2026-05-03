#include <Servo.h>
#include <NewPing.h>

#define TRIG_PIN 12
#define ECHO_PIN 11
#define MAX_DISTANCE 200

#define PAN_PIN 9
#define TILT_PIN 10

#define SERVO_SETTLE 10
#define PAN_MIN 25
#define PAN_MAX 150

#define SCAN_STEP 1
#define LOCAL_RANGE 60
#define TRACK_THRESHOLD 10

Servo panServo;
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);

int pan = 90;
int lockedDist = 0;
int lockedAngle = 90;

// single clean measurement
int measure()
{
  delay(SERVO_SETTLE);

  int d = sonar.ping_cm();

  if (d == 0) return MAX_DISTANCE;

  return d;
}

// move + stabilize
int moveAndMeasure(int angle)
{
  panServo.write(angle);
  return measure();
}

// full scan
int scanWide()
{
  int bestAngle = 90;
  int bestDist = MAX_DISTANCE;

  for (int a = PAN_MIN; a <= PAN_MAX; a += SCAN_STEP)
  {
    int d = moveAndMeasure(a);

    Serial.print("SCAN A:");
    Serial.print(a);
    Serial.print(" D:");
    Serial.println(d);

    if (d < bestDist)
    {
      bestDist = d;
      bestAngle = a;
    }
  }

  lockedDist = bestDist;
  return bestAngle;
}

// local scan around last known target
int scanLocal(int center)
{
  int start = max(PAN_MIN, center - LOCAL_RANGE);
  int end   = min(PAN_MAX, center + LOCAL_RANGE);

  int bestAngle = center;
  int bestDist = MAX_DISTANCE;

  for (int a = start; a <= end; a += 5)
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

void setup()
{
  Serial.begin(115200);
  panServo.attach(PAN_PIN);
  pinMode(13,OUTPUT);
  panServo.write(90);
  delay(5000);
}

void loop()
{
  // -------- INITIAL SCAN --------
  lockedAngle = scanWide();

  Serial.print("LOCKED AT: ");
  Serial.print(lockedAngle);
  Serial.print(" DIST: ");
  Serial.println(lockedDist);

  panServo.write(lockedAngle);

 
  while (true)
{
  int d = measure();

  Serial.print("TRACK D:");
  Serial.println(d);

  // -------- IGNORE INVALID --------
  if (d == 0 || d == MAX_DISTANCE)
  {
    Serial.println("IGNORED (INVALID)");
    continue;
  }

  int diff = abs(d - lockedDist);

  // -------- ONLY REAL CHANGE --------
  if (diff > TRACK_THRESHOLD)
  {
    Serial.println("LOCAL RESCAN");
    lockedAngle = scanLocal(lockedAngle);
    panServo.write(lockedAngle);

    Serial.print("NEW LOCK: ");
    Serial.println(lockedAngle);
  }
    digitalWrite(13,HIGH);
    delay(50);
     digitalWrite(13,LOW);
      delay(50);
}
}
