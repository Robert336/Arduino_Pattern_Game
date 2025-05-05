

// Arduino pin numbers
const int SW_pin = 2; // digital pin connected to switch output
const int X_pin = A0; // analog pin connected to X output
const int Y_pin = A1; // analog pin connected to Y output

const int LED_pin_pwm = 11;

const int LED_pin_up = 3; // yellow
const int LED_pin_down = 4; // green
const int LED_pin_left = 5; // blue
const int LED_pin_right = 6; // red

const int DELAY_TIME = 300;
const int MAX_LEVEL_SIZE = 6;
const int INIT_LEVEL_SIZE = 1;

const int DEADZONE = 212;
const int ANALOG_LOW_THRESHOLD = 512 - DEADZONE;
const int ANALOG_HIGH_THRESHOLD = 512 + DEADZONE;


const int LED_directions[4] = {LED_pin_up, LED_pin_down, LED_pin_left, LED_pin_right};

int current_level[MAX_LEVEL_SIZE]; // stores the generated level's disired inputs
int current_level_size = INIT_LEVEL_SIZE; // tracks the number of inputs for the current level

int current_step = 0; // tracks the step within the level
int last_stick_dir = -1;

bool isGenerating = true;

void setup() {
  // seed with random noise
  randomSeed(analogRead(A3));

  //pinMode(SW_pin, INPUT); // joystick button
  //digitalWrite(SW_pin, HIGH);
  
  Serial.begin(9600);
  displayFail(); // blink the lights
  delay(2000);
}

// Randomly generate a level
void generateLevel() {
  // generate X random ints between 0-3
  // [0, 0, 2, 1] -> [up, up, left, down]
  for(int i=0; i < current_level_size; i++){
    current_level[i] = random(4);
  }
}

// Display the level pattern to the lights
void presentLevel() {
  // loop through each of the desired inputs and display via the corresponding LED
  for (int i = 0; i < current_level_size; i++){
    digitalWrite(LED_directions[current_level[i]], HIGH);
    delay(DELAY_TIME);
    digitalWrite(LED_directions[current_level[i]], LOW);
    delay(DELAY_TIME);
    Serial.print("SHOWING DIRECTION: ");
    Serial.print(LED_directions[current_level[i]]);
    Serial.print("\n");
  }
}

// Failure light show -> ur bad at the game
void displayFail() {
  for (int i = 0; i <= 3; i++){
    delay(150);
    // Turn all LEDs ON
    digitalWrite(LED_directions[0], HIGH);
    digitalWrite(LED_directions[1], HIGH);
    digitalWrite(LED_directions[2], HIGH);
    digitalWrite(LED_directions[3], HIGH);
    delay(150);
    // Turn all LEDs OFF
    digitalWrite(LED_directions[0], LOW);
    digitalWrite(LED_directions[1], LOW);
    digitalWrite(LED_directions[2], LOW);
    digitalWrite(LED_directions[3], LOW);
  }  
}

// Win light show
void displayWin() {
  // Turn all LEDs ON sequentially
  for (int i = 0; i < sizeof(LED_directions) / sizeof(LED_directions[0]); i++) {
      digitalWrite(LED_directions[i], HIGH);
      delay(150);
  }
  // Turn all LEDs OFF sequentially
  for (int i = 0; i < sizeof(LED_directions) / sizeof(LED_directions[0]); i++) {
      digitalWrite(LED_directions[i], LOW);
      delay(150);
  }
  delay(100); // Additional delay at the end
}

// Debug stick inputs
void printStickPos(){
    //Serial.print("Switch:  ");
    //Serial.print(digitalRead(SW_pin));
    Serial.print("\n");
    Serial.print("X-axis: ");
    Serial.print(analogRead(X_pin));
    Serial.print("\n");
    Serial.print("Y-axis: ");
    Serial.println(analogRead(Y_pin));
    Serial.print("\n\n");
}

// returns a single direction [up, down, left, right]
// returns NULL if stick is in neutral positoin
int getStickDirection() {
  /* 
    TODO: handle the diagonal edge case
    moving the stick diagonally and checking directions sequentially
    favours directions in the order they are checked.
  */
  int x_pos_analog = analogRead(X_pin);
  int y_pos_analog = analogRead(Y_pin);

  if (x_pos_analog <= ANALOG_LOW_THRESHOLD){
    // UP
    return 0;
  } else if (x_pos_analog >= ANALOG_HIGH_THRESHOLD) {
    // DOWN
    return 1;
  } else if (y_pos_analog >= ANALOG_HIGH_THRESHOLD) {
    // LEFT
    return 2;
  } else if (y_pos_analog <= ANALOG_LOW_THRESHOLD) {
    // RIGHT
    return 3;
  }
  return -1; // NEUTRAL
}

// main game loop
void loop() {
  
  // Level setup and present the pattern
  if (isGenerating == true) { // State 1: Generating and Presenting
    generateLevel();
    presentLevel();
    isGenerating = false;
    current_step = 0; // reset step
  
  // Player's turn to mimic
  } else { // State 2: Player Input
    
    // set user input
    int stickDir = getStickDirection();

    // Check if the stick is in a new position that's not neutral
    if (stickDir != -1 || stickDir != last_stick_dir){
      // do nothing if no input or stick is held in the same position without returning to neutral

      // Check if the player's input matches the current step
      if (stickDir != current_level[current_step]){
        // FAIL: input does not match
        displayFail();
        current_level_size = INIT_LEVEL_SIZE; // reset level size on failure
        isGenerating = true; // Change state to generating mode

      // Check if the player completed the final step of the current level
      } else if (current_step == current_level_size - 1) {
        // WIN: Last/final step was completed
        displayWin();

        // Check if it's the last level
        if (current_level_size == MAX_LEVEL_SIZE){
          displayWin();
        } else {
          // Increment level size for the next lebel
          current_level_size++;
          delay(500); // Short delay before next level starts
        }
        
        isGenerating = true; // Change state to generating mode
      
      // If the input was correct but it's not the last step
      } else {
        // PASS: Current step was correct, move on to the next one.
        // show their input and go to the next step
        digitalWrite(LED_directions[stickDir], HIGH);
        delay(100); // Flash the LED briefly for feedback
        digitalWrite(LED_directions[stickDir], LOW);
        current_step++; // Move to the next step in the sequence/level
      }
    }

    // Update the last stick direction *only* if a valid direction was detected
    // This is crucial for preventing rapid fire inputs while holding the stick
    if (stickDir != -1) {
      last_stick_dir = stickDir;
    } else {
      // If the stick is in neutral (-1), reset last_stick_dir
      // This allows the player to make the same move again after returning to neutral
      last_stick_dir = -1;
    }
  }
  
  // printStickPos();
}
