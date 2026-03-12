// Flip Timer: Ball Switch + Touch Sensor + RGB LED

// Pin definitions
const int BALL_SWITCH_PIN = 2;
const int TOUCH_SENSOR_PIN = 3;
const int RED_PIN = 9;
const int GREEN_PIN = 10;
const int BLUE_PIN = 11;

// Timer durations
const unsigned long TIMER_10SEC = 10 * 1000;
const unsigned long TIMER_20SEC= 20 * 1000;
const unsigned long TIMER_30SEC = 30 * 1000;

// Timer modes
enum TimerMode {
  MODE_10SEC = 0,
  MODE_20SEC = 1,
  MODE_30SEC = 2
};

// State variables
TimerMode currentMode = MODE_10SEC;
unsigned long timerDuration = TIMER_10SEC;
unsigned long timerStartTime = 0;
unsigned long pausedTime = 0;
bool timerRunning = false;
bool timerPaused = false;

// Ball switch state tracking
bool lastBallState = HIGH;
bool ballStable = false;
unsigned long ballStableTime = 0;
const unsigned long STABLE_DURATION = 5000;

// Touch sensor state tracking
bool lastTouchState = LOW;
unsigned long touchPressTime = 0;
bool touchHandled = false;
const unsigned long LONG_PRESS_DURATION = 5000;

