// ST - TP DOG - Grupo 8 - Santiago Fernández, Paulina Gonzalez y Avner Yunik
// código de prueba para pulsador y led

#define BUTTON_PIN_1 1
#define BUTTON_PIN_2 3
#define BUTTON_PIN_3 5
#define BUTTON_PIN_4 4
#define BUTTON_PIN_5 21

#define LED_PIN 23
#define DEBOUNCE 20

int estadoLed = LOW; 
int estadoBoton;

unsigned long tiempoAnterior = 0; 



void setup() {
  // set the digital pin as output:
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN_1, INPUT_PULLUP);
}

void loop() {
  
  unsigned long tiempoActual = millis();

  if (tiempoActual - tiempoAnterior >= DEBOUNCE && digitalRead(BUTTON_PIN_1) == LOW) {
    tiempoAnterior = tiempoActual;


    if (estadoLed == LOW && tiempoActual - tiempoAnterior >= DEBOUNCE && digitalRead(BUTTON_PIN_1) == HIGH) {
      digitalWrite(LED_PIN, HIGH);
      tiempoAnterior = tiempoActual;
    } else {
      digitalWrite(LED_PIN, LOW);
      tiempoAnterior = tiempoActual;
    }

    
  }
}
