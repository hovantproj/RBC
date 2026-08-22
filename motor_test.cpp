#define LED_PIN 13

// Motor controls for Motor A1 and A2 (wired together)
const int in1 = 10; // in1 pin
const int in2 = 9; // in2 pin

// Motor controls for Motor B1 and B2 (wired together)
const int in3 = 6; // in3 pin
const int in4 = 5; // in4 pin

void setup() {
    // initialize digital pin LED_PIN as an output for testing.
    pinMode(LED_PIN, OUTPUT);

    // Set all motor control pins to output
    pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);
    pinMode(in3, OUTPUT);
    pinMode(in4, OUTPUT);

    delay(1)
}

void loop() {
    digitalWrite(LED_PIN , HIGH);

    // Drive forward
    analogueWrite(in1, 100);
    analogueWrite(in2, 0);
    analogueWrite(in3, 100);
    analogueWrite(in4, 0);

    '''
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
    '''
    delay(1000);

    digitalWrite(LED_PIN , LOW);

    // Drive backwards
    analogueWrite(in1, 0);
    analogueWrite(in2, 100);
    analogueWrite(in3, 0);
    analogueWrite(in4, 100);
    delay(1000);

    // Turn one way

    // Turn the other way
}