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

// wifi y mqtt
#include "Arduino.h"
#include <PubSubClient.h>
#include "time.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>


#define MQTT_HOST "sa11b75f.ala.us-east-1.emqxsl.com"
#define MQTT_PORT 8883
#define MQTT_USERNAME "esp32"
#define MQTT_PASSWORD "mirko15"
#define MQTT_PUB "/esp32/datos_sensores"


// Tiempo lectura
#define TIEMPO_LECTURA_BMP 2000
#define TIEMPO_LECTURA_INA219 2000
#define TIEMPO_LECTURA_LDR 100
#define TIEMPO_LECTURA_MQ 500
#define TIEMPO_LECTURA_OPTO 100
#define TIEMPO_LECTURA_RADAR 1000
#define TIEMPO_REFRESH_DISPLAY 100
#define TIEMPO_MENSAJES 2000
#define TIEMPO_DEBOUNCE 50
#define TIEMPO_WIFI 500
#define TIEMPO_CONEXION_MQTT 5000
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
// Button Pinout change:
/*Original pinout:
PIN1 --> 1
PIN2 --> 3
PIN3 --> 5
PIN4 --> 4
PIN5 --> 21

Swapped places between PIN1 and PIN2 with PIN4 and PIN5
*/
#define BUTTON_PIN_1 4
#define BUTTON_PIN_2 21
#define BUTTON_PIN_3 5
#define BUTTON_PIN_4 1
#define BUTTON_PIN_5 3


// Configuración de pines UART2 en ESP32
// LD2410C,
#define RXD2 16
#define TXD2 17

enum estadosDisplay {
  INIT,
  TEMP_PRESION,
  LUZ,
  GAS,
  PRESENCIA,
  OPTO,
  CORRIENTE,
  EMERGENCIA,
  VALOR_TEMP,
  VALOR_PRESION,
  VALOR_LUZ,
  VALOR_GAS,
  VALOR_CORRIENTE,
} estadosDisplay_t;


estadosDisplay pantalla = INIT;
estadosDisplay pantallaAnterior;
// 2. Crear una variable para guardar el estado actual

// estados wifi
enum estadosWifi {
  WIFI_CONECTANDO,
  WIFI_CONECTADO,
} estadosWifi_t;

// variable wifi
estadosWifi wifiConfig = WIFI_CONECTANDO;

enum estadosConexionMQTT {
  MQTT_CONECTADO,
  MQTT_DESCONECTADO,
} estadosConexionMQTT_t;

estadosConexionMQTT mqttConfig = MQTT_DESCONECTADO;

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

// UMBRALES
float umbralTemperatura = 25;
float umbralPresion = 1000;
float umbralLuz = 100;
float umbralGas = 100;
float umbralCorriente = 1000;  //mA


bool presencia = 0;
bool opto = 0;

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

volatile int timerWifi = 0;
unsigned long timerMQTTConfig = 0;

// TIMER
hw_timer_t* timer = NULL;  // timer
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

bool pulsacionBoton1;
bool pulsacionBoton2;
bool pulsacionBoton3;
bool pulsacionBoton4;
bool pulsacionBoton5;

bool lectura1;
bool lectura2;
bool lectura3;
bool lectura4;
bool lectura5;

bool emergencia;

// MQTT
const char* ssid = "Meca-IoT-V2";      // <-- Cambiar si usas Wi-Fi externo
const char* password = "IoT$2026";  // <-- Cambiar clave de Wi-Fi
const uint8_t name_device = 138;

// Timers auxiliar variables//////////////////////////
unsigned long now = millis();    /// valor actual
unsigned long lastMeasure1 = 0;  /// variable para contar el tiempo actual
unsigned long lastMeasure2 = 0;  /// variable para contar el tiempo actual

const unsigned long interval_envio = 30000;  // Intervalo de envio de datos mqtt
const unsigned long interval_leeo = 60000;   // Intervalo de lectura de datos y guardado en la cola

/// time
long unsigned int timestamp;  // hora
const char* ntpServer = "south-america.pool.ntp.org";
const long gmtOffset_sec = -10800;
const int daylightOffset_sec = 0;

/// variables ingresar a la cola struct
int indice_entra = 0;
int indice_saca = 0;
bool flag_vacio = 1;

