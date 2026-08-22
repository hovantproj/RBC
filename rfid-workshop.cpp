#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

// --- Pin Configuration ---
#define RST_PIN 9
#define SS_PIN  10
#define SERVO_PIN 8

// --- RFID Setup ---
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

// --- Servo setup ---
Servo myservo;  // create servo object to control a servo

// Function prototype
bool readBlockData(byte blockAddr, byte trailerBlock, byte* outputBuffer);
int pos = 0;    // variable to store the servo position

int findSectorTrailer(int blockToRead) {
  return (3-blockToRead%4)+blockToRead;
}

void setup() {
  myservo.attach(8);  // attaches the servo on pin 9 to the servo object

  Serial.begin(9600);
  while (!Serial); // Wait for Serial Monitor (for ATmega32U4 boards like Leonardo/Micro)

  SPI.begin();
  mfrc522.PCD_Init();
  delay(4);

  // Set standard factory Key A: 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

}

void loop() {
  byte cardData[16];
  
  // Sector 11: Block 44 data, Block 47
  byte blockToRead = 60;
  byte sectorTrailer = findSectorTrailer(blockToRead);

  if (readBlockData(blockToRead, sectorTrailer, cardData)) {
    pos = cardData;				// try experimenting with this value and rerun the code
    myservo.write(pos);              // tell servo to go to position in pos
  }
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
