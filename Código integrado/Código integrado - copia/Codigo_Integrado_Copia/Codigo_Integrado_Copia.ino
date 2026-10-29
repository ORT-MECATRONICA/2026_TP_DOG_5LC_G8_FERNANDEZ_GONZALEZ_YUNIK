// ST - TP DOG - Grupo 8 - Santiago Fernández, Paulina Gonzalez y Avner Yunik

/*
  Código de prueba integrador de sensores y módulos. Incluirá
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
#define TIEMPO_REFRESH_DISPLAY 200
#define TIEMPO_MENSAJES 2000
#define TIEMPO_DEBOUNCE 50

// pinout
#define OPTO_PIN 39

// i2c lcd,
#define SCL_PIN_A 27
#define SDA_PIN_A 13

// Bmp 280
#define SCL_PIN_B 19
#define SDA_PIN_B 18

#define LDR_PIN 34
#define GAS_PIN 33
#define RELAY_PIN 32

#define LED_PIN_1 23
#define LED_PIN_2 22

#define BUTTON_PIN_1 1
#define BUTTON_PIN_2 3
#define BUTTON_PIN_3 5
#define BUTTON_PIN_4 4
#define BUTTON_PIN_5 21



// Configuración de pines UART2 en ESP32
// LD2410C,
#define RXD2 16
#define TXD2 17

// Configuración de actuadores

Adafruit_BMP280 bmp(&Wire1);  // I2C
Adafruit_INA219 ina219;       // ina219
ld2410 radar;                 // ld2410

// corregir bus i2c
LiquidCrystal_I2C lcd(0x27, 16, 2);  // LCD

// BOOL
bool bmpListo = false;
bool ina219Listo = false;

// Uso de Struct para recopilar datos del bmp a la vez.
struct datosBMP {
  float temperatura;
  float presion;
};

// declaración del struct
datosBMP BMP;
// variables valores
float corriente = 0;
float temperatura = 0;
float presion = 0;
float valorLuz = 0;
float valorGas = 0;

bool presencia = 0;
bool opto = 0;

// test variable
int contador = 0;

// VARIABLES timer
volatile int timerBMP = 0;
volatile int timerINA219 = 0;
volatile int timerLDR = 0;
volatile int timerGas = 0;
volatile int timerOpto = 0;
volatile int timerRadar = 0;
volatile int timerPulsador = 0;
volatile int timerDisplay = 0;
volatile int timerMensajes = 0;
volatile int timerRelay = 0;

volatile int timerBoton1 = 0;
volatile int timerBoton2 = 0;
volatile int timerBoton3 = 0;
volatile int timerBoton4 = 0;
volatile int timerBoton5 = 0;
// TIMER
hw_timer_t *timer = NULL;  // timer
void IRAM_ATTR onTimer();  // function interrupts every 1msD

// Estados botones

bool ultimoBoton1 = HIGH;
bool estadoBoton1 = HIGH;

bool ultimoBoton2 = HIGH;
bool estadoBoton2 = HIGH;

bool ultimoBoton3 = HIGH;
bool estadoBoton3 = HIGH;

bool ultimoBoton4 = HIGH;
bool estadoBoton4 = HIGH;

bool ultimoBoton5 = HIGH;
bool estadoBoton5 = HIGH;



void setup() {

  pinMode(GAS_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(OPTO_PIN, INPUT);

  pinMode(LED_PIN_1, OUTPUT);
  pinMode(LED_PIN_2, OUTPUT);

  pinMode(RELAY_PIN, OUTPUT);

  Wire.begin(SDA_PIN_A, SCL_PIN_A, 100000);
  Wire1.begin(SDA_PIN_B, SCL_PIN_B, 100000);


  pinMode(BUTTON_PIN_1, INPUT_PULLUP);
  pinMode(BUTTON_PIN_2, INPUT_PULLUP);
  pinMode(BUTTON_PIN_3, INPUT_PULLUP);
  pinMode(BUTTON_PIN_4, INPUT_PULLUP);
  pinMode(BUTTON_PIN_5, INPUT_PULLUP);

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

  if (bmpListo == true) {

    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                    Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                    Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                    Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                    Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
  }

  // establecer INA219
  if (!ina219.begin(&Wire1)) {
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
  /*
  BMP = lecturaBMP();
  temperatura = BMP.temperatura;
  presion = BMP.presion;
 
  valorLuz = lecturaLDR();
  valorGas = lecturaGas();
  opto = lecturaOpto();
  presencia = lecturaRadar();
  corriente = lecturaINA219();
  display();
  relay();
  */



  //Serial.println(presencia);
  relay();
  display();
  valorLuz = lecturaLDR();
  presencia = lecturaRadar();
  pulsadores();
  // mensajes();
}


