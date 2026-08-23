#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <math.h>

#define LED_PIN 13

// --- Pin Configuration ---

// RFID pin configuration
#define RST_PIN 4
#define SS_PIN 2

// Servo pin configuration
#define GRIP_SERVO_PIN 3
#define RAISE_SERVO_PIN 11

// Motor controls for Motor A1 and A2 (wired together)
const int in1 = 10;
const int in2 = 9;

// Motor controls for Motor B1 and B2 (wired together)
const int in3 = 6;
const int in4 = 5;

// --- Motor Setup ---

void forward(int ms);
void backward(int ms);
void turnRight();
void turnLeft();
void stop();

// --- Servo Setup ---

Servo gripServo;
Servo raiseServo;

int gripServoPos;
int raiseServoPos;

const int stepSize = 5;
const int minServo = 0;
const int maxGripPos = 75;   // 75 is open, 0 is closed
const int maxRaisePos = 95;  // Max for raise

void close_grip();
void open_grip();
void raise();
void lower();

// --- RFID Setup ---

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

bool readBlockData(byte blockAddr, byte trailerBlock, byte* outputBuffer);
int findSectorTrailer(int blockToRead);

// --- Pathfinding ---

// Position arrays are [x, y]
int pos[2] = {0, 0};
int nextPos[2] = {0, 0};
int currentAnimal[2] = {0, 0};

int direction = 0;  // NORTH = 0, EAST = 1, SOUTH = 2, WEST = 3

bool travellingToAnimal = false;
bool haveAnimal = false;

int animalsSaved = 0;
int animalsChecked = 0;
int closest = 0;

// There are at most 3 animal coordinates read from blocks 52, 53 and 54.
// Each row stores {x, y}.
const int MAX_ANIMALS = 3;
int animalPosArray[MAX_ANIMALS][2];
int animalCount = 0;

byte coordBlockAddr = 56;

void determineNextPos();
void orient();
float distanceTo(const int startPos[2], const int endPos[2]);
void determineFriendliness();
void removeAnimal(int index);

void setup() {
    // --- Testing ---
    pinMode(LED_PIN, OUTPUT);

    // --- Servo ---
    gripServo.attach(GRIP_SERVO_PIN);
    raiseServo.attach(RAISE_SERVO_PIN);

    gripServoPos = maxGripPos;
    raiseServoPos = maxRaisePos;

    gripServo.write(gripServoPos);
    raiseServo.write(raiseServoPos);

    // --- RFID ---
    Serial.begin(9600);

    // Keep this only if using a Leonardo/Micro-style board.
    // On an Uno/Nano this is usually unnecessary.
    // while (!Serial);

    SPI.begin();
    mfrc522.PCD_Init();
    delay(4);

    // Standard factory Key A: FF FF FF FF FF FF
    for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
    }

    // --- Motors ---
    pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);
    pinMode(in3, OUTPUT);
    pinMode(in4, OUTPUT);
    stop();
    delay(1000);
}

void loop() {
    raise();
    lower();
    stop();
    digitalWrite(LED_PIN, LOW);

    // Obtain animal coordinates from start tag.
    byte blockData[16];

    animalCount = 0;

    for (int i = 52; i < 55 && animalCount < MAX_ANIMALS; i++) {
        byte blockToRead = (byte)i;
        byte sectorTrailer = (byte)findSectorTrailer(blockToRead);

        while (!readBlockData(blockToRead, sectorTrailer, blockData)) {
            delay(500);
        }
        animalPosArray[animalCount][0] = (int)blockData[3];
        animalPosArray[animalCount][1] = (int)blockData[11];
        animalCount++;
    }

    // Keep searching for animals until quota is met.
    while (animalsSaved < 2 && animalsChecked < 3 && animalCount > 0) {
        int currentAnimalsChecked = animalsChecked;

        // Determine closest animal.
        float bestDistance = 100.0;

        for (int i = 0; i < animalCount; i++) {
            float distance = distanceTo(pos, animalPosArray[i]);

            if (distance < bestDistance) {
                bestDistance = distance;
                closest = i;
            }
        }

        currentAnimal[0] = animalPosArray[closest][0];
        currentAnimal[1] = animalPosArray[closest][1];

        // Until we check an animal, keep moving towards it.
        while (currentAnimalsChecked == animalsChecked) {
            determineNextPos();

            // determineNextPos() may inspect the animal and increment
            // animalsChecked once the robot reaches the pickup position.
            if (currentAnimalsChecked != animalsChecked) {
                break;
            }

            orient();

            // Head towards nextPos slowly, checking RFID coordinates.
            while (pos[0] != nextPos[0] || pos[1] != nextPos[1]) {
                forward(10);

                if (readBlockData(coordBlockAddr, coordBlockAddr, blockData)) {
                    pos[0] = (int)blockData[3];
                    pos[1] = (int)blockData[11];
                }
            }
        }
    }

    // Head back to (2, 2) and drop animals.
    while (pos[0] != 2 || pos[1] != 2) {
        // Once animal searching is finished, make the target the start/drop point.
        nextPos[0] = 2;
        nextPos[1] = 2;

        orient();

        while (pos[0] != nextPos[0] || pos[1] != nextPos[1]) {
            forward(10);

            if (readBlockData(coordBlockAddr, coordBlockAddr, blockData)) {
                digitalWrite(LED_PIN, HIGH);
                pos[0] = (int)blockData[3];
                pos[1] = (int)blockData[11];
            }
        }

        digitalWrite(LED_PIN, LOW);
    }

    stop();

    // End program.
    while (true) {
    }
}

