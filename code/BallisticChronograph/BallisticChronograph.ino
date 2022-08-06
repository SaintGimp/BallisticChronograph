#include <splash.h>
#include <Adafruit_SH110X.h>

// OLED FeatherWing buttons map to different pins depending on board:
#if defined(ESP8266)
  #define BUTTON_A  0
  #define BUTTON_B 16
  #define BUTTON_C  2
#elif defined(ESP32) && !defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S2)
  #define BUTTON_A 15
  #define BUTTON_B 32
  #define BUTTON_C 14
#elif defined(ARDUINO_STM32_FEATHER)
  #define BUTTON_A PA15
  #define BUTTON_B PC7
  #define BUTTON_C PC5
#elif defined(TEENSYDUINO)
  #define BUTTON_A  4
  #define BUTTON_B  3
  #define BUTTON_C  8
#elif defined(ARDUINO_NRF52832_FEATHER)
  #define BUTTON_A 31
  #define BUTTON_B 30
  #define BUTTON_C 27
#else // 32u4, M0, M4, nrf52840, esp32-s2 and 328p
  #define BUTTON_A  9
  #define BUTTON_B  6
  #define BUTTON_C  5
#endif

const int PULSE1_PIN = 9;
const int PULSE2_PIN = 11;

const int millimetersBetweenSensors = 426;

volatile long time1 = 0;
volatile long time2 = 0;

int numberOfEvents = 0;
int unfinishedEvents = 0;
long intervalInMicros = 0;
float speedInMetersPerSecond = 0;

Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PULSE1_PIN, INPUT);
  pinMode(PULSE2_PIN, INPUT);
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(PULSE1_PIN), pulse1_falling, FALLING);
  attachInterrupt(digitalPinToInterrupt(PULSE2_PIN), pulse2_falling, FALLING);

  display.begin(0x3C, true);
  display.display();
  delay(1000);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);
  display.setRotation(1);

  display.println("Ready...");
  display.display();
}

void loop() {  
  if (!digitalRead(BUTTON_C)) {
    numberOfEvents = 0;
    unfinishedEvents = 0;

    display.clearDisplay();
    display.setCursor(0,0);
    display.println("Ready...");
    display.display();
  }

  if (time1 > 0 && time2 == 0 || time1 == 0 && time2 > 0)
  {
    digitalWrite(LED_BUILTIN, HIGH);

    // Reset after some time if only one sensor triggered
    long currentTime = micros();
    if ((time1 > 0 && currentTime - time1 > 2000000) || (time2 > 0 && currentTime - time2 > 2000000))
    {
      time1 = 0;
      time2 = 0;
      unfinishedEvents++;
      displayData(intervalInMicros, speedInMetersPerSecond);
      digitalWrite(LED_BUILTIN, LOW);
    }
  }
  
  if (time1 > 0 && time2 > 0)
  {
    numberOfEvents++;
    
    intervalInMicros = abs(time1 - time2);
    speedInMetersPerSecond = (millimetersBetweenSensors / 1000.0f) / (intervalInMicros / 1000000.0f);
    displayData(intervalInMicros, speedInMetersPerSecond);

    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    time1 = 0;
    time2 = 0;
  }
}

void displayData(long intervalInMicros, float speedInMetersPerSecond) {
    display.clearDisplay();
    display.setCursor(0,0);
    display.print(intervalInMicros);
    display.println(" us");
    display.print(speedInMetersPerSecond);
    display.println(" m/s");
    display.print(numberOfEvents);
    display.println(" events");
    display.print(unfinishedEvents);
    display.println(" unfinished");
    display.display();

    Serial.print(speedInMetersPerSecond);
    Serial.println(" m/s");  
}

void pulse1_falling() {
  if (time1 == 0) {
    time1 = micros();
  }
}

void pulse2_falling() {
  if (time2 == 0) {
    time2 = micros();
  }
}
