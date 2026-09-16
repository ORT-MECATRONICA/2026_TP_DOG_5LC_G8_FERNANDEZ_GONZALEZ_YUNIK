// ST - TP DOG - Grupo 8 - Santiago Fernández, Paulina Gonzalez y Avner Yunik
// código de prueba para pulsador y led

#define LED_PIN 16
#define PULSADOR_PIN 17
#define DEBOUNCE 20

int estadoLed = LOW; 
int estadoBoton;

unsigned long tiempoAnterior = 0; 



void setup() {
  // set the digital pin as output:
  pinMode(LED_PIN, OUTPUT);
  pinMode(PULSADOR_PIN, INPUT);
}

void loop() {
  
  unsigned long tiempoActual = millis();

  if (tiempoActual - tiempoAnterior >= DEBOUNCE && digitalRead(PULSADOR_PIN) == LOW) {
    tiempoAnterior = tiempoActual;


    if (estadoLed == LOW && tiempoActual - tiempoAnterior >= DEBOUNCE && digitalRead(PULSADOR_PIN) == HIGH) {
      digitalWrite(LED_PIN, HIGH);
      tiempoAnterior = tiempoActual;
    } else {
      digitalWrite(LED_PIN, LOW);
      tiempoAnterior = tiempoActual;
    }

    
  }
}
