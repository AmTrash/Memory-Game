#include <Adafruit_CircuitPlayground.h>
#include <AsyncDelay.h>
#include <Wire.h>
#include <SPI.h>

int Score = 0;
volatile bool gameState = false;
volatile bool intFlag = false;

int sequence[20];
int userSequence[20];
int userColor[20]; // Array to store the colors of selected LEDs
int level = 1;
int userIndex = 0;
int currentLED = 0;
int currentColor = 0; // 0 = Green, 1 = Blue, 2 = Red

#define CLICKTHRESHHOLD 120

AsyncDelay roundDelay;
AsyncDelay speechDelay;

const uint8_t spONE[]       PROGMEM = {0xCC,0x67,0x75,0x42,0x59,0x5D,0x3A,0x4F,0x9D,0x36,0x63,0xB7,0x59,0xDC,0x30,0x5B,0x5C,0x23,0x61,0xF3,0xE2,0x1C,0xF1,0xF0,0x98,0xC3,0x4B,0x7D,0x39,0xCA,0x1D,0x2C,0x2F,0xB7,0x15,0xEF,0x70,0x79,0xBC,0xD2,0x46,0x7C,0x52,0xE5,0xF1,0x4A,0x6A,0xB3,0x71,0x47,0xC3,0x2D,0x39,0x34,0x4B,0x23,0x35,0xB7,0x7A,0x55,0x33,0x8F,0x59,0xDC,0xA2,0x44,0xB5,0xBC,0x66,0x72,0x8B,0x64,0xF5,0xF6,0x98,0xC1,0x4D,0x42,0xD4,0x27,0x62,0x38,0x2F,0x4A,0xB6,0x9C,0x88,0x68,0xBC,0xA6,0x95,0xF8,0x5C,0xA1,0x09,0x86,0x77,0x91,0x11,0x5B,0xFF,0x0F};
const uint8_t spTWO[]       PROGMEM = {0x0E,0x38,0x6E,0x25,0x00,0xA3,0x0D,0x3A,0xA0,0x37,0xC5,0xA0,0x05,0x9E,0x56,0x35,0x86,0xAA,0x5E,0x8C,0xA4,0x82,0xB2,0xD7,0x74,0x31,0x22,0x69,0xAD,0x1C,0xD3,0xC1,0xD0,0xFA,0x28,0x2B,0x2D,0x47,0xC3,0x1B,0xC2,0xC4,0xAE,0xC6,0xCD,0x9C,0x48,0x53,0x9A,0xFF,0x0F};
const uint8_t spTHREE[]     PROGMEM = {0x02,0xD8,0x2E,0x9C,0x01,0xDB,0xA6,0x33,0x60,0xFB,0x30,0x01,0xEC,0x20,0x12,0x8C,0xE4,0xD8,0xCA,0x32,0x96,0x73,0x63,0x41,0x39,0x89,0x98,0xC1,0x4D,0x0D,0xED,0xB0,0x2A,0x05,0x37,0x0F,0xB4,0xA5,0xAE,0x5C,0xDC,0x36,0xD0,0x83,0x2F,0x4A,0x71,0x7B,0x03,0xF7,0x38,0x59,0xCD,0xED,0x1E,0xB4,0x6B,0x14,0x35,0xB7,0x6B,0x94,0x99,0x91,0xD5,0xDC,0x26,0x48,0x77,0x4B,0x66,0x71,0x1B,0x21,0xDB,0x2D,0x8A,0xC9,0x6D,0x88,0xFC,0x26,0x28,0x3A,0xB7,0x21,0xF4,0x1F,0xA3,0x65,0xBC,0x02,0x38,0xBB,0x3D,0x8E,0xF0,0x2B,0xE2,0x08,0xB7,0x34,0xFF,0x0F};

void setup() {
  Serial.begin(9600);
  CircuitPlayground.begin();
  
  attachInterrupt(digitalPinToInterrupt(7), switchh, CHANGE);
  
  CircuitPlayground.setAccelRange(LIS3DH_RANGE_4_G);
  CircuitPlayground.setAccelTap(1, CLICKTHRESHHOLD);
  attachInterrupt(digitalPinToInterrupt(CPLAY_LIS3DH_INTERRUPT), tapTime, FALLING);

  randomSeed(analogRead(A3));

  setAllLEDs(255, 255, 255);
}

void loop() {
  if (intFlag) {
    delay(20);
    intFlag = 0;
  }
  
  if (gameState) {
    gameStart();
  } else {
    idleGame();
  }

  handleButtonPresses();

  delay(100);
}

void idleGame() {
  setAllLEDs(255, 255, 255);
}