char mqtt_payload[150];

WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);

typedef struct  // Estructura mqtt
{
  long time;
  float T1;     /// temp en grados
  float P1;     /// valor presion entre 0 y 99
  float luz;    /// valor entre 0 y 99 . si hay luz es cercano al 100
  float G1;     /// valor gas entre 0 y 99
  float cor;    /// valor corriente entre 0 y 99
  bool pre;     /// Lectura sensores de presencia 1 presencia 0 no presencia
  bool oct;     /// Lectura sensores de presencia 1 presencia 0 no presencia
  bool Alarma;  // si se supera un umbral

} estructura;

const int valor_max_struct = 1000;          /// valor vector de struct
estructura datos_struct[valor_max_struct];  /// Guardo valores hasta que lo pueda enviar
estructura aux2;
// Función Setup
void setup() {

  Serial.end();  // ya que usa gpio1 como input
  WiFi.begin(ssid, password);
  timerMQTTConfig = millis();

  pinMode(GAS_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(OPTO_PIN, INPUT);

  pinMode(LED_PIN_1, OUTPUT);
  pinMode(LED_PIN_2, OUTPUT);


  digitalWrite(LED_PIN_1, LOW);
  digitalWrite(LED_PIN_2, LOW);

  pinMode(RELAY_PIN, OUTPUT);

  Wire.begin(SDA_PIN_A, SCL_PIN_A, 100000);
  Wire1.begin(SDA_PIN_B, SCL_PIN_B, 100000);


  pinMode(BUTTON_PIN_1, INPUT_PULLUP);
  pinMode(BUTTON_PIN_2, INPUT_PULLUP);
  pinMode(BUTTON_PIN_3, INPUT_PULLUP);
  pinMode(BUTTON_PIN_4, INPUT_PULLUP);
  pinMode(BUTTON_PIN_5, INPUT_PULLUP);


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
    //Serial.println("OK: Sensor LD2410 detectado correctamente.");
  } else {
    //Serial.println("ERROR: No se pudo conectar con el sensor LD2410. Revisa las conexiones.");
  }


  // establecer BMP280
  unsigned status;
  status = bmp.begin(0x76);
  if (!status) {

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
    // Serial.println("No se encontró el sensor INA219");
    ina219Listo = false;
  } else {
    ina219Listo = true;
  }

  // inicialización timer
  timer = timerBegin(1000000);  // 1 MHz = 1 µs
  timerAttachInterrupt(timer, &onTimer);
  timerAlarm(timer, 1000, true, 0);  // tick cada 1 ms
  lcd.clear();
}

// Función Loop
void loop() {

  // wifi
  if (WiFi.status() != WL_CONNECTED) {
    connectToWifi();
  }
  // mqtt
  if (!mqttClient.connected()) {
    connectToMqtt();
  }

  if (mqttConfig == MQTT_CONECTADO) {
    mqttClient.loop();
  }


  // Lectura de actuadores.

  BMP = lecturaBMP();
  temperatura = BMP.temperatura;
  presion = BMP.presion;
  valorLuz = lecturaLDR();
  valorGas = lecturaGas();
  opto = lecturaOpto();
  presencia = lecturaRadar();
  corriente = lecturaINA219();

  controlMQTT();
  relay();
  display();
  pulsadores();
  leds();
}


// FUNCIONES DE ACTUADORES
datosBMP lecturaBMP() {

  datosBMP bmpPack;

  if (timerBMP >= TIEMPO_LECTURA_BMP) {

    bmpPack.temperatura = bmp.readTemperature();
    bmpPack.presion = bmp.readPressure();

    timerBMP = 0;
    return bmpPack;
  }
}

float lecturaINA219() {
  if (ina219Listo == true && timerINA219 >= TIEMPO_LECTURA_INA219) {

    // leer valores
    float valorCorriente = ina219.getCurrent_mA();


    timerINA219 = 0;
    return valorCorriente;
  }
}

