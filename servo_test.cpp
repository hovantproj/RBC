Servo gripServo;  // create servo object to control the servo responsible for opening and closing of the gripper
Servo raiseServo; // create servo object to control the raising and lowering of the gripper via a pulley

int gripServoPos; // Current positions
int raiseServoPos;

const int stepSize = 5;

const int minServo = 0;
const int maxGripPos = 75; // 75 is open, 0 is closed
const int maxRaisePos = 95; // Max for raise

void close_grip();
void open_grip();

void raise();
void lower();

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
    raiseServoPos = minServo;
}