// FUNCIONES DE ACTUADORES
datosBMP lecturaBMP() {

  datosBMP bmpPack;

  if (timerBMP >= TIEMPO_LECTURA_BMP) {

    bmpPack.temperatura = bmp.readTemperature();
    bmpPack.presion = bmp.readPressure();


    /*
   
   Serial.print(F("Temperatura = "));
    Serial.print(temperatura);
    Serial.println(" *C");

    Serial.print(F("Presión = "));
    Serial.print(presion);
    Serial.println(" Pa");
    
    */

    return bmpPack;


    timerBMP = 0;
  }
}

float lecturaINA219() {
  if (ina219Listo == true && timerINA219 >= TIEMPO_LECTURA_INA219) {

    // leer valores
    float valorCorriente = ina219.getCurrent_mA();

    // imprimir corriente
    /*  Serial.print("Corriente del LED: ");
    Serial.print(corriente);
    Serial.print("mA");
    Serial.println(" ");
    */
    return valorCorriente;

    timerINA219 = 0;
  }
}

float lecturaLDR() {

  if (timerLDR >= TIEMPO_LECTURA_LDR) {



    float valorLDR = analogRead(LDR_PIN);
    // int mapeoLectura = (lectura / 4095) * 100;  // 4095 es el valor máximo que puede leer el ADC del ESP32
    float mapeoLecturaLDR = map(valorLDR, 0, 4095, 100, 0);
    /*
    Serial.print("Lectura LDR: ");
    Serial.println(valorLDR);
    Serial.print("Lectura LDR en porcentaje: ");
    Serial.println(mapeoLecturaLDR);
    */
    return mapeoLecturaLDR;
    timerLDR = 0;
  }
}

float lecturaGas() {
  if (timerGas >= TIEMPO_LECTURA_MQ) {

    float lecturaGas = analogRead(GAS_PIN);
    float mapeoLecturaGas = map(lecturaGas, 0, 4095, 0, 100);

    Serial.print("Lectura GAS sin mapear: ");
    Serial.println(lecturaGas);
    Serial.print("Lectura GAS en porcentaje: ");
    Serial.println(mapeoLecturaGas);

    return mapeoLecturaGas;
    timerGas = 0;
  }
}

bool lecturaOpto() {

  if (timerOpto >= TIEMPO_LECTURA_OPTO) {
    bool estado = digitalRead(OPTO_PIN);
    return estado;

    // LOW --> 12V - HIGH --> 0V
    timerOpto = 0;
  }
}

bool lecturaRadar() {

  if (timerRadar >= TIEMPO_LECTURA_RADAR) {
    radar.read();
    if (radar.isConnected()) {
      static uint32_t ultimoEnvio = 0;

      bool boolRadar = radar.presenceDetected();
      return boolRadar;

      timerRadar = 0;
    }
  }
}

void display() {
  if (timerDisplay >= TIEMPO_REFRESH_DISPLAY) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(valorLuz);
    lcd.setCursor(0, 1);
    lcd.print(presencia);
    timerDisplay = 0;
  }
}

