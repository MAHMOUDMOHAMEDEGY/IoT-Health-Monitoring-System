#include <ESP8266WiFi.h>       // Libreria per gestire la connessione WiFi
#include <WebSocketsClient.h>  // Libreria per gestire la connessione WebSocket
#include <ArduinoJson.h>       // Libreria per gestire i file JSON
#include <WiFiClientSecureBearSSL.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
extern "C" {
#include <espnow.h>
}

uint8_t esp32Address[] = { 0xD0, 0xEF, 0x76, 0x44, 0xCC, 0x3C };
String esp32IpAddress = "";

const char* ssid = "iPhone di Andrea (6)";
const char* password = "Andrea10.";


const char* mqttServer = "mqtt-dashboard.com";
const int mqttPort = 1883;
const char* mqttTopic = "testtopic/andrea";

// Crea un'istanza di WiFiClient e PubSubClient
WiFiClient espClient1;
PubSubClient mqttclient(espClient1);

static const char* awsEndpoint = "a1v69d0m0sa5ro-ats.iot.us-east-1.amazonaws.com";

static const char* rootCA = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";

// Device Certificate
static const char* certificate = R"KEY(
-----BEGIN CERTIFICATE-----
MIIDWjCCAkKgAwIBAgIVALwoi/REHmdhhozVPZjBRb+q1T8HMA0GCSqGSIb3DQEB
CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t
IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0yNDA3MDUxNzI0
MjJaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh
dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDFDJbW3C3BwBav1+uU
7ulnK64Imm85HBPw+6hQduf6C+dfyCSoKbls2xTgr7ffMavSgJ87NYnuoj9pfZNd
9WCkqcjs+hEbJzrcR5wxA4QV7hdwvGwQXITH2b1Ocl2A1EXpKStFXuntTsqRxuZE
D0K5YBoful+7NU4mtsNCSVL7+Uq9XkZCvZYNGvA/5rmM4SnXI0X4eCcPnXWnMlP7
13vQIN6R8L1bmuapMS5QbgwodPWEWhUb+lUf5K7h6/VuDidnML79+dXSCb0bcvjw
75pqHdEskbEKem9w55lrudXq9tCQiZIY7b2PEUqB6cWYJETuz/B23DF1VL+tXYtv
HBwPAgMBAAGjYDBeMB8GA1UdIwQYMBaAFBiXLRC1QM81Cmo4OsAlmJwpjKe3MB0G
A1UdDgQWBBSKk8GR7guzq2fCzH/nwUO1GwVKLTAMBgNVHRMBAf8EAjAAMA4GA1Ud
DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEAWEoMGuo48a/pAQerkCuBClT2
sud9laUy4vKa938iI/COdpJI80bj5+1f/7+FxQmAinhfX/ibA6uPmeXsA2dQnsR+
iJd4+/qH6AziOlB4/6VVfLz9YGlscQKG2ViwBT6QAKTVRV71IQGTc8VKttJ0Jz2U
oxTS+Pw5jlDbYYi1DBFkCeJLYnNH1+Y0/sDfQ832hOcODCR+twjxektA2kX9+k7H
Ah3X2dGM9bXd4b+QR7h7C2gXpIh63IUdCucnbY9HoWyPjP3gsAiKNj1U3gYRPuPB
tXiUdJ6J2fM3crfKoWa7hhao8vZjPZDfj+9Yyf13Bh1vXadXBKuYkkBort3fQw==
-----END CERTIFICATE-----
)KEY";

