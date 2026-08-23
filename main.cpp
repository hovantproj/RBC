#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <vector>
using namespace std;

#define LED_PIN 13

// --- Pin Configuration ---
// Servo pin configuration
#define RST_PIN 
#define SS_PIN  
#define GRIP_SERVO_PIN 3 // attach grip servo object to this
#define RAISE_SERVO_PIN 11 // attach raise servo object to this
// Motor controls for Motor A1 and A2 (wired together)
const int in1 = 10; // in1 pin
const int in2 = 9; // in2 pin
// Motor controls for Motor B1 and B2 (wired together)
const int in3 = 6; // in3 pin
const int in4 = 5; // in4 pin

// --- Motor Setup ---

// Function prototypes for movement
void forward(int ms);
void backward(int ms);
void turnRight();
void turnLeft();
void stop();

// --- Servo Setup ---
Servo gripServo;  // create servo object to control the servo responsible for opening and closing of the gripper
Servo raiseServo; // create servo object to control the raising and lowering of the gripper via a pulley

int gripServoPos; // Current positions
int raiseServoPos;

const int stepSize = 5;

const int minServo = 0;
const int maxGripPos = 75; // 75 is open, 0 is closed
const int maxRaisePos = 95; // Max for raise

void grab();
void release();

void raise();
void lower();

// --- RFID setup ---
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

// Function prototypes
bool readBlockData(byte blockAddr, byte trailerBlock, byte* outputBuffer);
int findSectorTrailer(int blockToRead);

// --- Pathfinding ---
vector<int> pos = {0, 0}; // starts at (0,0)
vector<int> nextPos = {0, 0};
vector<int> currentAnimal = {0,0}; // position of next animal to retrieve
int direction = 0; // NORTH = 0, EAST = 1, SOUTH = 2, WEST = 3
bool travellingToAnimal = false;
bool haveAnimal = false;

// function prototypes
void determineNextPos();
void orient();
void reOrient();

void setup() {
    // --- Testing ---
    // initialize digital pin LED_PIN as an output for testing.
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
    while (!Serial); // Wait for Serial Monitor (for ATmega32U4 boards like Leonardo/Micro)

    SPI.begin();
    mfrc522.PCD_Init();
    delay(4);

    // Set standard factory Key A: 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
    for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
    }

    // --- Motors ---
    // Set all motor control pins to output
    pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);
    pinMode(in3, OUTPUT);
    pinMode(in4, OUTPUT);

    delay(10000);
}

// obtain animal coordinates 

    // while not yet 2 friendly animals
    // decide which one is closest

    // travel to it (orient -> travel along x -> turn -> travel along y)
        // along the way, verify direction by comparing current coordinate to last coordinate

    // once in coordinate space of purple tags, verify animal friendliness
        // if friendly, grab it
        // if not friendly, idk just backtrack

    // we might wna pivot to instead grabbing both in one go...
    // somehow backtrack... (should i just use the same algorithm? or try to go backwards to avoid turns...)
        // if i've already gotten one friendly animal, maybe try not to knock over the other one since it needs to be on the ground at the end

    // finish while loop, stop for 30s back at the start

void loop() {
    // obtain animal coordinates from start tag
    byte blockData[16];
    byte blockToRead = ;
    byte sectorTrailer = findSectorTrailer(blockToRead);
}


void forward(int ms) {
    analogWrite(in1, 50);
    analogWrite(in2, 0);
    analogWrite(in3, 60);
    analogWrite(in4, 0);
    delay(ms);
    return;
}

void backward(int ms) {
    analogWrite(in1, 0);
    analogWrite(in2, 50);
    analogWrite(in3, 0);
    analogWrite(in4, 60);
    delay(ms);
    return;
}

void turnRight() {
    analogWrite(in1, 100);
    analogWrite(in2, 0);
    analogWrite(in3, 0);
    analogWrite(in4, 120);
    delay(550);
    return;
}

void turnLeft() {
    analogWrite(in1, 0);
    analogWrite(in2, 100);
    analogWrite(in3, 110);
    analogWrite(in4, 0);
    delay(550);
    return;
}

void stop() {
    analogWrite(in1, 0);
    analogWrite(in2, 0);
    analogWrite(in3, 0);
    analogWrite(in4, 0);
    return;
}