float lecturaLDR() {

  if (timerLDR >= TIEMPO_LECTURA_LDR) {



    float valorLDR = analogRead(LDR_PIN);
    // int mapeoLectura = (lectura / 4095) * 100;  // 4095 es el valor máximo que puede leer el ADC del ESP32
    float mapeoLecturaLDR = map(valorLDR, 0, 4095, 100, 0);

    timerLDR = 0;
    return mapeoLecturaLDR;
  }
}

float lecturaGas() {
  if (timerGas >= TIEMPO_LECTURA_MQ) {

    float lecturaGas = analogRead(GAS_PIN);
    float mapeoLecturaGas = map(lecturaGas, 0, 4095, 0, 100);


    timerGas = 0;
    return mapeoLecturaGas;
  }
}

bool lecturaOpto() {

  if (timerOpto >= TIEMPO_LECTURA_OPTO) {
    bool estado = digitalRead(OPTO_PIN);


    // LOW --> 12V - HIGH --> 0V
    timerOpto = 0;
    return estado;
  }
}

bool lecturaRadar() {

  if (timerRadar >= TIEMPO_LECTURA_RADAR) {
    radar.read();
    if (radar.isConnected()) {
      static uint32_t ultimoEnvio = 0;

      bool boolRadar = radar.presenceDetected();
      timerRadar = 0;
      return boolRadar;
    }
  }
}