// Device Private key
static const char* privateKey = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
MIIEowIBAAKCAQEAxQyW1twtwcAWr9frlO7pZyuuCJpvORwT8PuoUHbn+gvnX8gk
qCm5bNsU4K+33zGr0oCfOzWJ7qI/aX2TXfVgpKnI7PoRGyc63EecMQOEFe4XcLxs
EFyEx9m9TnJdgNRF6SkrRV7p7U7KkcbmRA9CuWAaH7pfuzVOJrbDQklS+/lKvV5G
Qr2WDRrwP+a5jOEp1yNF+HgnD511pzJT+9d70CDekfC9W5rmqTEuUG4MKHT1hFoV
G/pVH+Su4ev1bg4nZzC+/fnV0gm9G3L48O+aah3RLJGxCnpvcOeZa7nV6vbQkImS
GO29jxFKgenFmCRE7s/wdtwxdVS/rV2LbxwcDwIDAQABAoIBAGwe1EVg2tqNA+XI
nENENF2f4gZmdtDnTynTbC96jx7smTA9KZ/8BPpt267NvB6Dqrv+R2C9p8mAzTqc
5NeRZE31u4IMVIaJqApmYJWkUD2YPtRlDwaLPXBLUUxSBGxEDXg6VrvhsIm9yfjj
IkYF+Z6t1KfEI/Mc9JzZe5pLkXUgBh6M3lkZJco4FnprWddz+o4sCvBLDawe83oF
luUAJAeXX2TiSCzddjiPfhw56oRW2dvsZhJ4ZcpBQNFejoelOtq16bLV8g9wDfNo
q0E0AjofG763su3/CMsGEgEg2EPdH5c284ffQnlm1vX2hqFBO/iZ26CllUbpmxgL
G7I6X8ECgYEA4k8Th6SDdiXW9Ru/RgqPC6UUZwunCY/X35guWKrqjChAhTUyveuO
3IoyMc2KTGF6A+JqfcyZUmID2vh+dfogSp1HsRIPVPXg5UF9bhUUkMSM1dHSvD+5
8zKWsrUNcN5IbC1YpgSt09u55bxS+2iwJnaKy0zRXbWNwG6bLtzreMMCgYEA3ubI
D7uBCbma4pO127Ypqui2s3CPv6XXBDJ9OfHvqZtEBch71x8ig4APDmy8LSNKO+91
0CgJMEjZLJ/oLG3sItceKMwyOm3FbFumKR2Vau8yK/M9Q6/p11oseRYa3zxBP0fd
S/6zrhEh2yQlw5SaAxAQJkkSlmEwDzyqL/CEOsUCgYB/H8hqs6EdJxhey11grOG5
utuBHuyP1HuBIG04ZZonbR0BNubwBHhVrlOP4lPDoCX4NKF5VQrzWhesU1ZGU2D8
SOlSsEBoi03vuiAJTFInGCG8oobsNCfyUwKQFGAefN7V1YsshwhWL+F3CZjbnO0G
TiSdGuOo7ilYZUlKw9KKHQKBgBi8kc01KM/UejzU2aTFZYBjDQuC3WEOXXtIwx7w
G4G+CmF960hnWyQuzPzz0jpMJUvbej6cgtCJ9Rf/svtjQ4ZmSyGJ77UOQ4+P6DO9
5bwVSYMZHl5polDU4AScEGVfwXntVsC9RmF140T3kP1Qe3sFiFVHXLm1lWjLebOS
RIplAoGBANDDqqhNLdnYycaTanN1S3b82bPNUZX5roSb1bEIF4NM6Qo/ZT9K8Dlb
VBSNh4M6c8wgt5Sm9swZpTXbXQrLUkdrdwqk8GCfE96qyxqHnqptAhSZAmPUvgSf
4YFIhqbUt5kNP7jG1Kj23LzhB9OEDnChuAB0FSNjmAG+6c3tA9VG
-----END RSA PRIVATE KEY-----
)KEY";

BearSSL::WiFiClientSecure espClient;
PubSubClient client(awsEndpoint, 8883, espClient);
WebSocketsClient webSocket;

// Gestiamo il tempo di report di ESP8266
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;               // invia un messaggio ogni 5 secondi
const unsigned long minTimerDelay = 625;       // valore minimo del timerDelay
const unsigned long defaultTimerDelay = 5000;  // valore predefinito del timerDelay

int battiti_cardiaci[10];     // array per memorizzare le misurazioni del battito cardiaco
int passi[10];                // array per memorizzare le misurazioni dei passi
int count = 0;                // contatore per le misurazioni
unsigned long startTime = 0;  // tempo di inizio della raccolta delle misurazioni
bool allarmeAttivo = false;   // stato dell'allarme