/**
 * Finds the sector trailer of the sector any block is in
 * 
 * @param blockToRead   The block index we want to access (0-63 on a 1K card)
 * @return              The index of the sector trailer
 */
int findSectorTrailer(int blockToRead) {
  return (3-blockToRead%4)+blockToRead;
}

/**
 * Reads a 16-byte data block from a MIFARE RFID tag.
 * 
 * @param blockAddr     The block index to read (0-63 on a 1K card)
 * @param trailerBlock  The sector trailer index for authentication
 * @param outputBuffer  Pointer to a 16-byte array to store output
 * @return              True if read succeeded, False otherwise
 */
bool readBlockData(byte blockAddr, byte trailerBlock, byte* outputBuffer) {
  // Check if a card is present and can be read
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return false;
  }

  byte rawBuffer[18]; // MIFARE_Read requires at least 18 bytes (16 data + 2 CRC)
  byte bufferSize = sizeof(rawBuffer);

  // 1. Authenticate sector access
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

  // 2. Read raw block data
  status = mfrc522.MIFARE_Read(blockAddr, rawBuffer, &bufferSize);

  // 3. Reset card & reader state (critical for reliability)
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  if (status != MFRC522::STATUS_OK) {
    return false;
  }

  // 4. Copy payload into caller's array
  memcpy(outputBuffer, rawBuffer, 16);
  return true;
}

void determineNextPos() {
    // if have animal, need to navigate to 0,0
    if (haveAnimal) {
        // if x coordinates don't match, travel to 0,y
        if (pos[0] != 0) {
            nextPos[0] = 0;
            nextPos[1] = pos[1];
        // if y coordinates don't match travel to 0,0
        } else if (pos[0] != 0) {
            nextPos[0] = 0;
            nextPos[1] = 0;
        }

        return;
    // otherwise travelling to animal: if x coordinates don't match, travel to matching X
    } else if (pos[0] != currentAnimal[0]) {
        // if y coordinate is already matching, we only need to travel to 1 less to pick up the animal
        if (pos[1] == currentAnimal[1]) {
            nextPos[0] = currentAnimal[0] - 1;
            nextPos[1] = pos[1];
            travellingToAnimal = true
        } else {
            nextPos[0] = currentAnimal[0];
            nextPos[1] = pos[1];
            travellingToAnimal = false
        }
    // if y coordinates don't match, travel to 1 less to pick it up
    } else if (pos[1] != currentAnimal[1]) {
        nextPos[0] = pos[0];
        nextPos[1] = currentAnimal[1] - 1;
        travellingToAnimal = true;
    }
    return;
}

void orient() {
    int diff = 0;
    int desired = 0;
    // heading east
    if (pos[0] < nextPos[0]) {
        desired = 1;
    // heading west
    } else if (pos[0] > nextPos[0]) {
        desired = 3;
    // heading north
    } else if (pos[1] < nextPos[1]) {
        desired = 0;
    // heading south
    } else if (pos[1] > nextPos[1]) {
        desired = 2;
    }

    diff = (desired - direction + 4) % 4;
    switch (diff) {
        case 0: break;
        case 1: turnRight(); break;
        case 2: turnRight(); turnRight(); break;
        case 3: turnLeft(); break;
    }
    direction = desired;
    return;
}

void toggle_servo(Servo &servo, int &currentPos, int minPos, int maxPos) {
    if (currentPos == minPos) {
        for (int pos = minPos; pos <= maxPos; pos += stepSize) {
            servo.write(pos);
        }
        currentPos = maxPos;
    }

    else {
        for (int pos = maxPos; pos >= minPos; pos -= stepSize) {
            servo.write(pos);
        }
        currentPos = minPos;
    }
}

void close_grip() {
    gripServo.write(minServo);
    gripServoPos = minServo;
}

void open_grip() {
    gripServo.write(maxGripPos);
    gripServoPos = maxGripPos;
}

void raise() {
    raiseServo.write(maxRaisePos);
    raiseServoPos = maxRaisePos; // For debugging
}

void lower() {
    raiseServo.write(minServo);
    raiseServo = minServo;
}
