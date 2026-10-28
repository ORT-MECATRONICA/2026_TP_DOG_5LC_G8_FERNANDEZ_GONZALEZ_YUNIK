// ST - TP DOG - G8 - Fernández, Gonzalez, Yunik.
// Código de prueba de Telegram.

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

#define DIRECCION_WIFI "MECA-IoT-V2"
#define CONTRASENA_WIFI "IoT$2026"

// Reemplazar con las credenciales
#define BOT_TOKEN "***"
#define CHAT_ID "***"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

typedef enum {
  WIFI_INICIANDO,
  WIFI_ESPERANDO_CONEXION,
  WIFI_CONECTADO
} WIFI_t;

WIFI_t estadoWifi = WIFI_INICIANDO;

void setup() {
  Serial.begin(115200);
  
  // Configuración inicial del Wi-Fi y certificado de Telegram
  WiFi.mode(WIFI_STA);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
}

void loop() {
  // Máquina de estados no bloqueante para el Wi-Fi
  switch (estadoWifi) {
    case WIFI_INICIANDO:
      Serial.println("Iniciando conexión Wi-Fi...");
      WiFi.begin(DIRECCION_WIFI, CONTRASENA_WIFI);
      estadoWifi = WIFI_ESPERANDO_CONEXION;
      break;

    case WIFI_ESPERANDO_CONEXION:
      // Comprobación rápida sin bloquear la ejecución del loop
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWi-Fi conectado con éxito!");
        Serial.print("Dirección IP: ");
        Serial.println(WiFi.localIP());
        
        estadoWifi = WIFI_CONECTADO;
        
        // Enviar el mensaje solo una vez al conectarse
        bot.sendMessage(CHAT_ID, "Se activó BOT", "");
      }
      break;

    case WIFI_CONECTADO:
      // Si la conexión se pierde en algún momento, volvemos a intentar
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Conexión perdida. Reintentando...");
        estadoWifi = WIFI_INICIANDO;
      }
      break;
  }

  // --- Tu código principal va aquí ---
  // El loop sigue ejecutándose libremente en cada ciclo sin pausar nada.
}
