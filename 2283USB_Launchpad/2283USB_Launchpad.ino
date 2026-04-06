#include <FastLED.h>
#include <Joystick.h>

Joystick_ Joystick;
#define NUM_LEDS 36
#define DATA_PIN 2
CRGB leds[NUM_LEDS];

const byte ROWS = 6;
const byte COLS = 6;

int index = 0;

byte rowPins[ROWS] = {3,4,5,6,7,8};
byte colPins[COLS] = {A0,15,14,16,10,9};

unsigned long previousMillis = 0;           // Stores last time LEDs were updated
int count = 0;                              // Stores count for incrementing up to the NUM_LEDs

void shootingStarAnimation(int red, int green, int blue, int tail_length, int delay_duration, int interval, int direction){
  /*
   * red - 0 to 255 red color value
   * green - 0 to 255 green color value
   * blue - 0 to 255 blue color value
   * tail_length - value which represents number of pixels used in the tail following the shooting star
   * delay_duration - value to set animation speed. Higher value results in slower animation speed.
   * interval - time between each shooting star (in miliseconds)
   * direction - value which changes the way that the pixels travel (uses -1 for reverse, any other number for forward)
  */
  unsigned long currentMillis = millis();   // Get the time
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;         // Save the last time the LEDs were updated
    count = 0;                              // Reset the count to 0 after each interval
  }
  if (direction == -1) {        // Reverse direction option for LEDs
    if (count < NUM_LEDS) {
      leds[NUM_LEDS - (count % (NUM_LEDS+1))].setRGB(red, green, blue);    // Set LEDs with the color value
      count++;
    }
  }
  else {
    if (count < NUM_LEDS) {     // Forward direction option for LEDs
      leds[count % (NUM_LEDS+1)].setRGB(red, green, blue);    // Set LEDs with the color value
      count++;
    }
  }
  fadeToBlackBy(leds, NUM_LEDS, tail_length);                 // Fade the tail LEDs to black
  FastLED.show();
  delay(delay_duration);                                      // Delay to set the speed of the animation
}

void setup() {
  // put your setup code here, to run once:
  for(int i = 0; i < ROWS; i++){
    pinMode(rowPins[i], INPUT_PULLUP);
  }
  for(int i = 0; i < COLS; i++){
    pinMode(colPins[i], OUTPUT);
  }

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  Serial.begin(9600);
  Joystick.begin();

}

int lastButtonState[6][6] = {{0,0,0,0,0,0},
                             {0,0,0,0,0,0},
                             {0,0,0,0,0,0},
                             {0,0,0,0,0,0},
                             {0,0,0,0,0,0},
                             {0,0,0,0,0,0}};

int buttonIDs[6][6] = {{0,6,12,18,24,30},
                       {1,7,13,19,25,31},
                       {2,8,14,20,26,32},
                       {3,9,15,21,27,33},
                       {4,10,16,22,28,34},
                       {5,11,17,23,29,35}};

int ledIDs[6][6] =     {{35,34,33,32,31,30},
                       {24,25,26,27,28,29},
                       {23,22,21,20,19,18},
                       {12,13,14,15,16,17},
                       {11,10,9,8,7,6},
                       {0,1,2,3,4,5}};

void ledDefaults(){
  leds[30] = CRGB :: Red; //Disparar
  leds[35] = CRGB :: Blue; //Speaker
  leds[34] = CRGB :: Green; //Amp
  leds[33] = CRGB :: Violet; //Pegado
  leds[29] = CRGB :: Yellow; //Comer
  leds[24] = CRGB :: OrangeRed; //Handoff
  leds[18] = CRGB :: Peru; //Source
  leds[21] = CRGB :: Magenta; //Climb
}

void loop() {
  // put your main code here, to run repeatedly:
  for(int i = 0; i<=72; i++){
  shootingStarAnimation(0, 255, 255, random(10, 60), random(5, 40), random(2000, 8000), 1);
  }
  while(true){
  for(int i = 0; i<NUM_LEDS; i++){
    leds[i] = CRGB :: Black;
  }
  ledDefaults();
  FastLED.show();

  for(int i = 0; i < COLS; i++){
    digitalWrite(colPins[i],LOW);
    for(int j = 0; j < ROWS; j++){
      int currentButtonState = !digitalRead(rowPins[j]);
      if(currentButtonState != lastButtonState[i][j]){
        Joystick.setButton(buttonIDs[i][j], currentButtonState);
        lastButtonState[i][j]=currentButtonState;
      }
        if(currentButtonState == 1){
        leds[ledIDs[j][i]] = CRGB :: Cyan;
        FastLED.show();

      }
    }
    digitalWrite(colPins[i],HIGH);
  }

}
}
