#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Pins for the KY-040 rotary encoder
const int CLK_PIN = 2;  // Clock pin
const int DT_PIN = 3;   // Data pin 
const int SW_PIN = 4;   // Switch pin
const int rLED_PIN = 5;  // red led
const int bLED_PIN = 6;  // blue led

// Variables to store encoder state
volatile int lastEncoded = 0;
volatile long encoderValue = 0;

const int ENCODER_STEP = 1; // Change this value as needed

// Menu states
enum MenuState {
  MAIN_MENU,
  HEAT_MENU,
  COOL_MENU,
  ON_OFF_BACK_MENU,
  ON_OFF_PERCENTAGE_MENU
};

MenuState currentMenu = MAIN_MENU;

// Menu items
const char* mainMenuItems[] = {"Heat", "Cool"};
const char* heatMenuItems[] = {"On", "Off", "Back"};
const char* coolMenuItems[] = {"On", "Off", "Back"};
const char* onOffBackMenuItems[] = {"On", "Off", "Back"};
const char* onOffPercentageMenuItems[] = {"Percentage", "Back"};

int selectedItemIndex = 0; // Selected menu item index

// Settings
bool heatOn = false;
bool coolOn = false;
bool heatSettingChanged = false;
bool coolSettingChanged = false;
int heatPercentage = 0;

void setup() {
  pinMode(CLK_PIN, INPUT);
  pinMode(DT_PIN, INPUT);
  pinMode(SW_PIN, INPUT_PULLUP);
  pinMode(rLED_PIN, OUTPUT);
  pinMode(bLED_PIN, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(CLK_PIN), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(DT_PIN), updateEncoder, CHANGE);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.clearDisplay();
  display.display();
}

void loop() {
  // Handle menu navigation
  switch (currentMenu) {
    case MAIN_MENU:
      displayMenu(mainMenuItems, 2);
      break;
    case HEAT_MENU:
      displayMenu(heatMenuItems, 3);
      break;
    case COOL_MENU:
      displayMenu(coolMenuItems, 3);
      break;
    case ON_OFF_BACK_MENU:
      displayMenu(onOffBackMenuItems, 3);
      break;
    case ON_OFF_PERCENTAGE_MENU:
      displayMenu(onOffPercentageMenuItems, 2);
      break;
  }

  // Read encoder input and update menu selection
  int delta = readEncoder();
  if (delta != 0) {
    selectedItemIndex = (selectedItemIndex + delta) % getMenuSize();
    if (selectedItemIndex < 0) {
      selectedItemIndex += getMenuSize();
    }
    delay(200); // Debounce
  }

  // Handle menu item selection
  if (digitalRead(SW_PIN) == LOW) {
    switch (currentMenu) {
      case MAIN_MENU:
        if (selectedItemIndex == 0) {
          currentMenu = HEAT_MENU;
        } else if (selectedItemIndex == 1) {
          currentMenu = COOL_MENU;
        }
        break;
      case HEAT_MENU:
        if (selectedItemIndex == 0) {
          display.clearDisplay();
          display.setTextSize(3);
          display.setCursor(44, 22);
          display.println("ON");
          display.display();
          delay(2000);
          currentMenu = MAIN_MENU;
          digitalWrite(rLED_PIN, HIGH); // Turn on heat LED
          digitalWrite(bLED_PIN, LOW); // Turn off cool LED
        } else if (selectedItemIndex == 1) {
          display.clearDisplay();
          display.setTextSize(3);
          display.setCursor(44, 22);
          display.println("OFF");
          display.display();
          delay(2000);
          currentMenu = MAIN_MENU;
          digitalWrite(rLED_PIN, LOW); // Turn off heat LED
        } else if (selectedItemIndex == 2) {
          currentMenu = MAIN_MENU;
          selectedItemIndex = 0;
        }
        break;

      case COOL_MENU:
        if (selectedItemIndex == 0) {
          display.clearDisplay();
          display.setTextSize(3);
          display.setCursor(44, 22);
          display.println("ON");
          display.display();
          delay(2000);
          currentMenu = MAIN_MENU;
          digitalWrite(bLED_PIN, HIGH); // Turn on cool LED
          digitalWrite(rLED_PIN, LOW); // Turn off heat LED
        } else if (selectedItemIndex == 1) {
          display.clearDisplay();
          display.setTextSize(3);
          display.setCursor(44, 22);
          display.println("OFF");
          display.display();
          delay(2000);
          currentMenu = MAIN_MENU;
          digitalWrite(bLED_PIN, LOW); // Turn off cool LED
        } else if (selectedItemIndex == 2) {
          currentMenu = MAIN_MENU;
          selectedItemIndex = 1;
        }
        break;

      

      case ON_OFF_BACK_MENU:
        if (selectedItemIndex == 0) {
          if (currentMenu == HEAT_MENU) {
            heatOn = true;
          } else if (currentMenu == COOL_MENU) {
            coolOn = true;
          }
          currentMenu = MAIN_MENU;
          selectedItemIndex = 0;
        } else if (selectedItemIndex == 1) {
          if (currentMenu == HEAT_MENU) {
            heatOn = false;
          } else if (currentMenu == COOL_MENU) {
            coolOn = false;
          }
          currentMenu = MAIN_MENU;
          selectedItemIndex = 1;
        } else if (selectedItemIndex == 2) {
          currentMenu = MAIN_MENU;
          selectedItemIndex = 0;
        }
        break;
            case ON_OFF_PERCENTAGE_MENU:
        if (selectedItemIndex == 0) {
          currentMenu = MAIN_MENU;
          selectedItemIndex = 0;
        } else if (selectedItemIndex == 1) {
          currentMenu = MAIN_MENU;
          selectedItemIndex = 0;
        }
        break;
    }
    delay(200); // Debounce
  }

  // Update settings
  if (heatSettingChanged) {
    Serial.print("Heat ");
    Serial.println(heatOn ? "On" : "Off");
    heatSettingChanged = false;
  }
  if (coolSettingChanged) {
    Serial.print("Cool ");
    Serial.println(coolOn ? "On" : "Off");
    coolSettingChanged = false;
  }
}
void displayMenu(const char* items[], int itemCount) {
  display.clearDisplay();

  // Display current screen name at the top left corner
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2); // Set text size to 2 for "HOME"
  display.setCursor(0, 0);
  switch (currentMenu) {
    case MAIN_MENU:
      display.println("HOME");
      break;
    case HEAT_MENU:
      display.println("HEAT");
      break;
    case COOL_MENU:
      display.println("COOL");
      break;
    default:
      break;
  }

  // Display menu items
  int yPos = 30; // Initial Y position after "HOME"
  for (int i = 0; i < itemCount; ++i) {
    display.setTextSize(1); // Reset text size to 1 for menu items
    display.setCursor(0, yPos);
    if (i == selectedItemIndex) {
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }
    display.println(items[i]);
    yPos += 10; // Increase Y position for smaller line spacing
  }

  display.display();
}