void display() {

  if (timerDisplay >= TIEMPO_REFRESH_DISPLAY) {
    if (pantalla != pantallaAnterior) {
      lcd.clear();
      pantallaAnterior = pantalla;
    }
  }

  switch (pantalla) {
    case INIT:


      lcd.setCursor(0, 0);
      lcd.print("1: VISUALIZAR");
      lcd.setCursor(0, 1);
      lcd.print("2: MODIFICAR");

      if (pulsacionBoton1) {
        pantalla = TEMP_PRESION;
        pulsacionBoton1 = false;
      } else if (pulsacionBoton2) {
        pantalla = VALOR_TEMP;
        pulsacionBoton2 = false;
      }


      break;
    case TEMP_PRESION:

      lcd.setCursor(0, 0);
      lcd.print("Temp: " + String(temperatura) + " C");
      lcd.setCursor(0, 1);
      lcd.print("Presion: " + String(presion));

      if (pulsacionBoton1) {
        pantalla = LUZ;
        pulsacionBoton1 = false;
      } else if (pulsacionBoton2) {
        pantalla = EMERGENCIA;
        pulsacionBoton2 = false;
      } else if (pulsacionBoton3) {
        pantalla = INIT;
        pulsacionBoton3 = false;
      }
      break;

    case LUZ:

      lcd.setCursor(0, 0);
      lcd.print("Valor Luz: ");
      lcd.setCursor(0, 1);
      lcd.print(valorLuz);

      if (pulsacionBoton1) {
        pantalla = GAS;
        pulsacionBoton1 = false;
      } else if (pulsacionBoton2) {
        pantalla = TEMP_PRESION;
        pulsacionBoton2 = false;
      } else if (pulsacionBoton3) {
        pantalla = INIT;
        pulsacionBoton3 = false;
      }
      break;

    case GAS:

      lcd.setCursor(0, 0);
      lcd.print("Valor Gas: ");
      lcd.setCursor(0, 1);
      lcd.print(valorGas);

      if (pulsacionBoton1) {
        pantalla = PRESENCIA;
        pulsacionBoton1 = false;
      } else if (pulsacionBoton2) {
        pantalla = LUZ;
        pulsacionBoton2 = false;
      } else if (pulsacionBoton3) {
        pantalla = INIT;
        pulsacionBoton3 = false;
      }
      break;

    case PRESENCIA:

      if (presencia == 0) {


        lcd.setCursor(0, 0);
        lcd.print("Presencia:");
        lcd.setCursor(0, 1);
        lcd.print("NO");
      } else {


        lcd.setCursor(0, 0);
        lcd.print("Presencia:");
        lcd.setCursor(0, 1);
        lcd.print("SI");
      }


      if (pulsacionBoton1) {
        pantalla = CORRIENTE;
        pulsacionBoton1 = false;
      } else if (pulsacionBoton2) {
        pantalla = GAS;
        pulsacionBoton2 = false;
      } else if (pulsacionBoton3) {
        pantalla = INIT;
        pulsacionBoton3 = false;
      }
      break;

    case CORRIENTE:

      lcd.setCursor(0, 0);
      lcd.print("Corriente:");
      lcd.setCursor(0, 1);
      lcd.print(corriente);



      if (pulsacionBoton1) {
        pantalla = OPTO;
        pulsacionBoton1 = false;
      } else if (pulsacionBoton2) {
        pantalla = PRESENCIA;
        pulsacionBoton2 = false;
      } else if (pulsacionBoton3) {
        pantalla = INIT;
        pulsacionBoton3 = false;
      }

      break;

    case OPTO:

      lcd.setCursor(0, 0);
      lcd.print("Opto:");
      lcd.setCursor(0, 1);
      lcd.print(opto);



      if (pulsacionBoton1) {
        pantalla = EMERGENCIA;
        pulsacionBoton1 = false;
      } else if (pulsacionBoton2) {
        pantalla = CORRIENTE;
        pulsacionBoton2 = false;
      } else if (pulsacionBoton3) {
        pantalla = INIT;
        pulsacionBoton3 = false;
      }
      break;
      case EMERGENCIA:

      lcd.setCursor(0, 0);
      lcd.print("EMERGENCIA:");
      lcd.setCursor(0, 1);
      lcd.print(emergencia);



      if (pulsacionBoton1) {
        pantalla = TEMP_PRESION;
        pulsacionBoton1 = false;
      } else if (pulsacionBoton2) {
        pantalla = OPTO;
        pulsacionBoton2 = false;
      } else if (pulsacionBoton3) {
        pantalla = INIT;
        pulsacionBoton3 = false;
      }
      break;
    case VALOR_TEMP:

      // RANGO DE UMBRAL DE TEMPERATURA --> -40°C a 80°C
      lcd.setCursor(0, 0);
      lcd.print("Temp: " + String(umbralTemperatura));

      if (pulsacionBoton1) {
        pantalla = VALOR_PRESION;
        pulsacionBoton1 = false;
      } else if (pulsacionBoton2) {
        pantalla = VALOR_CORRIENTE;
        pulsacionBoton2 = false;
      } else if (pulsacionBoton3) {
        pantalla = INIT;
        pulsacionBoton3 = false;
      } else if (pulsacionBoton4) {
        if (umbralTemperatura < 80) { umbralTemperatura++; }
        pulsacionBoton4 = false;
      } else if (pulsacionBoton5) {
        if (umbralTemperatura > -40) { umbralTemperatura--; }
        pulsacionBoton5 = false;
      }
      break;
    case VALOR_PRESION:

      // RANGO DE UMBRAL DE PRESION --> 0 Pa a 200 hPa
      lcd.setCursor(0, 0);
      lcd.print("Pa: " + String(umbralPresion));

      if (pulsacionBoton1) {
        pantalla = VALOR_LUZ;
        pulsacionBoton1 = false;
      } else if (pulsacionBoton2) {
        pantalla = VALOR_TEMP;
        pulsacionBoton2 = false;
      } else if (pulsacionBoton3) {
        pantalla = INIT;
        pulsacionBoton3 = false;
      } else if (pulsacionBoton4) {
        if (umbralPresion < 200) { umbralPresion++; }
        pulsacionBoton4 = false;
      } else if (pulsacionBoton5) {
        if (umbralPresion > 0) { umbralPresion--; }
        pulsacionBoton5 = false;
      }
      break;

    case VALOR_LUZ:

      // RANGO DE UMBRAL DE LUZ --> 0% a 100%
      lcd.setCursor(0, 0);
      lcd.print("Luz: " + String(umbralLuz));

      if (pulsacionBoton1) {
        pantalla = VALOR_GAS;
        pulsacionBoton1 = false;
      } else if (pulsacionBoton2) {
        pantalla = VALOR_PRESION;
        pulsacionBoton2 = false;
      } else if (pulsacionBoton3) {
        pantalla = INIT;
        pulsacionBoton3 = false;
      } else if (pulsacionBoton4) {
        if (umbralLuz < 100) { umbralLuz++; }
        pulsacionBoton4 = false;
      } else if (pulsacionBoton5) {
        if (umbralLuz > 0) { umbralLuz--; }
        pulsacionBoton5 = false;
      }
      break;

    case VALOR_GAS:

      // RANGO DE UMBRAL DE GAS --> 0% a 100%
      lcd.setCursor(0, 0);
      lcd.print("Gas: " + String(umbralGas));

      if (pulsacionBoton1) {
        pantalla = VALOR_CORRIENTE;
        pulsacionBoton1 = false;
      } else if (pulsacionBoton2) {
        pantalla = VALOR_LUZ;
        pulsacionBoton2 = false;
      } else if (pulsacionBoton3) {
        pantalla = INIT;
        pulsacionBoton3 = false;
      } else if (pulsacionBoton4) {
        if (umbralGas < 100) { umbralGas++; }
        pulsacionBoton4 = false;
      } else if (pulsacionBoton5) {
        if (umbralGas > 0) { umbralGas--; }
        pulsacionBoton5 = false;
      }
      break;

    case VALOR_CORRIENTE:

      // RANGO DE UMBRAL DE corriente --> 0mA a 5000mA
      lcd.setCursor(0, 0);
      lcd.print("Corriente: " + String(umbralCorriente));

      if (pulsacionBoton1) {
        pantalla = VALOR_TEMP;
        pulsacionBoton1 = false;
      } else if (pulsacionBoton2) {
        pantalla = VALOR_GAS;
        pulsacionBoton2 = false;
      } else if (pulsacionBoton3) {
        pantalla = INIT;
        pulsacionBoton3 = false;
      } else if (pulsacionBoton4) {
        if (umbralCorriente < 5000) { umbralCorriente = umbralCorriente + 100; }
        pulsacionBoton4 = false;
      } else if (pulsacionBoton5) {
        if (umbralCorriente > 0) { umbralCorriente = umbralCorriente - 100; }
        pulsacionBoton5 = false;
      }
      break;
  }
}
void pulsadores() {
  lectura1 = digitalRead(BUTTON_PIN_1);
  lectura2 = digitalRead(BUTTON_PIN_2);
  lectura3 = digitalRead(BUTTON_PIN_3);
  lectura4 = digitalRead(BUTTON_PIN_4);
  lectura5 = digitalRead(BUTTON_PIN_5);

  if (lectura1 != ultimoBoton1) {

    timerBoton1 = 0;
    ultimoBoton1 = lectura1;
  }

  if (timerBoton1 >= TIEMPO_DEBOUNCE) {

    if (lectura1 != estadoBoton1) {

      estadoBoton1 = lectura1;

      if (estadoBoton1 == LOW) {
        pulsacionBoton1 = true;
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
        pulsacionBoton2 = true;
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
        pulsacionBoton3 = true;
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
        pulsacionBoton4 = true;
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
        pulsacionBoton5 = true;
      }
    }
  }
}

