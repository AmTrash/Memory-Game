# Interactive Memory Game using Circuit Playground Express

## Description
This is a memory sequence game using the Circuit Playground Express (CPX). The game starts with all pixels lit up in white. When the player is ready, they use the switch to start the game. The game follows a "ready, set, go" sequence, the same as a race car start. The CPX will then light up a random sequence of pixel locations, followed by a rising melody for each LED being lit. As the player's level increases, the number of pixels in the sequence also increases. The player must memorize and replicate the sequence using the buttons on the CPX. Correct sequences result in positive feedback with green lights, a nice tone, and the score increasing by 1. If the player guesses incorrectly, the game shows red lights, plays a loser tone followed by a voice saying "LOSER HAHA," and resets the score to 0. The game can be reset using the switch.

## Rules
- Start Game:
  - All pixels light up white initially.
  - The player flips the switch to start the game.
- Ready, Set, Go:
  - The CPX displays a "ready, set, go" sequence.
- Sequence Display:
  - CPX lights up a sequence of random pixel locations.
  - The sequence length increases with each level.
- Player Input:
  - The player uses the left button to cycle through the LEDs.
  - The player uses the right button to select the current LED.
  - The player uses taps to change the color of the current LED.
- Feedback:
  - If the player gets the sequence correct, the CPX flashes green twice, plays a happy tone, and the score increases by 1.
  - If the player gets the sequence wrong, the CPX flashes red, plays a losing tone, and the score resets to 0.
- Reset Game:
  - The player flips the switch to reset the game.

## Inputs
- Switch: Start and reset the game.
- Left Button: Cycle through the LEDs.
- Right Button: Select the current LED.
- Tap: Change the color of the current LED.

## Outputs
- LEDs: Display sequences, feedback, and game status.
- Speaker: Play tones for feedback.

## Code
The `.ino` file for the project is included in this repository.
