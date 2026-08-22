#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <vector>

void loop() {
  byte cardData[16];
  
  // Sector 11: Block 44 data, Block 47
  byte blockToRead = 60;
  byte sectorTrailer = findSectorTrailer(blockToRead);

  if (readBlockData(blockToRead, sectorTrailer, cardData)) {
    pos = cardData;				// try experimenting with this value and rerun the code
  }
}

std::vector<int> getCoordinates() {
    vector<int> coordinates;

    // 
}