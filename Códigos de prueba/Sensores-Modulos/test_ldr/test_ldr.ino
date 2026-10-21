// ST - TP DOG - Grupo 8 - Santiago Fernández, Paulina Gonzalez y Avner Yunik
// Código para probar LDR y envía resultado (mapeado) por puerto serie.

#define LDR 32              // pin analógico GPIO32 para el LDR
#define LED 33  // pin led
#define TIEMPO_LECTURA 20  // 20 milisegundos

// Variables para millis
unsigned long tiempoAnterior = 0;  // Tiempo anterior
unsigned long tiempoActual = 0;    // Tiempo actual
void setup() {
  Serial.begin(115200);
  pinMode(LDR, INPUT);  // definir pin
  pinMode(LED, OUTPUT); // pin led
}

void loop() {

  tiempoActual = millis();  // lee el tiempo con millis(); para que no sea bloqueante

  if (tiempoActual - tiempoAnterior >= TIEMPO_LECTURA) {
    tiempoAnterior = tiempoActual;  // guardar el último tiempo de lectura.

    int lectura = analogRead(LDR);
    // int mapeoLectura = (lectura / 4095) * 100;  // 4095 es el valor máximo que puede leer el ADC del ESP32
    int mapeoLectura = map(lectura, 0, 4095, 0, 100);
    Serial.print("Lectura LDR: ");
    Serial.println(lectura);
    Serial.print("Lectura LDR en porcentaje: ");
    Serial.println(mapeoLectura);
    if (mapeoLectura < 50)
    {
      digitalWrite(LED,LOW);
    } else {
      digitalWrite(LED,HIGH);
    }
    
    
  }
}
