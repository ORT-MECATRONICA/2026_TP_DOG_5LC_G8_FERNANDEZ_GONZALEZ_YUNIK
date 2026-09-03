// ST - TP DOG - Grupo 8 - Santiago Fernández, Paulina Gonzalez y Avner Yunik

#include <ld2410.h>

// Crear la instancia del sensor
ld2410 radar;

// Configuración de pines UART2 en ESP32
#define RXD2 16
#define TXD2 17

void setup() {
  // Serial para el Monitor Serie del Arduino IDE
  Serial.begin(115200);
  
  // Serial2 para la comunicación con el módulo LD2410C (Baudrate por defecto: 256000)
  Serial2.begin(256000, SERIAL_8N1, RXD2, TXD2);

  Serial.print("\n--- Iniciando prueba de sensor HLK-LD2410C-P ---");

  // Inicializar el radar con el puerto Serial2
  if (radar.begin(Serial2)) {
    Serial.println("OK: Sensor LD2410 detectado correctamente.");
  } else {
    Serial.println("ERROR: No se pudo conectar con el sensor LD2410. Revisa las conexiones.");
  }
}

void loop() {
  // Mantiene la comunicación con el sensor en cada ciclo
  radar.read();

  // Comprobar si hay lecturas disponibles
  if (radar.isConnected()) {
    static uint32_t ultimoEnvio = 0;

    // Reportar el estado cada 500 milisegundos para no saturar la consola
    if (millis() - ultimoEnvio > 1000) {
      ultimoEnvio = millis();

      if (radar.presenceDetected()) {
        Serial.print("[PRESENCIA DETECTADA] -> ");

        if (radar.stationaryTargetDetected()) {
          Serial.print("Objetivo Estático: ");
          Serial.print(radar.stationaryTargetDistance());
          Serial.print(" cm (Energía: ");
          Serial.print(radar.stationaryTargetEnergy());
          Serial.print(") | ");
        }

        if (radar.movingTargetDetected()) {
          Serial.print("Objetivo en Movimiento: ");
          Serial.print(radar.movingTargetDistance());
          Serial.print(" cm (Energía: ");
          Serial.print(radar.movingTargetEnergy());
          Serial.print(")");
        }
        Serial.println();
      } else {
        Serial.println("[---] Sin presencia humana detectada.");
      }
    }
  }
}