// Juliette Park
// April, 2024
// CSE 493 F - A1 Interactive Nightlight
// Project: Gentle Ger Getaway

// The following code describes the Arduino code for an interactive 
// nightlight with 3 modes (selected via buttons).
// Mode 1: RGB fade in/out through a full rainbow with photoresistor (light) responsiveness
// Mode 2: User's lofi-input (DIY potentiometer) to decide color.
// Mode 3: Party mode! Closing the ger's door will make the lights flash as if there is a party inside.

// Credits: setColor(), modeFadeLights(), and setRGBLedColor() were based on
// Jon Froehlich's code; access source here 
// https://makeabilitylab.github.io/physcomp/arduino/rgb-led.html

const boolean COMMON_ANODE = false; 

const int RGB_RED_PIN = 6;
const int RGB_GREEN_PIN  = 5;
const int RGB_BLUE_PIN  = 3;
const int DELAY_MS = 20; // delay in ms between changing colors
const int MAX_COLOR_VALUE = 255; 

// PHOTORESISTOR
const int PHOTOCELL_INPUT_PIN = A0;
const int MIN_PHOTOCELL_VAL = 200;
const int MAX_PHOTOCELL_VAL = 800;
int light = 255;

// LOFI INPUT
const int LOFI_INPUT_PIN = A1;
const int MIN_LOFI_VAL = 150;
const int MAX_LOFI_VAL = 800;
const int NUM_COLOR_OPTIONS = 6;

// BUTTON MODE
const int BUTTON_PIN = 2;
const int NUMBER_OF_MODES = 3;
const int OFF = 1;
const int ON = 0;

// PARTY DOOR
const int PARTY_DELAY = 100;
const int PARTY_INPUT_PIN = 8;

enum RGB{
  RED,
  GREEN,
  BLUE,
  NUM_COLORS
};

int _rgbLedValues[] = {255, 0, 0}; // Red, Green, Blue
enum RGB _curFadingUpColor = GREEN;
enum RGB _curFadingDownColor = RED;
const int FADE_STEP = 5;

int currMode = 1; // default mode 1
int pastState = 1; // default unpressed

// red, blue, green, purple, cyan, white in that order
int _colorOptions[NUM_COLOR_OPTIONS][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 0}, {0, 1, 1}, {1, 1, 1}};

void setup()
{
  // RGB pins for output
  pinMode(RGB_RED_PIN, OUTPUT);
  pinMode(RGB_GREEN_PIN, OUTPUT);
  pinMode(RGB_BLUE_PIN, OUTPUT);
  
  // Photoresistor input
  pinMode(PHOTOCELL_INPUT_PIN, INPUT);

  // Lofi color picker input
  pinMode(LOFI_INPUT_PIN, INPUT);
  
  // Button input pin
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Party door input
  pinMode(PARTY_INPUT_PIN, INPUT_PULLUP);
  
  // Turn on Serial so we can verify expected colors via Serial Monitor
  Serial.begin(9600); 
  Serial.println("Red, Green, Blue");

  // Set initial color
  setColor(_rgbLedValues[RED], _rgbLedValues[GREEN], _rgbLedValues[BLUE], light);
  delay(DELAY_MS);
}

void loop()
{
  int button = digitalRead(BUTTON_PIN);
  
  if(pastState == OFF && button == ON) {
  	// Means button used to be unpressed but is now pressed
    // Increment count
    currMode++;
    if(currMode > NUMBER_OF_MODES) {
    	currMode = 1; // 1-based mode system
      	resetRGB(); // reset to start of fade
    }
  }
    
  pastState = button; // log last button state
  
  if(currMode == 1) { // Mode 1: RGB fade w/ photoresistor
  	modeFadeLights();
  } else if(currMode == 2){ // Mode 2: Lofi input to select color
  	// setRGBOff();
    modeSelectColor();
  } else {
  	modeParty(); // Mode 3: Party!
  }
  
}

// When the button (lofi button in the shape of a door)
// is pressed, lights will flash between colors
void modeParty()
{ 
  int doorClosed = digitalRead(PARTY_INPUT_PIN);
  if(doorClosed == 0) {
    // red
    setRgbLedColor(HIGH, LOW, LOW);
    delay(PARTY_DELAY);

    // green
    setRgbLedColor(LOW, HIGH, LOW);
    delay(PARTY_DELAY);

    // blue
    setRgbLedColor(LOW, LOW, HIGH);
    delay(PARTY_DELAY);

    // purple
    setRgbLedColor(HIGH, LOW, HIGH);
    delay(PARTY_DELAY);

    // turqoise
    setRgbLedColor(LOW, HIGH, HIGH);
    delay(PARTY_DELAY);

    // white
    setRgbLedColor(HIGH, HIGH, HIGH);
    delay(PARTY_DELAY);
  }
}

