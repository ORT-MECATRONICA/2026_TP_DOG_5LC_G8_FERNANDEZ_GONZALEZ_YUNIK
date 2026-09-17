// ST - TP DOG - Grupo 8 - Santiago Fernández, Paulina Gonzalez y Avner Yunik

/*
 [INCOMPLETO] Código de prueba integrador de sensores y módulos. Incluirá
    • LCD 16x2
    • MQ FLYING FISH
    • BMP 280
    • INA 219 DC
    • ENTRADA OPTOCOPLADA PC817C
    • RADAR HLK LD2410C
    • RELÉ
    • LDR
*/

// Librerías
#include <Adafruit_BMP280.h>
#include <Adafruit_INA219.h>
#include <LiquidCrystal_I2C.h>
#include <ld2410.h>
#include <Wire.h>
#include <SPI.h>

#define BAUD 115200

// Tiempo lectura
#define TIEMPO_LECTURA_BMP 2000
#define TIEMPO_LECTURA_INA219 2000
#define TIEMPO_LECTURA_LDR 20
#define TIEMPO_LECTURA_MQ 500
#define TIEMPO_LECTURA_OPTO 100
#define TIEMPO_LECTURA_RADAR 1000

// pinout
#define OPTO_PIN 4
#define SCL_PIN 21
#define SDA_PIN 22
#define LDR_PIN 32
#define LED_PIN 18
#define GAS_PIN 33
#define RELAY_PIN 13

// Configuración de pines UART2 en ESP32
#define RXD2 16
#define TXD2 17

// Configuración de actuadores
Adafruit_BMP280 bmp;                 // I2C
Adafruit_INA219 ina219;              // ina219
ld2410 radar;                        // ld2410
LiquidCrystal_I2C lcd(0x27, 16, 2);  // LCD

// BOOL
bool bmpListo = false;
bool ina219Listo = false;

// variables valores
float corriente = 0;
float temperatura = 0;
float presion = 0;

// VARIABLES timer
volatile int timerBMP = 0;
volatile int timerINA219 = 0;
volatile int timerLDR = 0;
volatile int timerGas = 0;
volatile int timerOpto = 0;
volatile int timerRadar = 0;
volatile int timerPulsador = 0;

// TIMER
hw_timer_t *timer = NULL;  // timer
void IRAM_ATTR onTimer();  // function interrupts every 1msD

void setup() {

  pinMode(GAS_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(OPTO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.begin(BAUD);

  // Establecer LCD
  lcd.init();
  lcd.backlight();

  // Primera imrpesión de pantalla
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TP DOG G8 5LC");
  lcd.setCursor(0, 1);
  lcd.print("Codigo Integrado");

  // establecer radar
  Serial2.begin(256000, SERIAL_8N1, RXD2, TXD2);

  if (radar.begin(Serial2)) {
    Serial.println("OK: Sensor LD2410 detectado correctamente.");
  } else {
    Serial.println("ERROR: No se pudo conectar con el sensor LD2410. Revisa las conexiones.");
  }


  // establecer BMP280
  unsigned status;
  status = bmp.begin(0x76);
  if (!status) {
    Serial.println(F("BMP No Configurado!"));
    Serial.print("ID SENSOR: 0x");
    Serial.println(bmp.sensorID(), 16);
    bmpListo = false;
  } else {
    bmpListo = true;
  }

  if (bmpListo = true) {

    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                    Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                    Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                    Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                    Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
  }

  // establecer INA219
  if (!ina219.begin()) {
    Serial.println("No se encontró el sensor INA219");
    ina219Listo = false;
  } else {
    ina219Listo = true;
  }

  // inicialización timer
  timer = timerBegin(1000000);  // 1 MHz = 1 µs
  timerAttachInterrupt(timer, &onTimer);
  timerAlarm(timer, 1000, true, 0);  // tick cada 1 ms
}

// Función Loop
void loop() {

  // Lectura de actuadores.
  lecturaBMP();
  lecturaINA219();
  lecturaLDR();
  lecturaGas();
  lecturaOpto();
  lecturaRadar();

}


// FUNCIONES DE ACTUADORES
void lecturaBMP() {

  if (timerBMP >= TIEMPO_LECTURA_BMP) {
    temperatura = bmp.readTemperature();
    presion = bmp.readPressure();

    Serial.print(F("Temperatura = "));
    Serial.print(temperatura);
    Serial.println(" *C");

    Serial.print(F("Presión = "));
    Serial.print(presion);
    Serial.println(" Pa");

    timerBMP = 0;
  }
}

void lecturaINA219() {
  if (ina219Listo = true && timerINA219 >= TIEMPO_LECTURA_INA219) {

    // leer valores
    corriente = ina219.getCurrent_mA();

    // imprimir corriente
    Serial.print("Corriente del LED: ");
    Serial.print(corriente);
    Serial.print("mA");
    Serial.println(" ");
    timerINA219 = 0;
  }
}

void lecturaLDR() {

  if (timerLDR >= TIEMPO_LECTURA_LDR) {

    int lecturaLDR = analogRead(LDR_PIN);
    // int mapeoLectura = (lectura / 4095) * 100;  // 4095 es el valor máximo que puede leer el ADC del ESP32
    int mapeoLecturaLDR = map(lecturaLDR, 0, 4095, 0, 100);
    Serial.print("Lectura LDR: ");
    Serial.println(lecturaLDR);
    Serial.print("Lectura LDR en porcentaje: ");
    Serial.println(mapeoLecturaLDR);
    timerLDR = 0;
  }
}

void lecturaGas() {
  if (timerGas >= TIEMPO_LECTURA_MQ) {

    int lecturaGas = analogRead(GAS_PIN);
    // int mapeoLectura = (lectura / 4095) * 100;  // 4095 es el valor máximo que puede leer el ADC del ESP32
    int mapeoLecturaGas = map(lecturaGas, 0, 4095, 0, 100);
    Serial.print("Lectura GAS sin mapear: ");
    Serial.println(lecturaGas);
    Serial.print("Lectura GAS en porcentaje: ");
    Serial.println(mapeoLecturaGas);
    timerGas = 0;
  }
}

void lecturaOpto() {

  if (timerOpto >= TIEMPO_LECTURA_OPTO) {
    int estado = digitalRead(OPTO_PIN);

    // Lógica inversa: LOW significa que hay 12V en la entrada
    if (estado == LOW) {
      Serial.println("Señal de 12V DETECTADA");
    } else {
      Serial.println("Sin señal (0V)");
    }
    timerOpto = 0;
  }
}

void lecturaRadar() {

  if (timerRadar >= TIEMPO_LECTURA_RADAR) {
    radar.read();
    if (radar.isConnected()) {
      static uint32_t ultimoEnvio = 0;

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
    timerRadar = 0;
  }

}

void IRAM_ATTR onTimer() {
  timerBMP += 1;
  timerINA219 += 1;
  timerLDR += 1;
  timerGas += 1;
  timerOpto += 1;
  timerRadar += 1;
  timerPulsador += 1;
}