void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      Serial.printf("[WSc] Connesso al server: %s\n", payload);
      break;
    case WStype_TEXT:
      Serial.printf("[WSc] Messaggio ricevuto: %s\n", payload);

      // Analizza il payload JSON
      DynamicJsonDocument doc(2048);  // aumentato la dimensione del buffer
      DeserializationError error = deserializeJson(doc, payload);

      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
      }

      // Verifica se l'indice è nei limiti
      if (count < 10) {
        battiti_cardiaci[count] = doc["BattitoCardiaco"];  // memorizza la misurazione del battito cardiaco
        passi[count] = doc["PassiEffettuati"];             // memorizza la misurazione dei passi
        Serial.printf("Battiti cardiaci: %d, Passi: %d\n", battiti_cardiaci[count], passi[count]);

        // Se è la prima misurazione, inizia a misurare il tempo
        if (count == 0) {
          startTime = millis();
        }
        count++;
      }

      // Se abbiamo raccolto 10 misurazioni, calcola il tempo impiegato e i passi al secondo
      if (count == 10) {
        unsigned long endTime = millis();
        unsigned long timeTaken = endTime - startTime + timerDelay;  // tempo totale in millisecondi
        float timeTakenInSeconds = timeTaken / 1000.0;               // converte il tempo in secondi

        doc["Allarme"] = "Nessun Allarme";

        // Calcola il totale dei passi
        int totalPassi = passi[9] - passi[0];

        float passiPerSecondo = totalPassi / timeTakenInSeconds;  // calcola i passi al secondo
        Serial.printf("Tempo impiegato per raccogliere 10 misurazioni: %lu ms\n", timeTaken);
        Serial.printf("Totale passi: %d, Passi al secondo: %.2f\n", totalPassi, passiPerSecondo);

        // Calcola la media dei battiti cardiaci
        int sommaBattiti = 0;
        for (int i = 0; i < 10; i++) {
          sommaBattiti += battiti_cardiaci[i];
        }
        float mediaBattiti = sommaBattiti / 10.0;
        Serial.printf("Media battiti cardiaci: %.2f\n", mediaBattiti);

        bool nuovoAllarme = false;

        if (mediaBattiti > 100 && passiPerSecondo < 2) {
          // Crea un oggetto JSON con una stringa
          doc["Allarme"] = "Probabile Tachicardia";
          nuovoAllarme = true;
        }

        if (mediaBattiti < 50 && totalPassi != 0) {
          // Crea un oggetto JSON con una stringa
          doc["Allarme"] = "Probabile Brachicardia";
          nuovoAllarme = true;
        }

        // Calcola la deviazione standard dei battiti cardiaci
        float sommaDifferenzeQuadrato = 0;
        for (int i = 0; i < 10; i++) {
          sommaDifferenzeQuadrato += pow(battiti_cardiaci[i] - mediaBattiti, 2);
        }
        float deviazioneStandardBattiti = sqrt(sommaDifferenzeQuadrato / 10);
        Serial.printf("Deviazione standard battiti cardiaci: %.2f\n", deviazioneStandardBattiti);

        if (deviazioneStandardBattiti > 20 && passiPerSecondo < 2) {
          doc["Allarme"] = "Probabile Aritmia";
          nuovoAllarme = true;
        }

        if (nuovoAllarme) {
          // Dimezza il timerDelay se non ha già raggiunto il valore minimo
          if (timerDelay > minTimerDelay) {
            timerDelay = max(timerDelay / 2, minTimerDelay);
          }
          allarmeAttivo = true;
        } else if (allarmeAttivo) {
          // Ripristina il timerDelay al valore predefinito se non ci sono più allarmi
          timerDelay = defaultTimerDelay;
          allarmeAttivo = false;
        }

        // Serializza l'oggetto JSON e stampalo a video
        String output;
        if (serializeJson(doc, output) == 0) {
          Serial.println("Failed to serialize JSON");
        } else {
          Serial.println(output);
          if (client.connected()) {
            client.publish("iot/health", output.c_str());
          }
        }
        count = 0;  // resetta il contatore per una nuova raccolta
      }
      break;
  }
}



void onDataSent(uint8_t* mac_addr, uint8_t sendStatus) {
  Serial.print("Messaggio inviato: ");
  Serial.println(sendStatus == 0 ? "Successo" : "Fallito");
}

void onDataRecv(uint8_t* mac_addr, uint8_t* incomingData, uint8_t len) {
  // Converti i dati ricevuti in una stringa e salva in esp32IpAddress
  esp32IpAddress = String((char*)incomingData).substring(0, len);

  Serial.print("Indirizzo IP ricevuto da ESP32: ");
  Serial.println(esp32IpAddress);

  webSocket.begin(esp32IpAddress, 80);
  webSocket.onEvent(webSocketEvent);
}



