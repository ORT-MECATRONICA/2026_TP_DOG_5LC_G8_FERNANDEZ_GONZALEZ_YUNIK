// ST - TP DOG - Grupo 8 - Santiago Fernández, Paulina Gonzalez y Avner Yunik
// código de prueba opto PC817C

#define PIN_OPTO 4  // Pin GPIO conectado al colector del PC817C
#define TIEMPO_LECTURA 100


unsigned long tiempoAnterior = millis();

void setup() {
  Serial.begin(115200);
  // Configuramos el pin como entrada (el pull-up es externo de 10k)
  pinMode(PIN_OPTO, INPUT);
}

void loop() {


  unsigned long tiempoActual = millis();

  if (tiempoActual - tiempoAnterior >= TIEMPO_LECTURA) {

    tiempoAnterior = tiempoActual;

    int estado = digitalRead(PIN_OPTO);

    // Lógica inversa: LOW significa que hay 12V en la entrada
    if (estado == LOW) {
      Serial.println("Señal de 12V DETECTADA");
    } else {
      Serial.println("Sin señal (0V)");
    }
  }
}