void setup() {
  pinMode(BALL_SWITCH_PIN, INPUT_PULLUP);
  pinMode(TOUCH_SENSOR_PIN, INPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  
  Serial.begin(9600);
  Serial.println("=== Flip Timer Ready! ===");
  Serial.println("Flip to start timer");
  Serial.println("Touch sensor: tap=pause, hold=reset");
  Serial.println("Modes: Green=10sec, Blue=20sec, Red=30sec");
  
  // Show current mode (green)
  showMode(currentMode);
  delay(1000);
  setColor(0, 0, 0);
}

void loop() {
  handleBallSwitch();
  handleTouchSensor();
  updateTimer();
}

// BALL SWITCH HANDLING
void handleBallSwitch() {
  int ballReading = digitalRead(BALL_SWITCH_PIN);
  
  if (ballReading != lastBallState) {
    ballStable = false;
    ballStableTime = millis();
    lastBallState = ballReading;
    return;
  }
  
  if (!ballStable && (millis() - ballStableTime > STABLE_DURATION)) {
    ballStable = true;
    
    if (!timerRunning && !timerPaused) {
      startTimer();
    }
  }
}

// TOUCH SENSOR HANDLING
void handleTouchSensor() {
  int touchReading = digitalRead(TOUCH_SENSOR_PIN);
  
  // Touch just pressed
  if (touchReading == HIGH && lastTouchState == LOW) {
    touchPressTime = millis();
    touchHandled = false;
  }
  
  // Check for long press while still held (RESET)
  if (touchReading == HIGH && !touchHandled) {
    if (millis() - touchPressTime >= LONG_PRESS_DURATION) {
      resetTimer();
      touchHandled = true;  // Mark as handled so release doesn't trigger
    }
  }
  
  // Touch released
  if (touchReading == LOW && lastTouchState == HIGH) {
    unsigned long pressDuration = millis() - touchPressTime;
    
    // Only process short press if it wasn't already handled as long press
    if (!touchHandled && pressDuration < LONG_PRESS_DURATION) {
      if (timerRunning) {
        pauseTimer();
      } else if (timerPaused) {
        resumeTimer();
      } else {
        cycleMode();
      }
    }
  }
  
  lastTouchState = touchReading;
}

// TIMER FUNCTIONS
void startTimer() {
  Serial.print("Timer started: ");
  Serial.print(timerDuration / 1000);
  Serial.println(" seconds");
  
  timerStartTime = millis();
  timerRunning = true;
  timerPaused = false;
  ballStable = false;
  
  showMode(currentMode);
}

void pauseTimer() {
  Serial.println("Timer paused");
  timerPaused = true;
  timerRunning = false;
  pausedTime = millis() - timerStartTime;
  setColor(255, 255, 0);
}

void resumeTimer() {
  Serial.println("Timer resumed");
  timerRunning = true;
  timerPaused = false;
  timerStartTime = millis() - pausedTime;
  showMode(currentMode);
}

void resetTimer() {
  Serial.println("Timer reset!");
  timerRunning = false;
  timerPaused = false;
  
  // Flash white briefly
  setColor(255, 255, 255);
  delay(300);
  setColor(0, 0, 0);
  delay(200);
  
  // Show current mode briefly after reset
  showMode(currentMode);
  delay(500);
  setColor(0, 0, 0);
}

void cycleMode() {
  currentMode = (TimerMode)((currentMode + 1) % 3);
  
  switch(currentMode) {
    case MODE_10SEC:
      timerDuration = TIMER_10SEC;
      Serial.println("Mode: 10 seconds (Green)");
      break;
    case MODE_20SEC:
      timerDuration = TIMER_20SEC;
      Serial.println("Mode: 20 seconds (Blue)");
      break;
    case MODE_30SEC:
      timerDuration = TIMER_30SEC;
      Serial.println("Mode: 30 seconds (Red)");
      break;
  }
  
  showMode(currentMode);
  delay(500);
  setColor(0, 0, 0);
}

void updateTimer() {
  if (!timerRunning) return;
  
  unsigned long elapsed = millis() - timerStartTime;
  
  if (elapsed >= timerDuration) {
    timerComplete();
    return;
  }
  
  static unsigned long lastPrint = 0;
  static int lastSecond = 0;
  if (millis() - lastPrint >= 1000) {
    int currentSecond = elapsed / 1000;
    if (currentSecond != lastSecond) {
      Serial.print("Mode: ");
      Serial.print(timerDuration / 1000);
      Serial.print(" seconds (");
      
      switch(currentMode) {
        case MODE_10SEC:
          Serial.print("Green");
          break;
        case MODE_20SEC:
          Serial.print("Blue");
          break;
        case MODE_30SEC:
          Serial.print("Red");
          break;
      }
      
      Serial.print(") → ");
      Serial.print(currentSecond);
      Serial.println(" second");
      
      lastSecond = currentSecond;
    }
    lastPrint = millis();
  }
  
  float progress = (float)elapsed / timerDuration;
  
  if (progress > 0.9) {
    blinkCurrentColor(200);
  } else if (progress > 0.5) {
    pulseCurrentColor();
  } else {
    showMode(currentMode);
  }
}

void timerComplete() {
  Serial.println("=== TIMER COMPLETE! ===");
  
  for (int cycle = 0; cycle < 3; cycle++) {
    setColor(255, 0, 0);   delay(150);
    setColor(255, 127, 0); delay(150);
    setColor(255, 255, 0); delay(150);
    setColor(0, 255, 0);   delay(150);
    setColor(0, 0, 255);   delay(150);
    setColor(75, 0, 130);  delay(150);
    setColor(148, 0, 211); delay(150);
  }
  
  for (int i = 0; i < 5; i++) {
    setColor(255, 255, 255);
    delay(200);
    setColor(0, 0, 0);
    delay(200);
  }
  
  timerRunning = false;
  setColor(0, 0, 0);
}

// LED CONTROL FUNCTIONS
void setColor(int red, int green, int blue) {
  analogWrite(RED_PIN, red);
  analogWrite(GREEN_PIN, green);
  analogWrite(BLUE_PIN, blue);
}

void showMode(TimerMode mode) {
  switch(mode) {
    case MODE_10SEC:
      setColor(0, 255, 0);
      break;
    case MODE_20SEC:
      setColor(0, 0, 255);
      break;
    case MODE_30SEC:
      setColor(255, 0, 0);
      break;
  }
}

void blinkCurrentColor(int delayTime) {
  static unsigned long lastBlink = 0;
  static bool blinkState = false;
  
  if (millis() - lastBlink > delayTime) {
    blinkState = !blinkState;
    if (blinkState) {
      showMode(currentMode);
    } else {
      setColor(0, 0, 0);
    }
    lastBlink = millis();
  }
}

void pulseCurrentColor() {
  static unsigned long lastUpdate = 0;
  static int brightness = 255;
  static int fadeAmount = -5;
  
  if (millis() - lastUpdate > 30) {
    brightness += fadeAmount;
    
    if (brightness <= 50 || brightness >= 255) {
      fadeAmount = -fadeAmount;
    }
    
    switch(currentMode) {
      case MODE_10SEC:
        setColor(0, brightness, 0);
        break;
      case MODE_20SEC:
        setColor(0, 0, brightness);
        break;
      case MODE_30SEC:
        setColor(brightness, 0, 0);
        break;
    }
    
    lastUpdate = millis();
  }
}