void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("Esp32")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void reconnectmqtt() {
  // Loop fino a quando non siamo connessi
  while (!mqttclient.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Crea un client ID casuale
    String mqttclientId = "ESP8266Client-";
    mqttclientId += String(random(0xffff), HEX);

    // Prova a connetterti
    if (mqttclient.connect(mqttclientId.c_str())) {
      Serial.println("connected");

      // Iscrivi ai topic desiderati
      mqttclient.subscribe(mqttTopic);  // Sostituisci con il tuo topic
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttclient.state());
      Serial.println(" try again in 5 seconds");

      // Attendi 5 secondi prima di ritentare
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");

  // Stampa il messaggio ricevuto
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  DynamicJsonDocument doc(2048);  // aumentato la dimensione del buffer
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

  // Verifica se l'indice è nei limiti
  if (count < 10) {
    battiti_cardiaci[count] = doc["BattitoCardiaco"];  // memorizza la misurazione del battito cardiaco
    passi[count] = doc["PassiEffettuati"];             // memorizza la misurazione dei passi
    Serial.printf("Battiti cardiaci: %d, Passi: %d\n", battiti_cardiaci[count], passi[count]);

    // Se è la prima misurazione, inizia a misurare il tempo
    if (count == 0) {
      startTime = millis();
    }
    count++;
  }

  // Se abbiamo raccolto 10 misurazioni, calcola il tempo impiegato e i passi al secondo
  if (count == 10) {
    unsigned long endTime = millis();
    unsigned long timeTaken = endTime - startTime + timerDelay;  // tempo totale in millisecondi
    float timeTakenInSeconds = timeTaken / 1000.0;               // converte il tempo in secondi

    doc["Allarme"] = "Nessun Allarme";

    // Calcola il totale dei passi
    int totalPassi = passi[9] - passi[0];

    float passiPerSecondo = totalPassi / timeTakenInSeconds;  // calcola i passi al secondo
    Serial.printf("Tempo impiegato per raccogliere 10 misurazioni: %lu ms\n", timeTaken);
    Serial.printf("Totale passi: %d, Passi al secondo: %.2f\n", totalPassi, passiPerSecondo);

    // Calcola la media dei battiti cardiaci
    int sommaBattiti = 0;
    for (int i = 0; i < 10; i++) {
      sommaBattiti += battiti_cardiaci[i];
    }
    float mediaBattiti = sommaBattiti / 10.0;
    Serial.printf("Media battiti cardiaci: %.2f\n", mediaBattiti);

    bool nuovoAllarme = false;

    if (mediaBattiti > 100 && passiPerSecondo < 2) {
      // Crea un oggetto JSON con una stringa
      doc["Allarme"] = "Probabile Tachicardia";
      nuovoAllarme = true;
    }

    if (mediaBattiti < 50 && totalPassi != 0) {
      // Crea un oggetto JSON con una stringa
      doc["Allarme"] = "Probabile Brachicardia";
      nuovoAllarme = true;
    }

    // Calcola la deviazione standard dei battiti cardiaci
    float sommaDifferenzeQuadrato = 0;
    for (int i = 0; i < 10; i++) {
      sommaDifferenzeQuadrato += pow(battiti_cardiaci[i] - mediaBattiti, 2);
    }
    float deviazioneStandardBattiti = sqrt(sommaDifferenzeQuadrato / 10);
    Serial.printf("Deviazione standard battiti cardiaci: %.2f\n", deviazioneStandardBattiti);

    if (deviazioneStandardBattiti > 20 && passiPerSecondo < 2) {
      doc["Allarme"] = "Probabile Aritmia";
      nuovoAllarme = true;
    }

    if (nuovoAllarme) {
      // Dimezza il timerDelay se non ha già raggiunto il valore minimo
      if (timerDelay > minTimerDelay) {
        timerDelay = max(timerDelay / 2, minTimerDelay);
      }
      allarmeAttivo = true;
    } else if (allarmeAttivo) {
      // Ripristina il timerDelay al valore predefinito se non ci sono più allarmi
      timerDelay = defaultTimerDelay;
      allarmeAttivo = false;
    }

    // Serializza l'oggetto JSON e stampalo a video
    String output;
    if (serializeJson(doc, output) == 0) {
      Serial.println("Failed to serialize JSON");
    } else {
      Serial.println(output);
      if (client.connected()) {
        client.publish("iot/health", output.c_str());
      }
    }
    count = 0;  // resetta il contatore per una nuova raccolta
  }
}



void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != 0) {
    Serial.println("Errore nell'inizializzazione di ESP-NOW");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(onDataRecv);
  esp_now_add_peer(esp32Address, ESP_NOW_ROLE_COMBO, 1, NULL, 0);


  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connessione WiFi...");
  }
  Serial.println("Connesso!");

  setup_wifi();

  Serial.print("Resolving hostname: ");
  Serial.println(awsEndpoint);

  IPAddress ip;
  if (WiFi.hostByName(awsEndpoint, ip)) {
    Serial.print("MQTT broker IP address: ");
    Serial.println(ip);
  } else {
    Serial.println("Failed to resolve hostname");
  }

  // Set up the client (same as before)
  espClient.setInsecure();
  espClient.setClientRSACert(new BearSSL::X509List(certificate), new BearSSL::PrivateKey(privateKey));


  mqttclient.setServer(mqttServer, mqttPort);
  mqttclient.setCallback(callback);

  WiFi.setSleepMode(WIFI_MODEM_SLEEP);
}

void loop() {

  if (!mqttclient.connected()) {
    reconnectmqtt();
  }

  if ((millis() - lastTime) > timerDelay) {
    // invia un messaggio
    webSocket.sendTXT("messaggio_specifico");
    lastTime = millis();
      Serial.println("Going to sleep (Modem-Sleep)...");
    // Gestisci la connessione e i messaggi MQTT
  }

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  mqttclient.loop();


  webSocket.loop();




  // Nel loop gestiamo il tempo in cui deve essere mandato il messaggio di trigger ad ESP8266
}