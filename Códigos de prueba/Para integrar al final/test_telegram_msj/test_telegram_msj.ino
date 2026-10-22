// ST - TP DOG - G8 - Fernández, Gonzalez, Yunik.
// Código de prueba de Telegram.

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

#define DIRECCION_WIFI "MECA-IoT-V2"
#define CONTRASENA_WIFI "IoT$2026"

#define BOT_TOKEN "8438715881:AAFf6t6TPPEJZA0VLe90II4_dTmAOmxJ0C0"
#define CHAT_ID "8189001815"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

typedef enum {
  WIFI_DESCONECTADO,
    WIFI_CONECTADO
} WIFI_t;

WIFI_t WIFI = WIFI_DESCONECTADO;

void setup() {

  Serial.begin(115200);

  switch (WIFI) {
    case WIFI_DESCONECTADO:
      WiFi.mode(WIFI_STA);
      WiFi.begin(DIRECCION_WIFI, CONTRASENA_WIFI);
      client.setCACert(TELEGRAM_CERTIFICATE_ROOT);  // Add root certificate for api.telegram.org
      if (WiFi.status() == WL_CONNECTED) {
        WIFI = WIFI_CONECTADO;
      }
      break;

    case WIFI_CONECTADO:
      bot.sendMessage(CHAT_ID, "Se activó BOT", "");
      break;
  }


}

void loop() {
  // put your main code here, to run repeatedly:
}
