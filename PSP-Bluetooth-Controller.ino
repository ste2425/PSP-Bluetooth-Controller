#include <Bluepad32.h>

// Currently only use the first connected gamepad. Bluepad32 supports upto 4
// Would be good to allow multiple, maybe could use different controllers for emulators etc
GamepadPtr myGamepad = nullptr;

// Controller masks ---------------------------------------------
// These are coppied from Gamepad.h from Bluepad32 however they are declared against a Gamepad instance
// That or my lack of C++ knowledge prevents me from accessing them without a gamepad instance.
enum {
  DPAD_UP = 1 << 0,
  DPAD_DOWN = 1 << 1,
  DPAD_RIGHT = 1 << 2,
  DPAD_LEFT = 1 << 3,
};

enum {
  BUTTON_A = 1 << 0,
  BUTTON_B = 1 << 1,
  BUTTON_X = 1 << 2,
  BUTTON_Y = 1 << 3,
  BUTTON_SHOULDER_L = 1 << 4,
  BUTTON_SHOULDER_R = 1 << 5,
  BUTTON_TRIGGER_L = 1 << 6,
  BUTTON_TRIGGER_R = 1 << 7,
  BUTTON_THUMB_L = 1 << 8,
  BUTTON_THUMB_R = 1 << 9,
};

enum {
  MISC_BUTTON_SYSTEM = 1 << 0,  // AKA: PS, Xbox, etc.
  MISC_BUTTON_BACK = 1 << 1,    // AKA: Select, Share, -
  MISC_BUTTON_HOME = 1 << 2,    // AKA: Start, Options, +
};

// ---------------------------------------------------------------

typedef enum {
  TYPE_DPAD = 0,
  TYPE_BUTTONS = 1,
  TYPE_HOME = 2
} controllerType;

typedef struct {
  const byte pin;
  const int controllerMask;
  bool pressed;
  controllerType type;
} pinMap;

pinMap mappedPins[] = {
  // Bottom buttons
  { 15, MISC_BUTTON_SYSTEM, false, TYPE_HOME }, // home - D15 - Pin 20
  { 14, MISC_BUTTON_HOME, false, TYPE_HOME }, // start - D14 - Pin 19
  { 13, MISC_BUTTON_BACK, false, TYPE_HOME }, // select - D13 - Pin 16
  { 12, BUTTON_THUMB_R, false, TYPE_BUTTONS },  // screen - D12 - Pin 15

  // Left hand buttons
  { 11, BUTTON_SHOULDER_L, false, TYPE_BUTTONS  }, // Left Shoulder Button - D11 - pin 14
  { 10, DPAD_UP, false, TYPE_DPAD  }, // up - D10 - pin 13
  { 9, DPAD_DOWN, false, TYPE_DPAD  }, // down - D9 - pin 12
  { 8, DPAD_RIGHT, false, TYPE_DPAD  }, // right - D8 - Pin 11
  { 7, DPAD_LEFT, false, TYPE_DPAD  }, // left - D7 - Pin 10
  
  // Right hand buttons
  { 6, BUTTON_SHOULDER_R, false, TYPE_BUTTONS }, // Right shoulder Button - D6 - Pin 9
  { 5, BUTTON_A, false, TYPE_BUTTONS  }, // Cross - D5 - pin 8
  { 4, BUTTON_B, false, TYPE_BUTTONS  }, // Circle - D4 - Pin 7
  { 3, BUTTON_X, false, TYPE_BUTTONS  }, // Square - D3 - Pin 6
  { 2, BUTTON_Y, false, TYPE_BUTTONS  } // Triangle - D2 - Pin 5
};
const byte mappedPinsSize = sizeof(mappedPins) / sizeof(pinMap);

void setup() {

  Serial.begin(9600);
  
  while (!Serial) {
    ;
  }
  
  String fv = BP32.firmwareVersion();
  Serial.print("Firmware version installed: ");
  Serial.println(fv);

  BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);

  // This needs to be behind a reset button later
  BP32.forgetBluetoothKeys();
}


void onConnectedGamepad(GamepadPtr gp) {
  if (myGamepad == nullptr) {
    myGamepad = gp;
    
    Serial.println("CALLBACK: Gamepad is connected");
  }
}

void onDisconnectedGamepad(GamepadPtr gp) {
  if (myGamepad != nullptr && myGamepad == gp) {
    myGamepad = nullptr;
    
    Serial.println("CALLBACK: Gamepad is disconnected");
  }
}

void releaseUnusedPins(){
  for(byte i = 0; i < mappedPinsSize; i++){
    if (mappedPins[i].pressed == false){
      pinMode(mappedPins[i].pin, INPUT);
    }
  }
}

void resetPinPressed() {
  for(byte i = 0; i < mappedPinsSize; i++){
    mappedPins[i].pressed = false;
  }
}

void pressPin(byte pin) {  
    Serial.println("PRessing pin:");
    Serial.println(pin);
    
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}

void updateButtons(GamepadPtr gamepad) {
  for (byte i = 0; i < mappedPinsSize; i++){
    pinMap *mapping = &mappedPins[i];

    switch (mapping->type) {
      case TYPE_DPAD:
        if (gamepad->dpad() & mapping->controllerMask) {
          mapping->pressed = true;
          pressPin(mapping->pin);
        } else {
          mapping->pressed = false;
        }
      break;
      case TYPE_BUTTONS:
        if (gamepad->buttons() & mapping->controllerMask) {
          mapping->pressed = true;
          pressPin(mapping->pin);
        } else {          
          mapping->pressed = false;
        }
      break;
    }
  }
}

void loop() {
  BP32.update();
  
  if (myGamepad && myGamepad->isConnected()) {  
    updateButtons(myGamepad);
  } else {
    resetPinPressed();
  }
  
  releaseUnusedPins();
  
  delay(3);
}
