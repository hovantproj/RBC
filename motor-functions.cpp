#define LED_PIN 13

// Motor controls for Motor A1 and A2 (wired together)
const int in1 = 10; // in1 pin
const int in2 = 9; // in2 pin

// Motor controls for Motor B1 and B2 (wired together)
const int in3 = 6; // in3 pin
const int in4 = 5; // in4 pin

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

void setup() {
    // initialize digital pin LED_PIN as an output for testing.
    pinMode(LED_PIN, OUTPUT);

    // Set all motor control pins to output
    pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);
    pinMode(in3, OUTPUT);
    pinMode(in4, OUTPUT);

    delay(1000);
}

void loop() {
    // DRIVING TEST

    digitalWrite(LED_PIN, HIGH);

    // forward
    forward(1000);
    digitalWrite(LED_PIN, LOW);

    // 90 degree to the right
    turnRight();
    digitalWrite(LED_PIN, HIGH);

    // forward
    forward(1000);
    digitalWrite(LED_PIN, LOW);

    // 90 degree to the left
    turnLeft();
    digitalWrite(LED_PIN, HIGH);

    // backwards
    backward(1000);
    digitalWrite(LED_PIN, LOW);

    // 3 x 90 degree to the right
    turnRight();
    delay(100);
    digitalWrite(LED_PIN, HIGH);
    turnRight();
    delay(100);
    digitalWrite(LED_PIN, LOW);
    turnRight();
    delay(100);
    digitalWrite(LED_PIN, HIGH);

    // forward
    forward(1000);
    digitalWrite(LED_PIN, LOW);

    stop();
    delay(10000);
}