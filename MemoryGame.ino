#include <Adafruit_CircuitPlayground.h>

int Score = 0;
volatile bool gameState = false;
volatile bool intFlag = 0;

int sequence[20]; 
int userSequence[20]; 
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

  // Initialize random number generator
  randomSeed(analogRead(A0));

  // Light up all LEDs in white initially
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
  // Ensure all LEDs are white during idle
  setAllLEDs(255, 255, 255);
}

void gameStart() {
  Serial.println("Game Start State");

  // Ready, Set, Go sequence
  setAllLEDs(255, 255, 0); // Yellow for Ready
  CircuitPlayground.playTone(440, 500);
  if (!gameState) return; 
  delay(300);

  setAllLEDs(255, 165, 0); // Orange for Set
  CircuitPlayground.playTone(523, 600);
  if (!gameState) return; 
  delay(300);

  setAllLEDs(0, 255, 0); // Green for Go
  CircuitPlayground.playTone(587, 250);
  if (!gameState) return; 
  delay(200);

  while (gameState) {
    CircuitPlayground.clearPixels();
    generateSequence(level);
    displaySequence(level);

    // Initialize user interaction
    userIndex = 0;
    currentLED = 0;
    currentColor = 0;
    highlightCurrentLED(currentLED); 

    // Wait for user input
    while (userIndex < level && gameState) {
      handleButtonPresses();
      delay(100);
    }

    // Check if the user's input is correct
    if (checkUserSequence(level)) {
      // Correct sequence
      correctSequenceFeedback();
      delay(1000); 
      level++; 
    } else {
    
      incorrectSequenceFeedback();
      break;
    }
  }

  gameState = false; // Reset game state to idle
}

void generateSequence(int level) {
  for (int i = 0; i < level; i++) {
    sequence[i] = (random(0, 10) * 10) + random(0, 3); // Combine LED position and color
  }
}

void displaySequence(int level) {
  for (int i = 0; i < level; i++) {
    int led = sequence[i] / 10; // Extract LED position
    int color = sequence[i] % 10; // Extract color
    setLEDColor(led, color);
    CircuitPlayground.playTone(440 + 100 * i, 500); // Rising melody
    delay(500);
    CircuitPlayground.setPixelColor(led, 0, 0, 0); // Turn off the LED
    delay(250);
  }
}

bool checkUserSequence(int level) {
  for (int i = 0; i < level; i++) {
    if (userSequence[i] != sequence[i]) {
      return false; // Sequence does not match
    }
  }
  return true; // Sequence matches
}

void setAllLEDs(int red, int green, int blue) {
  for (int i = 0; i < 10; i++) {
    CircuitPlayground.setPixelColor(i, red, green, blue);
  }
}

void setLEDColor(int index, int color) {
  switch (color) {
    case 0: // Green
      CircuitPlayground.setPixelColor(index, 0, 255, 0);
      break;
    case 1: // Blue
      CircuitPlayground.setPixelColor(index, 0, 0, 255);
      break;
    case 2: // Red
      CircuitPlayground.setPixelColor(index, 255, 0, 0);
      break;
  }
}

void highlightCurrentLED(int led) {
  CircuitPlayground.setPixelColor(led, 255, 255, 255); // Highlight current LED
}

void switchh() {
  intFlag = 1;
  gameState = !gameState;
  userIndex = 0; // Reset user index
  level = 1; // Reset level
  clearUserSequence(); // Clear user sequence
  setAllLEDs(255, 255, 255); // Reset all LEDs to white
}

void clearUserSequence() {
  for (int i = 0; i < 20; i++) {
    userSequence[i] = 0; // Clear the user sequence
  }
}

void leftButtonPress() {
  if (gameState) {
    if (userSequence[currentLED] == -1) { // Turn off only if not selected
      CircuitPlayground.setPixelColor(currentLED, 0, 0, 0); // Turn off current LED
    } else {
      setLEDColor(currentLED, userSequence[currentLED] % 10); // Restore selected color
    }
    currentLED = (currentLED + 1) % 10; // Cycle to the next LED
    highlightCurrentLED(currentLED); 
  }
}

void rightButtonPress() {
  if (gameState) {
    selectCurrentPixel();
  }
}

void tapTime() {
  // Cycle through colors: Green -> Blue -> Red
  currentColor = (currentColor + 1) % 3;
  setLEDColor(currentLED, currentColor);
}

void selectCurrentPixel() {
  // Store the color of the selected LED in user sequence
  userSequence[userIndex] = (currentLED * 10) + currentColor; // Combine LED and color

  // Code to handle selecting the current pixel
  Serial.print("Selected Pixel: ");
  Serial.println(currentLED);
  userIndex++;
  if (userIndex < level) {
    currentLED = 0; // Reset to the first LED
    highlightCurrentLED(currentLED); // Highlight new current LED
  }

  // Keep the selected LED lit with the correct color
  setLEDColor(currentLED, currentColor);

  // Play two fast-paced tones if level is 2 or above
  if (level >= 2) {
    CircuitPlayground.playTone(500, 100);
    delay(100);
    CircuitPlayground.playTone(500, 100);
  }
}

void correctSequenceFeedback() {
  // Flash green twice and play a tone
  for (int i = 0; i < 2; i++) {
    setAllLEDs(0, 255, 0); // Green
    CircuitPlayground.playTone(523, 200); // Short happy tone
    delay(200);
    setAllLEDs(0, 0, 0); 
    delay(200);
  }
  CircuitPlayground.clearPixels();
  clearUserSequence(); 
}

void incorrectSequenceFeedback() {
  // Flash red and play a losing tone
  setAllLEDs(255, 0, 0); // Red
  CircuitPlayground.playTone(100, 1000); // Error tone
  Serial.println("LOSER HAHA");
  delay(1000);
  CircuitPlayground.clearPixels();
  Score = 0;
  level = 1;
  clearUserSequence();
}