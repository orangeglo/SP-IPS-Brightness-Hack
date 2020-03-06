#include <avr/sleep.h>
#include <EEPROM.h>

#define BUTTON_PIN 2 // pin 7 on the tiny85
#define BRIGHTNESS_CONTROL_PIN 3 // pin 2 on the tiny85
#define CHECKSUM_ADDRESS 0x05
#define CHECKSUM_VALUE 0x69
#define BRIGHTNESS_ADDRESS 0x00

#define INITIAL_BRIGHTNESS_STEP 3 // 0 indexed
#define TOTAL_BRIGHTNESS_LEVELS 6
#define STARTUP_DELAY 300

byte brightness = INITIAL_BRIGHTNESS_STEP;
byte syncBrightness = INITIAL_BRIGHTNESS_STEP;
byte checksum;

void setup() {
  pinMode(BUTTON_PIN, INPUT);
  pinMode(BRIGHTNESS_CONTROL_PIN, INPUT);
  
  // load brightness value or setup EEPROM if it feels like the first time
  EEPROM.get(CHECKSUM_ADDRESS, checksum);
  if (checksum == CHECKSUM_VALUE){
    EEPROM.get(BRIGHTNESS_ADDRESS, brightness);
  } else {
    EEPROM.write(CHECKSUM_ADDRESS, CHECKSUM_VALUE);
    EEPROM.write(BRIGHTNESS_ADDRESS, brightness);
  }

  // delay before ribbon will respond
  delay(STARTUP_DELAY);

  // sync brightness setting
  while (brightness != syncBrightness) {
    pulseBrightnessControl();
    syncBrightness = (syncBrightness + 1) % TOTAL_BRIGHTNESS_LEVELS;
  }
}

void loop() {
  // sleep and interrupt technique from https://forum.arduino.cc/index.php?topic=608825.0
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  cli();
  attachInterrupt(0, setBrightness, LOW); 
  sei();
  sleep_cpu();

  // wakes here
  sleep_disable();

  // update brightness
  brightness = (brightness + 1) % TOTAL_BRIGHTNESS_LEVELS;
  EEPROM.update(BRIGHTNESS_ADDRESS, brightness);
  pulseBrightnessControl();
}

void setBrightness() {
  detachInterrupt(0);
}

void pulseBrightnessControl() {
  pinMode(BRIGHTNESS_CONTROL_PIN, OUTPUT);
  digitalWrite(BRIGHTNESS_CONTROL_PIN, LOW);
  delay(200);
  pinMode(BRIGHTNESS_CONTROL_PIN, INPUT);
  delay(10);
}