void relay() {

  if (timerRelay >= 1000) {
    if (RELAY_PIN == LOW) {
      digitalWrite(RELAY_PIN, HIGH);
      //Serial.println("Relay HIGH");
    } else {
      digitalWrite(RELAY_PIN, LOW);
      //Serial.println("Relay LOW");
    }

    timerRelay = 0;
  }
}


// FUNCIONES MQTT
// Wifi --> hacerla no bloqueante
void connectToWifi() {
  switch (wifiConfig) {

    case WIFI_CONECTANDO:

      digitalWrite(LED_PIN_2, LOW);

      if (WiFi.status() == WL_CONNECTED) {

        wifiConfig = WIFI_CONECTADO;
        Serial.println("WiFi conectado");
      }

      break;


    case WIFI_CONECTADO:

      digitalWrite(LED_PIN_2, HIGH);
      if (WiFi.status() != WL_CONNECTED) {

        Serial.println("WiFi desconectado");

        WiFi.begin(ssid, password);

        wifiConfig = WIFI_CONECTANDO;
      }

      break;
  }
}

// hacerla no bloqueante
void connectToMqtt() {
  switch (mqttConfig) {

    case MQTT_DESCONECTADO:

      // Primero verificar conexión WiFi
      if (wifiConfig != WIFI_CONECTADO) {
        return;
      }

      // Esperar intervalo entre intentos
      if (millis() - timerMQTTConfig >= TIEMPO_CONEXION_MQTT) {

        Serial.println("Conectando a MQTT...");

        String clientId = "ESP32_Grupo_" + String(name_device) + "_" + WiFi.macAddress();

        timerMQTTConfig = millis();

        if (mqttClient.connect(clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {

          Serial.println("MQTT conectado");

          mqttConfig = MQTT_CONECTADO;

        } else {

          Serial.print("Error MQTT: ");
          Serial.println(mqttClient.state());
        }
      }

      break;


    case MQTT_CONECTADO:

      if (!mqttClient.connected()) {

        Serial.println("MQTT desconectado");

        mqttConfig = MQTT_DESCONECTADO;

        timerMQTTConfig = millis();
      }

      break;
  }
}



// MQTT FUNCTION
void fun_envio_mqtt() {
  fun_saca();           //// veo si hay valores nuevos
  if (flag_vacio == 0)  //// si hay los envio
  {

    snprintf(mqtt_payload, sizeof(mqtt_payload),
             "%d&%ld&%.2f&%.2f&%.2f&%.2f&%.2f&%u&%u&%u",
             name_device, aux2.time, aux2.T1, aux2.P1, aux2.luz,
             aux2.G1, aux2.cor, aux2.pre, aux2.oct, aux2.Alarma);

    aux2.time = 0;
    aux2.T1 = 0;
    aux2.P1 = 0;
    aux2.luz = 0;
    aux2.G1 = 0;
    aux2.cor = 0;
    aux2.pre = 0;
    aux2.oct = 0;
    aux2.Alarma = 0;



    mqttClient.publish(MQTT_PUB, mqtt_payload);
  } else {
  }
}

void fun_saca() {
  if (indice_saca != indice_entra) {
    aux2.time = datos_struct[indice_saca].time;
    aux2.T1 = datos_struct[indice_saca].T1;
    aux2.P1 = datos_struct[indice_saca].P1;
    aux2.luz = datos_struct[indice_saca].luz;
    aux2.G1 = datos_struct[indice_saca].G1;
    aux2.cor = datos_struct[indice_saca].cor;
    aux2.pre = datos_struct[indice_saca].pre;
    aux2.oct = datos_struct[indice_saca].oct;
    aux2.Alarma = datos_struct[indice_saca].Alarma;

    flag_vacio = 0;

    if (indice_saca >= (valor_max_struct - 1)) {
      indice_saca = 0;
    } else {
      indice_saca++;
    }

  } else {
    flag_vacio = 1;  ///// no hay datos
  }
}

void fun_entra(void) {
  if (indice_entra >= valor_max_struct) {
    indice_entra = 0;
  }


  timestamp = time(NULL);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {

    return;
  }


  datos_struct[indice_entra].time = timestamp;
  datos_struct[indice_entra].T1 = temperatura;
  datos_struct[indice_entra].P1 = presion;
  datos_struct[indice_entra].luz = valorLuz;
  datos_struct[indice_entra].G1 = valorGas;
  datos_struct[indice_entra].cor = corriente;
  datos_struct[indice_entra].pre = presencia;
  datos_struct[indice_entra].oct = opto;
  datos_struct[indice_entra].Alarma = emergencia;

  indice_entra++;
}

void leds() {
  if (temperatura >= umbralTemperatura || presion >= umbralPresion || valorLuz >= umbralLuz || valorGas >= umbralGas || corriente >= umbralCorriente) {
    digitalWrite(LED_PIN_1, HIGH);
    emergencia = 1;
  } else {
    digitalWrite(LED_PIN_1, LOW);
    emergencia = 0;
  }
}

void controlMQTT() {  // control de envío mqtt

  now = millis();
  if (now - lastMeasure1 > interval_envio) {
    lastMeasure1 = now;
    fun_envio_mqtt();
  }
  if (now - lastMeasure2 > interval_leeo) {
    lastMeasure2 = now;
    fun_entra();
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
  timerRelay += 1;
  timerDisplay += 1;
  timerWifi += 1;
  timerMQTTConfig += 1;
  timerBoton1 += 1;
  timerBoton2 += 1;
  timerBoton3 += 1;
  timerBoton4 += 1;
  timerBoton5 += 1;

  //timerMensajes += 1;
}
