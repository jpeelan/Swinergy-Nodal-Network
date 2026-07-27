const int REED_PIN = 32;

// Reed switches may briefly bounce between open and closed.
// The state must remain unchanged for 20 ms before we accept it.
const unsigned long DEBOUNCE_TIME_MS = 20;

// Send one graph sample every 10 ms = 100 samples per second.
const unsigned long SEND_INTERVAL_MS = 10;

int rawState;
int stableState;

unsigned long rawStateChangedAt = 0;
unsigned long lastSendTime = 0;

//1 = magnet just detected
//-1 = magnet just removed
//0 = no new change
int pendingChange = 0;

void setup() {
  Serial.begin(115200);

  // External pull-up resistor is already installed.
  pinMode(REED_PIN, INPUT);

  rawState = digitalRead(REED_PIN);
  stableState = rawState;

  rawStateChangedAt = millis();

  Serial.println("Reed sensor graph ready");
}

void loop() {
  unsigned long currentTime = millis();
  int newRawState = digitalRead(REED_PIN);

  // Detect any immediate electrical change.
  if (newRawState != rawState) {
    rawState = newRawState;
    rawStateChangedAt = currentTime;
  }

  // Accept the change only after it remains stable.
  if ((currentTime - rawStateChangedAt >= DEBOUNCE_TIME_MS) &&
      (stableState != rawState)) {

    stableState = rawState;

    // With your pull-up circuit:
    // LOW means the switch connected IO32 to ground.
    bool magnetDetected = (stableState == LOW);

    if (magnetDetected) {
      pendingChange = 1;
    } else {
      pendingChange = -1;
    }
  }

  // Send data to Python at a constant rate.
  if (currentTime - lastSendTime >= SEND_INTERVAL_MS) {
    lastSendTime = currentTime;

    int magnetDetected = (stableState == LOW) ? 1 : 0;

    Serial.print(currentTime);
    Serial.print(",");
    Serial.print(magnetDetected);
    Serial.print(",");
    Serial.println(pendingChange);

    // The change tick is sent only once.
    pendingChange = 0;
  }
}