// --- Motor Functions ---

void forward(int ms) {
    analogWrite(in1, 10);
    analogWrite(in2, 0);
    analogWrite(in3, 12);
    analogWrite(in4, 0);

    delay(ms);

    stop();
}

void backward(int ms) {
    analogWrite(in1, 0);
    analogWrite(in2, 10);
    analogWrite(in3, 0);
    analogWrite(in4, 12);
    
    delay(ms);

    stop();
}

void turnRight() {
    analogWrite(in1, 25);
    analogWrite(in2, 0);
    analogWrite(in3, 0);
    analogWrite(in4, 30);

    delay(2200);

    stop();
}

void turnLeft() {
    analogWrite(in1, 0);
    analogWrite(in2, 25);
    analogWrite(in3, 28);
    analogWrite(in4, 0);

    delay(2200);

    stop();
}

void stop() {
    analogWrite(in1, 0);
    analogWrite(in2, 0);
    analogWrite(in3, 0);
    analogWrite(in4, 0);
}

/**
 * Finds the sector trailer of the sector any block is in.
 *
 * @param blockToRead The block index we want to access (0-63 on a 1K card)
 * @return The index of the sector trailer
 */
int findSectorTrailer(int blockToRead) {
    return (3 - blockToRead % 4) + blockToRead;
}

/**
 * Reads a 16-byte data block from a MIFARE RFID tag.
 *
 * @param blockAddr The block index to read (0-63 on a 1K card)
 * @param trailerBlock The sector trailer index for authentication
 * @param outputBuffer Pointer to a 16-byte array to store output
 * @return true if read succeeded, false otherwise
 */
bool readBlockData(byte blockAddr, byte trailerBlock, byte* outputBuffer) {
    // Check if a card is present and can be read.
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
        digitalWrite(LED_PIN, LOW);
        return false;
    }

    digitalWrite(LED_PIN, HIGH);
    // MIFARE_Read requires at least 18 bytes:
    // 16 data bytes + 2 CRC bytes.
    byte rawBuffer[18];
    byte bufferSize = sizeof(rawBuffer);

    // Authenticate sector access.
    MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
        MFRC522::PICC_CMD_MF_AUTH_KEY_A,
        trailerBlock,
        &key,
        &(mfrc522.uid)
    );

    if (status != MFRC522::STATUS_OK) {
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        return false;
    }

    // Read raw block data.
    status = mfrc522.MIFARE_Read(blockAddr, rawBuffer, &bufferSize);

    // Reset card and reader state.
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();

    if (status != MFRC522::STATUS_OK) {
        return false;
    }

    // Copy the 16-byte payload into the caller's array.
    memcpy(outputBuffer, rawBuffer, 16);
    return true;
}

// --- Pathfinding Functions ---

void determineNextPos() {
    // If travellingToAnimal is true and this gets called again,
    // the robot has reached the square beside the animal.
    if (travellingToAnimal) {
        travellingToAnimal = false;
        determineFriendliness();
        return;
    }

    // If carrying an animal, navigate to (2, 2).
    if (haveAnimal) {
        if (pos[0] != 2) {
            nextPos[0] = 2;
            nextPos[1] = pos[1];
        } else if (pos[1] != 2) {
            nextPos[0] = 2;
            nextPos[1] = 2;
        }

        return;
    }

    // Otherwise, travel toward the current animal.
    if (pos[0] != currentAnimal[0]) {
        // If y already matches, stop one x-coordinate before the animal.
        if (pos[1] == currentAnimal[1]) {
            nextPos[0] = currentAnimal[0] - 1;
            nextPos[1] = pos[1];
            travellingToAnimal = true;
        } else {
            nextPos[0] = currentAnimal[0];
            nextPos[1] = pos[1];
            travellingToAnimal = false;
        }
    } else if (pos[1] != currentAnimal[1]) {
        // Stop one y-coordinate before the animal.
        nextPos[0] = pos[0];
        nextPos[1] = currentAnimal[1] - 1;
        travellingToAnimal = true;
    }
}

