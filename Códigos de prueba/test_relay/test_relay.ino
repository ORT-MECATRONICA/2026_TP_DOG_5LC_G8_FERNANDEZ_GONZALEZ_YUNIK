// ST - TP DOG - Grupo 8 - Santiago Fernández, Paulina Gonzalez y Avner Yunik
// Código para probar relé. Prende led cuando relé encendido

#define RELAY_PIN 13  // Pin Relay Input
#define LED_PIN 16    // Pin Led
#define INTERVAL 1000
unsigned long previousMillis = 0;

void setup() {
  pinMode(RELAY_PIN, OUTPUT);  // Set the relay pin as an output
  pinMode(LED_PIN, OUTPUT);    // Set the relay pin as an output
  Serial.begin(115200);
}

void loop() {

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= INTERVAL) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (RELAY_PIN == LOW) {
      digitalWrite(RELAY_PIN, HIGH);
      digitalWrite(LED_PIN, HIGH);

    } else {
      digitalWrite(RELAY_PIN, LOW);
      digitalWrite(LED_PIN, HIGH);
    }
  }
}