// incompleto

void mensajes() {

  if (timerMensajes >= TIEMPO_MENSAJES) {
    Serial.print("Temperatura: ");
    Serial.println(temperatura);

    Serial.print("Presion: ");
    Serial.println(presion);

    Serial.print("Corriente: ");
    Serial.println(corriente);

    Serial.print("Valor luz: ");
    Serial.println(valorLuz);

    Serial.print("Valor Gas: ");
    Serial.println(valorGas);

    Serial.print("OPTO: ");
    Serial.println(opto);

    Serial.print("PRESENCIA: ");
    Serial.println(presencia);

    Serial.print("relay pin: ");
    Serial.println(digitalRead(RELAY_PIN));

    timerMensajes = 0;
  }
}



void pulsadores() {
  bool lectura1 = digitalRead(BUTTON_PIN_1);
  bool lectura2 = digitalRead(BUTTON_PIN_2);
  bool lectura3 = digitalRead(BUTTON_PIN_3);
  bool lectura4 = digitalRead(BUTTON_PIN_4);
  bool lectura5 = digitalRead(BUTTON_PIN_5);

  if (lectura1 != ultimoBoton1) {

    timerBoton1 = 0;
    ultimoBoton1 = lectura1;
  }

  if (timerBoton1 >= TIEMPO_DEBOUNCE) {

    if (lectura1 != estadoBoton1) {

      estadoBoton1 = lectura1;

      if (estadoBoton1 == LOW) {
        digitalWrite(LED_PIN_1, HIGH);

      } else {
        digitalWrite(LED_PIN_1, LOW);
      }
    }
  }

  if (lectura2 != ultimoBoton2) {

    timerBoton2 = 0;
    ultimoBoton2 = lectura2;
  }

  if (timerBoton2 >= TIEMPO_DEBOUNCE) {

    if (lectura2 != estadoBoton2) {

      estadoBoton2 = lectura2;

      if (estadoBoton2 == LOW) {
        digitalWrite(LED_PIN_2, HIGH);

      } else {
        digitalWrite(LED_PIN_2, LOW);
      }
    }
  }

  if (lectura3 != ultimoBoton3) {

    timerBoton3 = 0;
    ultimoBoton3 = lectura3;
  }

  if (timerBoton3 >= TIEMPO_DEBOUNCE) {

    if (lectura3 != estadoBoton3) {

      estadoBoton3 = lectura3;

      if (estadoBoton3 == LOW) {
        contador++;

      } else {
      }
    }
  }

  if (lectura4 != ultimoBoton4) {

    timerBoton4 = 0;
    ultimoBoton4 = lectura4;
  }

  if (timerBoton4 >= TIEMPO_DEBOUNCE) {

    if (lectura4 != estadoBoton4) {

      estadoBoton4 = lectura4;

      if (estadoBoton4 == LOW) {
        contador--;

      } else {
      }
    }
  }

  if (lectura5 != ultimoBoton5) {

    timerBoton5 = 0;
    ultimoBoton5 = lectura5;
  }

  if (timerBoton5 >= TIEMPO_DEBOUNCE) {

    if (lectura5 != estadoBoton5) {

      estadoBoton5 = lectura5;

      if (estadoBoton5 == LOW) {
        contador = contador + 2;

      } else {
      }
    }
  }
}

void relay() {

  if (timerRelay >= 1000) {
    if (RELAY_PIN == LOW) {
      digitalWrite(RELAY_PIN, HIGH);
    } else {
      digitalWrite(RELAY_PIN, LOW);
    }

    timerRelay = 0;
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
  timerDisplay += 1;
  timerRelay += 1;


  timerBoton1 += 1;
  timerBoton2 += 1;
  timerBoton3 += 1;
  timerBoton4 += 1;
  timerBoton5 += 1;

  //timerMensajes += 1;
}
