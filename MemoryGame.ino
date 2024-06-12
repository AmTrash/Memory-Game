#include <Adafruit_CircuitPlayground.h>

int Score = 0;
volatile bool gameState = false;
volatile bool intFlag = 0;

int sequence[20];
int userSequence[20];
int userColor[20]; // Array to store the colors of selected LEDs
int level = 1;
int userIndex = 0;
int currentLED = 0;
int currentColor = 0; // 0 = Green, 1 = Blue, 2 = Red

#define CLICKTHRESHHOLD 120

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

    while (userIndex < level && gameState) {
      handleButtonPresses();
      delay(100);
    }

    if (checkUserSequence(level)) {
      correctSequenceFeedback();
      delay(1000);
      level++;
    } else {
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
  CircuitPlayground.setPixelColor(led, 255, 255, 255);
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
    if (userSequence[currentLED] == -1) {
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
