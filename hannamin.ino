/* Code for the Hannamin. This is meant to run on a PJRC Teensy 4.0 microcontroller. */

const int LED = 13;

void setup() {
  // TODO

  pinMode(LED, OUTPUT);
}

void loop() {
  // TODO

  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);
  delay(1000);
}
