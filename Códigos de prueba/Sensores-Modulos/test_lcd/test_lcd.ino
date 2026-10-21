// ST - TP DOG - Grupo 8 - Santiago Fernández, Paulina Gonzalez y Avner Yunik
// Código de prueba para LCD i2c con texto cambiante (ingresado por Serial)

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Definición pinout
#define SCL_PIN 21
#define SDA_PIN 22

// Configuración del LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);
String texto = "Hello World!";

void setup() {
  // Wire.begin
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.begin(115200);

  lcd.init();
  lcd.backlight();

  // Primera imrpesión de pantalla
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Text Input:");
  lcd.setCursor(0, 1);
  lcd.print(texto);
}

void loop() {

  if (Serial.available() > 0) {
    texto = Serial.readString();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Text Input:");
    lcd.setCursor(0, 1);
    lcd.print(texto);
  }
}