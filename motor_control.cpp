const int dirA = 7; // in1 pin
const int speedA = 3; // ena pin

const int dirB = 9
const int speedB = 5;

void forward(int spd, int *motors)
{
    digitalWrite(dirA, HIGH); // Forward
    analogWrite(speedA, spd);
}

void reverse()
{
    digitalWrite(dirA, LOW); // Reverse
    analogWrite(speedA, spd);
}

void stop()
{
    analogWrite(speedA, 0); // Stop
}

