// ST - TP DOG - Grupo 8 - Santiago Fernández, Paulina Gonzalez y Avner Yunik
// código de prueba para sensor de corriente
#include <Adafruit_INA219.h>
#include <Wire.h>

Adafruit_INA219 ina219;

//float tension = 0;
//float potencia = 0;
float corriente = 0;


void setup() {
  Serial.println("ESP32 Energy");

  delay(2000);

  if (!ina219.begin()) {
    Serial.println("No se encontró el sensor INA219");
    while (1) {
      delay(10);
    }
  }

  Serial.begin(115200);
}

void loop() {
  lecturaINA219();
}

void lecturaINA219() {

  // leer valores
  //tension = ina219.getBusVoltage_V();
  //potencia = ina219.getPower_mW();
  corriente = ina219.getCurrent_mA();


  // IMPRIMIR TENSIÓN
  /* Serial.print("Voltaje: ");
  Serial.print(tension);
  Serial.print("V");
  Serial.println(" ");
  */
  
  // imprimir corriente
  Serial.print("Corriente: ");
  Serial.print(corriente);
  Serial.print("mA");
  Serial.println(" ");

  
  // imprimir potencia
  /*
  Serial.print("Potencia: ");
  Serial.print(potencia);
  Serial.print("mW");
  Serial.println(" ");
  */

  delay(2000);
}