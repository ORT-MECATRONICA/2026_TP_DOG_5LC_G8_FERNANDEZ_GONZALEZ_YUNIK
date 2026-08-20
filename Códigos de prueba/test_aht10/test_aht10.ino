// ST - TP DOG - Grupo 8 - Santiago Fernández, Paulina Gonzalez y Avner Yunik
// Código para probar aht10 y envía resultados por puerto serie.


#include <Wire.h>
#include <Adafruit_AHTX0.h>

#define TIEMPO_LECTURA 2000

Adafruit_AHTX0 aht;

// Variables para millis
unsigned long tiempoAnterior = 0;  // Tiempo anterior
unsigned long tiempoActual = 0;    // Tiempo actual

void setup() {
  Serial.begin(115200);
  Serial.println("Prueba AHT10");

  if (!aht.begin()) {
    Serial.println("No se detectó AHT10");
    // while (1) delay(10);
  }
  Serial.println("Se detectó AHT10");
}

void loop() {

  tiempoActual = millis();  // lee el tiempo con millis(); para que no sea bloqueante

  if (tiempoActual - tiempoAnterior >= TIEMPO_LECTURA) {
    tiempoAnterior = tiempoActual;  // guardar el último tiempo de lectura.

    
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);  // Read temperature and humidity
    
    Serial.print("Temperature: ");
    Serial.print(temp.temperature); // devuelve valores en °C
    Serial.println(" °C");

    Serial.print("Humidity: ");
    Serial.print(humidity.relative_humidity); // Devuelve porcentaje
    Serial.println(" %");
  }
}
}
