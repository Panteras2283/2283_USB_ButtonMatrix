#include <FastLED.h>
#include <Joystick.h>
#include <EEPROM.h>

Joystick_ Joystick;
#define NUM_LEDS 36
#define DATA_PIN 2
CRGB leds[NUM_LEDS];

const byte ROWS = 6;
const byte COLS = 6;

byte rowPins[ROWS] = {3,4,5,6,7,8};
byte colPins[COLS] = {A0,15,14,16,10,9};

unsigned long previousMillis = 0;
int count = 0;

// --- NEW STATE VARIABLES ---
int currentProfile = 0; // 0 for Profile A, 1 for Profile B
int animationState = 0; // 0 = Custom Profile, 1 = Shooting Star, 2 = White, 3 = Black
int brightness = 100;

// Stored colors for 32 buttons across 2 profiles. 
CRGB profileColors[2][32]; 

// Serial reading variables
const byte numChars = 32;
char receivedChars[numChars];
boolean newData = false;

int lastButtonState[6][6] = {{0,0,0,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0,0,0}};

int buttonIDs[6][6] = {{0,6,12,18,24,30},
                       {1,7,13,19,25,31},
                       {2,8,14,20,26,32},
                       {3,9,15,21,27,33},
                       {4,10,16,22,28,34},
                       {5,11,17,23,29,35}};

int ledIDs[6][6] =    {{35,34,33,32,31,30},
                       {24,25,26,27,28,29},
                       {23,22,21,20,19,18},
                       {12,13,14,15,16,17},
                       {11,10,9,8,7,6},
                       {0,1,2,3,4,5}};

void setup() {
  for(int i = 0; i < ROWS; i++) pinMode(rowPins[i], INPUT_PULLUP);
  for(int i = 0; i < COLS; i++) pinMode(colPins[i], OUTPUT);

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(brightness);
  
  Serial.begin(9600); // Web Serial Configurator
  Joystick.begin();

  loadFromEEPROM(); // Load saved colors on boot
}

void loadFromEEPROM() {
  // EEPROM Address mapping: 0-95 for Profile 0, 100-195 for Profile 1
  for(int p = 0; p < 2; p++) {
    int startAddr = p * 100;
    for(int i = 0; i < 32; i++) {
      profileColors[p][i].r = EEPROM.read(startAddr + (i*3));
      profileColors[p][i].g = EEPROM.read(startAddr + (i*3) + 1);
      profileColors[p][i].b = EEPROM.read(startAddr + (i*3) + 2);
    }
  }
}

void saveToEEPROM() {
  for(int p = 0; p < 2; p++) {
    int startAddr = p * 100;
    for(int i = 0; i < 32; i++) {
      EEPROM.update(startAddr + (i*3), profileColors[p][i].r);
      EEPROM.update(startAddr + (i*3) + 1, profileColors[p][i].g);
      EEPROM.update(startAddr + (i*3) + 2, profileColors[p][i].b);
    }
  }
}

// Format: <SET,profile,id,r,g,b> or <SYNC> or <FLASH,r,g,b>
void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();
        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) ndx = numChars - 1;
            } else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        } else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

void parseData() {
    if (newData == true) {
        char * strtokIndx;
        strtokIndx = strtok(receivedChars, ",");
        
        if(strcmp(strtokIndx, "SET") == 0) {
            int p = atoi(strtok(NULL, ","));
            int bID = atoi(strtok(NULL, ","));
            int r = atoi(strtok(NULL, ","));
            int g = atoi(strtok(NULL, ","));
            int b = atoi(strtok(NULL, ","));
            if(bID < 32 && p < 2) {
              profileColors[p][bID] = CRGB(r, g, b);
            }
        } 
        else if (strcmp(strtokIndx, "SYNC") == 0) {
            saveToEEPROM();
            // Flash entire board green to confirm save
            fill_solid(leds, NUM_LEDS, CRGB::Green);
            FastLED.show();
            delay(500);
        }
        else if (strcmp(strtokIndx, "FLASH") == 0) {
            int r = atoi(strtok(NULL, ","));
            int g = atoi(strtok(NULL, ","));
            int b = atoi(strtok(NULL, ","));
            fill_solid(leds, NUM_LEDS, CRGB(r,g,b));
            FastLED.show();
            delay(300); // Quick flash
        }
        newData = false;
    }
}

