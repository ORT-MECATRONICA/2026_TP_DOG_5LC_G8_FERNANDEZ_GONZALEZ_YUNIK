//ST - TP DOG - Grupo 8 - Santiago Fernández, Paulina Gonzalez y Avner Yunik
// Código de prueba para LCD i2c
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Pinout definition
#define SCL_PIN 21
#define SDA_PIN 22

// Configuración del LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  // Wire.begin takes (SDA, SCL)
  Wire.begin(SDA_PIN, SCL_PIN);

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("ESP32 LCD Test");

  lcd.setCursor(0, 1);
  lcd.print("Status: Working!");
}

void loop() {
}