// This method reads in the lofi potentiometer input
// to decide and set a color.
// Each color is designated an equal portion of the total range.
void modeSelectColor() 
{
  // Read the lofi input value for color
  int lofiVal = analogRead(LOFI_INPUT_PIN);
  Serial.print("Lofi input val: ");
  Serial.println(lofiVal);
  
  // Decide color 
  // Multiply total range so each takes up 1-range worth of values
  int colorpick = map(lofiVal, MIN_LOFI_VAL, MAX_LOFI_VAL, MIN_LOFI_VAL, NUM_COLOR_OPTIONS*MAX_LOFI_VAL);
  
  int lowerRange = 0;
  int upperRange = MAX_LOFI_VAL;
  
  for(int i = 0; i < NUM_COLOR_OPTIONS; i++) {
    //if(colorpick < upperRange && colorpick >= lowerRange) {
    if(colorpick < upperRange) {
      // grab the color needed
      int red = _colorOptions[i][0];
      int green = _colorOptions[i][1];
      int blue = _colorOptions[i][2];
      setRgbLedColor(red, green, blue);
      break;
    }
    
    // update range
  	lowerRange = upperRange;
    upperRange += MAX_LOFI_VAL;
  }
}

// This method takes digital input (0 or 1) for each of the colors
// and sets the RGB values appropriately.
void setRgbLedColor(int red, int green, int blue)
{
  // If a common anode LED, invert values
  if(COMMON_ANODE == true){
    red = !red;
    green = !green;
    blue = !blue;
  }
  
  digitalWrite(RGB_RED_PIN, red);
  digitalWrite(RGB_GREEN_PIN, green);
  digitalWrite(RGB_BLUE_PIN, blue);  
}

// This method fades the RGB colors in and out through the entire rainbow
// Based on Jon Froehlich's code at https://makeabilitylab.github.io/physcomp/arduino/rgb-led.html
// Additionally takes in photoresistor values to adjust brightness of LED.
void modeFadeLights() {
  // Increment and decrement the RGB LED values for the current
  // fade up color and the current fade down color
  _rgbLedValues[_curFadingUpColor] += FADE_STEP;
  _rgbLedValues[_curFadingDownColor] -= FADE_STEP;
  int photocellVal = analogRead(PHOTOCELL_INPUT_PIN);
  
  // Now, map range to a 0-255 value
  // Set the max color value based on this light level
  light = map(photocellVal, MIN_PHOTOCELL_VAL, MAX_PHOTOCELL_VAL, 0, 255);
  light = constrain(light, 0, 255);
  

  // Check to see if we've reached our maximum color value for fading up
  // If so, go to the next fade up color (we go from RED to GREEN to BLUE
  // as specified by the RGB enum)
  // This fade code partially based on: https://gist.github.com/jamesotron/766994
  if(_rgbLedValues[_curFadingUpColor] > MAX_COLOR_VALUE){
    _rgbLedValues[_curFadingUpColor] = MAX_COLOR_VALUE;
    _curFadingUpColor = (RGB)((int)_curFadingUpColor + 1);

    if(_curFadingUpColor > (int)BLUE){
      _curFadingUpColor = RED;
    }
  }

  // Check to see if the current LED we are fading down has gotten to zero
  // If so, select the next LED to start fading down (again, we go from RED to 
  // GREEN to BLUE as specified by the RGB enum)
  if(_rgbLedValues[_curFadingDownColor] < 0){
    _rgbLedValues[_curFadingDownColor] = 0;
    _curFadingDownColor = (RGB)((int)_curFadingDownColor + 1);

    if(_curFadingDownColor > (int)BLUE){
      _curFadingDownColor = RED;
    }
  }

  // Set the color and then delay
  setColor(_rgbLedValues[RED], _rgbLedValues[GREEN], _rgbLedValues[BLUE], light);
  delay(DELAY_MS);
}

/**
 * setColor takes in values between 0 - 255 for the amount of red, green, and blue, respectively
 * where 255 is the maximum amount of that color and 0 is none of that color. You can illuminate
 * all colors by intermixing different combinations of red, green, and blue
 * 
 * This function is based on https://gist.github.com/jamesotron/766994
 */
void setColor(int red, int green, int blue, int light) {

  // If a common anode LED, invert values
  if(COMMON_ANODE == true){
    red = MAX_COLOR_VALUE - red;
    green = MAX_COLOR_VALUE - green;
    blue = MAX_COLOR_VALUE - blue;
  }
  
  // Transform the color values to respond to light level
  red = map(red, 0, 255, 0, light);
  red = constrain(red, 0, light);
  
  green = map(green, 0, 255, 0, light);
  green = constrain(green, 0, light);
  
  blue = map(blue, 0, 255, 0, light);
  blue = constrain(blue, 0, light);
  
  analogWrite(RGB_RED_PIN, red);
  analogWrite(RGB_GREEN_PIN, green);
  analogWrite(RGB_BLUE_PIN, blue);  
}

// This function sets all RGB values to 0 (turns off)
void setRGBOff() {
  _rgbLedValues[RED] = 0;
  _rgbLedValues[GREEN] = 0;
  _rgbLedValues[BLUE] = 0;
  setColor(0, 0, 0, 0);
}

// Resets RGB to start crossfade at the color red.
void resetRGB(){
  _rgbLedValues[RED] = 255;
  _rgbLedValues[GREEN] = 0;
  _rgbLedValues[BLUE] = 0;
  _curFadingUpColor = GREEN;
  _curFadingDownColor = RED;
}
