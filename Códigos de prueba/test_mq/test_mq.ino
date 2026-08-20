// ST - TP DOG - Grupo 8 - Santiago Fernández, Paulina Gonzalez y Avner Yunik
// Código para probar sensor de gas, simulado con un potenciómetro

#define GAS 33              // pin analógico GPIO33 para el sensor de gas (simulado con un pote)
#define TIEMPO_LECTURA 500  // 500 milisegundos

void setup() {
  Serial.begin(115200);
  pinMode(GAS, INPUT);  // definir pin
  Serial.println("Simulando mq flying fish.");
  Serial.println("Porcentaje más alto, mayor concentración de gas.");

}

void loop() {

  tiempoActual = millis();  // lee el tiempo con millis(); para que no sea bloqueante

  if (tiempoActual - tiempoAnterior >= TIEMPO_LECTURA) {
    tiempoAnterior = tiempoActual;  // guardar el último tiempo de lectura.

    int lectura = analogRead(LDR);
    // int mapeoLectura = (lectura / 4095) * 100;  // 4095 es el valor máximo que puede leer el ADC del ESP32
    int mapeoLectura = map(lectura, 0, 4095, 0, 100);
    Serial.print("Lectura GAS sin mapear: ");
    Serial.println(lectura);
    Serial.print("Lectura GAS en porcentaje: ");
    Serial.println(mapeoLectura);
  }

}
