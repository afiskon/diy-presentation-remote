#include <Arduino.h>
#include <Keyboard.h>

#define DATA_PIN A0
#define PACKAGE_SIZE 25 // # of long/short signals

#define COMMAND_DOWN 1
#define COMMAND_STOP 2
#define COMMAND_UP 3
#define COMMAND_LOCK 4

char package[PACKAGE_SIZE];
const char dipCode[] = { 1, 1, 0, 1, 0, 0, 1, 0 };

// Wait until next package
// If we see a low voltage for a long time we consider it a sync
void sync() {
    unsigned long startTime, endTime;
    do {
        startTime = millis();
        while(analogRead(DATA_PIN) < 600) {
            delayMicroseconds(3);
        }
        endTime = millis();
    } while(endTime - startTime < 7);
}

// Try to receive a new package into the package[] array
// Returns true on success and false on error
bool readPackage() {
    unsigned long startTime, highStart, highEnd;
    bool firstLoop = true;
    int delayCtr;

    startTime = millis();
    for(int sigNum = 0; sigNum < PACKAGE_SIZE; sigNum++) {
        if(!firstLoop) {
            // wait until high signal
            delayCtr = 0;
            while((analogRead(DATA_PIN) < 300) && (delayCtr < 500/3)) {
                delayMicroseconds(3);
                delayCtr++;
            }
            if(delayCtr == 500/3)
                return false; // too long low signal
        }

        firstLoop = false;

        highStart = micros();

        // wait until low signal
        delayCtr = 0;
        while((analogRead(DATA_PIN) >= 300) && (delayCtr < 1100/3)) {
            delayMicroseconds(3);
            delayCtr++;
        }
        if(delayCtr == 1100/3)
            return false; // too long high signal

        highEnd = micros();

        // long: 1000 us, short: 330 us
        package[sigNum] = highEnd - highStart > 500 ? 1 : 0;
    }

    if(millis() - startTime > 33)
        return false; // package too long

    return true; // package received
}

// Check if package has a valid format
bool checkPackage() {
    bool hasZeroes = false;
    bool hasOnes = false;

    if(package[PACKAGE_SIZE - 1] != 0)
        return false;

    for(int i = 0; i < PACKAGE_SIZE/2; i++) {
        if(package[i*2] != package[i*2 + 1])
            return false;

        hasZeroes = hasZeroes || (package[i*2] == 0);
        hasOnes = hasOnes || (package[i*2] == 1);
    }

    return hasZeroes && hasOnes;
}

// Check if DIP code is correct
bool checkDipCode() {
    for(uint16_t i = 0; i < sizeof(dipCode); i++)
        if(package[i*2] != dipCode[i])
            return false;

    return true;
}

// Get command number or 0 in case of error
int readCommand() {
    uint16_t i = sizeof(dipCode);
    int cmdNum = 1;
    for(; i < PACKAGE_SIZE/2; i++, cmdNum++) {
        if(package[i*2] == 1)
            return cmdNum;
    }
    return 0;
}

void setup() {
    Serial.begin(9600);
    Keyboard.begin();
}

void loop() {
    sync();
    if(readPackage() && checkPackage() && checkDipCode()) {
        Serial.print("Good: ");
        for(int i = 0; i < PACKAGE_SIZE; i++)
            if(package[i] == 1)
                Serial.print("1");
            else
                Serial.print("0");

        int command = readCommand();
        Serial.println(", command: " + String(command));

        if(command == COMMAND_DOWN) {
            Keyboard.press(KEY_RIGHT_ARROW);
            delay(150);
        } else if(command == COMMAND_UP) {
            Keyboard.press(KEY_LEFT_ARROW);
            delay(150);
        }

        Keyboard.releaseAll();
    }
}
