#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>

// Pin Definitions ----------
#define RST_PIN        8
#define SS_PIN         10
#define LOCK_PIN       6
#define RESET_BTN_PIN  9

// EEPROM Definitions ----------
#define EEPROM_ADDR 0x50
#define EEPROM_PIN_ADDR 0x0000

// RFID Setup ----------
MFRC522 mfrc522(SS_PIN, RST_PIN);

// You can add more tags here (4-byte UIDs)
byte validTags[][4] = {
  {0xDE, 0xAD, 0xBE, 0xEF},
  {0x12, 0x34, 0x56, 0x78}
};
const int numTags = sizeof(validTags) / sizeof(validTags[0]);

// ---------- Keypad Setup ----------
const byte ROES = 3, CULS = 3;
char keys[ROES][CULS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
};
byte rowPins[ROES] = {D2, D3, D4};
byte colPins[CULS] = {D5, D6, D7};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROES, CULS);

// ---------- State Variables ----------
char storedPIN[5] = "0000";
char enteredPIN[5];
int pinIndex = 0;
unsigned long unlockTime = 0;
const unsigned long UNLOCK_DURATION = 5000;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  Wire.begin();

  pinMode(LOCK_PIN, OUTPUT);
  pinMode(RESET_BTN_PIN, INPUT_PULLUP);

  lockBox();
  loadPINFromEEPROM();
}

void loop() {
  // RFID Unlock
  if (checkRFID()) {
    unlockBox();
    delay(1000);
  }

  // PIN Input
  enteredPIN[pinIndex] = '\0';
  char key = keypad.getKey();
  if (key && isDigit(key)) {
    enteredPIN[pinIndex++] = key;
    if (pinIndex == 4) {
      enteredPIN[4] = '\0';
      if (strcmp(enteredPIN, storedPIN) == 0) {
        unlockBox();
      }
      else{
        Serial.println("Lock in brother. Wrong pin :(");
      }
      pinIndex = 0;
    }
  }

  // Reset Button
  if (digitalRead(RESET_BTN_PIN) == LOW) {
    delay(1000);
    if (digitalRead(RESET_BTN_PIN) == LOW) {
      enterResetMode();
    }
  }

  // Auto Re-lock
  if (digitalRead(LOCK_PIN) == HIGH && millis() - unlockTime > UNLOCK_DURATION) {
  lockBox();
  }
}

// Lock Control ----------
void unlockBox() {
  digitalWrite(LOCK_PIN, HIGH);
  unlockTime = millis();
  Serial.println("Unlocked!");
}

void lockBox() {
  digitalWrite(LOCK_PIN, LOW);
}

// RFID Functions ----------
bool checkRFID() {
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
    return false;

  for (int t = 0; t < numTags; t++) {
    bool match = true;
    for (int i = 0; i < 4; i++) {
      if (mfrc522.uid.uidByte[i] != validTags[t][i]) {
        match = false;
        break;
      }
    }
    if (match) {
      Serial.println("RFID matched");
      mfrc522.PICC_HaltA();
      return true;
    }
  }
  mfrc522.PICC_HaltA();
  return false;
}

// EEPROM Functions ----------
void savePINToEEPROM(char* pin) {
  Wire.beginTransmission(EEPROM_ADDR);
  Wire.write((EEPROM_PIN_ADDR >> 8) & 0xFF);
  Wire.write(EEPROM_PIN_ADDR & 0xFF);
  for (int i = 0; i < 4; i++) {
    Wire.write(pin[i]);
  }
  Wire.endTransmission();
  delay(10);
  memcpy(storedPIN, pin, 5);
}

void loadPINFromEEPROM() {
  Wire.beginTransmission(EEPROM_ADDR);
  Wire.write((EEPROM_PIN_ADDR >> 8) & 0xFF);
  Wire.write(EEPROM_PIN_ADDR & 0xFF);
  Wire.endTransmission();

  Wire.requestFrom(EEPROM_ADDR, 4);
  for (int i = 0; i < 4; i++) {
    if (Wire.available()) {
      storedPIN[i] = Wire.read();
    }
  }
  storedPIN[4] = '\0';
  Serial.print("Stored PIN: ");
  Serial.println(storedPIN);
}

// Reset Mode ----------
void enterResetMode() {
  Serial.println("Resetting PIN. Enter new 4-digit PIN:");
  char newPIN[5];
  int idx = 0;
  while (idx < 4) {
    char k = keypad.getKey();
    if (k && isDigit(k)) {
      newPIN[idx++] = k;
      Serial.print('*'); // mask entry
    }
  }
  newPIN[4] = '\0';
  savePINToEEPROM(newPIN);
  Serial.println("\nNew PIN saved!");