void applyCurrentProfile() {
  // Use the matrices to map the logical button ID to the correct physical LED
  for(int i = 0; i < COLS; i++){
    for(int j = 0; j < ROWS; j++){
      int bID = buttonIDs[i][j];
      int lID = ledIDs[j][i];
      if(bID < 32) {
        leds[lID] = profileColors[currentProfile][bID];
      }
    }
  }
}

void drawHardwareButtons() {
  // Always keep the hardware control buttons illuminated
  for(int i = 0; i < COLS; i++){
    for(int j = 0; j < ROWS; j++){
      int bID = buttonIDs[i][j];
      int lID = ledIDs[j][i];
      
      if(bID == 32) leds[lID] = CRGB::White;      // Brightness Down
      else if(bID == 33) leds[lID] = CRGB::White; // Brightness Up
      else if(bID == 34) leds[lID] = CRGB::Blue;  // Animation Toggle
      else if(bID == 35) {                        // Profile Switch
         leds[lID] = (currentProfile == 0) ? CRGB::Purple : CRGB::Orange;
      }
    }
  }
}

void shootingStarAnimation(int red, int green, int blue, int tail_length, int delay_duration, int interval, int direction){
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    count = 0;
  }
  if (count < NUM_LEDS) {
      leds[count].setRGB(red, green, blue);
      count++;
  }
  fadeToBlackBy(leds, NUM_LEDS, tail_length);
  FastLED.show();
  delay(delay_duration);
}

void loop() {
  recvWithStartEndMarkers();
  parseData();

  // --- 4-STATE ANIMATION LOGIC ---
  if (animationState == 0) {
    applyCurrentProfile();
  } else if (animationState == 1) {
    shootingStarAnimation(0, 255, 255, 30, 20, 2000, 1);
  } else if (animationState == 2) {
    fill_solid(leds, NUM_LEDS, CRGB::White); // Full White
  } else if (animationState == 3) {
    fill_solid(leds, NUM_LEDS, CRGB::Black); // Full Black (Off)
  }
  
  // Overwrite the animation/profile to ensure hardware buttons are always visible
  drawHardwareButtons();

  // Scan Matrix
  for(int i = 0; i < COLS; i++){
    digitalWrite(colPins[i], LOW);
    for(int j = 0; j < ROWS; j++){
      int currentButtonState = !digitalRead(rowPins[j]);
      int bID = buttonIDs[i][j];
      int lID = ledIDs[j][i];

      if(currentButtonState != lastButtonState[i][j]){
        
        // --- STANDARD BUTTONS (Send to PC) ---
        if(bID < 32) {
          Joystick.setButton(bID, currentButtonState);
        } 
        // --- HARDWARE CONTROLS (Do NOT send to PC) ---
        else if (currentButtonState == 1) { 
           if(bID == 32) { brightness = max(10, brightness - 20); FastLED.setBrightness(brightness); }
           else if(bID == 33) { brightness = min(255, brightness + 20); FastLED.setBrightness(brightness); }
           else if(bID == 34) { animationState = (animationState + 1) % 4; } // Cycle 0 -> 1 -> 2 -> 3 -> 0
           else if(bID == 35) { currentProfile = !currentProfile; } 
        }
        lastButtonState[i][j] = currentButtonState;
      }
      
      // Visual feedback when pressed (turns Cyan temporarily)
      if(currentButtonState == 1 && bID < 32){
        leds[lID] = CRGB::Cyan; 
      }
    }
    digitalWrite(colPins[i], HIGH);
  }
  FastLED.show();
}