int getMenuSize() {
  switch (currentMenu) {
    case MAIN_MENU:
      return 2;
    case HEAT_MENU:
    case COOL_MENU:
    case ON_OFF_BACK_MENU:
      return 3;
    case ON_OFF_PERCENTAGE_MENU:
      return 2;
    default:
      return 0;
  }
}

void updateEncoder() {
  static int lastState = LOW;
  int currentStateCLK = digitalRead(CLK_PIN);
  int currentStateDT = digitalRead(DT_PIN);

  if (currentStateCLK != lastState) {
    if (currentStateCLK == HIGH) {
      if (currentStateDT == HIGH) {
        // Clockwise rotation
        if (currentMenu == MAIN_MENU) {
          selectedItemIndex = (selectedItemIndex + 1) % 2;
        } else if (currentMenu == HEAT_MENU || currentMenu == COOL_MENU || currentMenu == ON_OFF_BACK_MENU || currentMenu == ON_OFF_PERCENTAGE_MENU) {
          selectedItemIndex = (selectedItemIndex + 1) % 3;
        }
      } else {
        // Counterclockwise rotation
        if (currentMenu == MAIN_MENU) {
          selectedItemIndex = (selectedItemIndex - 1 + 2) % 2;
        } else if (currentMenu == HEAT_MENU || currentMenu == COOL_MENU || currentMenu == ON_OFF_BACK_MENU || currentMenu == ON_OFF_PERCENTAGE_MENU) {
          selectedItemIndex = (selectedItemIndex - 1 + 3) % 3;
        }
      }

      // Update submenu navigation for "heat" menu
      if (currentMenu == HEAT_MENU && selectedItemIndex == 3) {
        currentMenu = ON_OFF_BACK_MENU; // Navigate to submenu
        selectedItemIndex = 0; // Reset submenu index
      }
    }
  }

  lastState = currentStateCLK;
}


int readEncoder() {
  int MSB = digitalRead(CLK_PIN);
  int LSB = digitalRead(DT_PIN);
  int encoded = (MSB << 1) | LSB;
  int sum = (lastEncoded << 2) | encoded;

  int delta = 0;
  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) {
    delta = 1;
  } else if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) {
    delta = -1;
  }

  lastEncoded = encoded;
  return delta;
}