void gameStart() {
  Serial.println("Game Start State");

  setAllLEDs(255, 255, 0);
  CircuitPlayground.playTone(440, 500);
  if (!gameState) return;
  delay(300);

  setAllLEDs(255, 165, 0);
  CircuitPlayground.playTone(523, 600);
  if (!gameState) return;
  delay(300);

  setAllLEDs(0, 255, 0);
  CircuitPlayground.playTone(587, 250);
  if (!gameState) return;
  delay(200);

  while (gameState) {
    CircuitPlayground.clearPixels();
    generateSequence(level);
    displaySequence(level);

    userIndex = 0;
    currentLED = 0;
    currentColor = 0;
    highlightCurrentLED(currentLED);

    roundDelay.start(10000, AsyncDelay::MILLIS); // Start 10-second timer
    speechDelay.start(1000, AsyncDelay::MILLIS); // Start speech timer

    while (userIndex < level && gameState) {
      handleButtonPresses();
      delay(100);

      if (roundDelay.isExpired()) {
        incorrectSequenceFeedback();
        gameState = false;
        break;
      }
      // Delete here if you wanna get rid of this stupid speech thing that messes with the audio
      unsigned long remainingTime = roundDelay.getDelay() - roundDelay.getDuration();
      int timerSecond = remainingTime / 1000;

      if (speechDelay.isExpired()) {
        speechDelay.restart();
        if (timerSecond == 13) {
          CircuitPlayground.speaker.say(spTHREE);
        } else if (timerSecond == 12) {
          CircuitPlayground.speaker.say(spTWO);
        } else if (timerSecond == 11) {
          CircuitPlayground.speaker.say(spONE);
        }
      }
    }

    if (gameState && checkUserSequence(level)) {
      correctSequenceFeedback();
      delay(1000);
      level++;
      roundDelay.start(10000, AsyncDelay::MILLIS); // Reset timer to 10 seconds for the next level
    } else if (gameState) {
      incorrectSequenceFeedback();
      break;
    }
  }
  gameState = false;
}

void generateSequence(int level) {
  for (int i = 0; i < level; i++) {
    sequence[i] = (random(0, 10) * 10) + random(0, 3);
  }
}

void displaySequence(int level) {
  for (int i = 0; i < level; i++) {
    int led = sequence[i] / 10;
    int color = sequence[i] % 10;
    setLEDColor(led, color);
    CircuitPlayground.playTone(440 + 100 * i, 500);
    delay(500);
    CircuitPlayground.setPixelColor(led, 0, 0, 0);
    delay(250);
  }
}

bool checkUserSequence(int level) {
  for (int i = 0; i < level; i++) {
    if (userSequence[i] != sequence[i]) {
      return false;
    }
  }
  return true;
}

void setAllLEDs(int red, int green, int blue) {
  for (int i = 0; i < 10; i++) {
    CircuitPlayground.setPixelColor(i, red, green, blue);
  }
}

void setLEDColor(int index, int color) {
  switch (color) {
    case 0:
      CircuitPlayground.setPixelColor(index, 0, 255, 0);
      break;
    case 1:
      CircuitPlayground.setPixelColor(index, 0, 0, 255);
      break;
    case 2:
      CircuitPlayground.setPixelColor(index, 255, 0, 0);
      break;
  }
}

void highlightCurrentLED(int led) {
  if (userColor[led] == -1) {
    CircuitPlayground.setPixelColor(led, 255, 255, 255);
  } else {
    setLEDColor(led, userColor[led]);
  }
}

void switchh() {
  intFlag = 1;
  gameState = !gameState;
  userIndex = 0;
  level = 1;
  clearUserSequence();
  setAllLEDs(255, 255, 255);
}

void clearUserSequence() {
  for (int i = 0; i < 20; i++) {
    userSequence[i] = -1;
    userColor[i] = -1;
  }
}

void leftButtonPress() {
  if (gameState) {
    if (userColor[currentLED] == -1) {
      CircuitPlayground.setPixelColor(currentLED, 0, 0, 0);
    } else {
      setLEDColor(currentLED, userColor[currentLED]);
    }
    currentLED = (currentLED + 1) % 10;
    highlightCurrentLED(currentLED);
  }
}

void rightButtonPress() {
  if (gameState) {
    selectCurrentPixel();
  }
}

void tapTime() {
  currentColor = (currentColor + 1) % 3;
  setLEDColor(currentLED, currentColor);
}

void selectCurrentPixel() {
  CircuitPlayground.speaker.end();
  userSequence[userIndex] = currentLED * 10 + currentColor;
  userColor[currentLED] = currentColor;
  Serial.print("Selected Pixel: ");
  Serial.println(currentLED);

  setLEDColor(currentLED, currentColor);

  userIndex++;
  if (userIndex < level) {
    currentLED = 0;
    highlightCurrentLED(currentLED);
  }

  if (level >= 2) {
    CircuitPlayground.playTone(500, 100);
    delay(100);
    CircuitPlayground.playTone(500, 100);
  }
}

void correctSequenceFeedback() {
  for (int i = 0; i < 2; i++) {
    setAllLEDs(0, 255, 0);
    CircuitPlayground.playTone(523, 200);
    delay(200);
    setAllLEDs(0, 0, 0);
    delay(200);
  }
  CircuitPlayground.clearPixels();
  clearUserSequence();
}

void incorrectSequenceFeedback() {
  CircuitPlayground.speaker.end();
  setAllLEDs(255, 0, 0);
  CircuitPlayground.playTone(100, 1000);
  Serial.println("LOSER HAHA");
  delay(1000);
  CircuitPlayground.clearPixels();
  Score = 0;
  level = 1;
  clearUserSequence();
}

void handleButtonPresses() {
  if (CircuitPlayground.leftButton()) {
    leftButtonPress();
    while (CircuitPlayground.leftButton());
  }

  if (CircuitPlayground.rightButton()) {
    rightButtonPress();
    while (CircuitPlayground.rightButton());
  }
}