void orient() {
    int desired = direction;

    // Heading east.
    if (pos[0] < nextPos[0]) {
        desired = 1;
    }
    // Heading west.
    else if (pos[0] > nextPos[0]) {
        desired = 3;
    }
    // Heading north.
    else if (pos[1] < nextPos[1]) {
        desired = 0;
    }
    // Heading south.
    else if (pos[1] > nextPos[1]) {
        desired = 2;
    }

    int diff = (desired - direction + 4) % 4;

    switch (diff) {
        case 0:
            break;

        case 1:
            turnRight();
            break;

        case 2:
            turnRight();
            turnRight();
            break;

        case 3:
            turnLeft();
            break;
    }

    direction = desired;
}

// --- Servo Functions ---

void close_grip() {
    for (int servoPos = gripServoPos; servoPos >= minServo; servoPos -= stepSize) {
        gripServo.write(servoPos);
        delay(20);
    }

    gripServo.write(minServo);
    gripServoPos = minServo;
}

void open_grip() {
    for (int servoPos = gripServoPos; servoPos <= maxGripPos; servoPos += stepSize) {
        gripServo.write(servoPos);
        delay(20);
    }

    gripServo.write(maxGripPos);
    gripServoPos = maxGripPos;
}

void raise() {
    for (int servoPos = raiseServoPos; servoPos <= maxRaisePos; servoPos += stepSize) {
        raiseServo.write(servoPos);
        delay(20);
    }

    raiseServo.write(maxRaisePos);
    raiseServoPos = maxRaisePos;
}

void lower() {
    for (int servoPos = raiseServoPos; servoPos >= minServo; servoPos -= stepSize) {
        raiseServo.write(servoPos);
        delay(20);
    }

    raiseServo.write(minServo);
    raiseServoPos = minServo;
}

/**
 * Calculates the Euclidean distance between two positions.
 *
 * @param startPos Array {x, y} of starting position
 * @param endPos Array {x, y} of ending position
 * @return Distance between the two positions
 */
float distanceTo(const int startPos[2], const int endPos[2]) {
    long xDiff = startPos[0] - endPos[0];
    long yDiff = startPos[1] - endPos[1];

    return sqrt((float)(xDiff * xDiff + yDiff * yDiff));
}

/**
 * Removes one animal coordinate from the fixed-size array by shifting
 * all later coordinates one position to the left.
 */
void removeAnimal(int index) {
    if (index < 0 || index >= animalCount) {
        return;
    }

    for (int i = index; i < animalCount - 1; i++) {
        animalPosArray[i][0] = animalPosArray[i + 1][0];
        animalPosArray[i][1] = animalPosArray[i + 1][1];
    }

    animalCount--;
}

void determineFriendliness() {
    // Only called when the robot is in the ring around an animal.
    byte blockData[16];

    byte blockToRead = 57;
    byte sectorTrailer = (byte)findSectorTrailer(blockToRead);

    if (!readBlockData(blockToRead, sectorTrailer, blockData)) {
        return;
    }

    byte animalType = blockData[15] & 0x0F;

    if (animalType == 3) {
        // Friendly animal.
        animalsChecked++;

        removeAnimal(closest);

        if (animalsSaved == 0) {
            // Sequence to pick up first animal.
            // Add your gripper/lift sequence here.
            // Example:
            // open_grip();
            // lower();
            // close_grip();
            // raise();
            open_grip();
            lower();
            close_grip();
            raise();

            animalsSaved++;
        } else {
            // Sequence to pick up second animal.
            // Add your gripper/lift sequence here.
            backward(300);
            lower();
            open_grip();
            forward(300);
            close_grip();
            raise();

            animalsSaved++;
        }

        haveAnimal = true;
    }
    else if (animalType == 0) {
        // Error sequence.
        // Example: spin / blink LED.
    }
    else if (animalType == 2) {
        // Unfriendly animal.
        animalsChecked++;
        removeAnimal(closest);
    }
}