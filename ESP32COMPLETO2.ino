#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <NimBLEDevice.h>
#include <esp_now.h>
#include <PubSubClient.h>  // Aggiungi la libreria MQTT

uint8_t esp8266Address[] = {0xB4, 0xE6, 0x2D, 0x39, 0x77, 0xB0};

// Callback per ESP-NOW
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Messaggio inviato: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Successo" : "Fallito");
}

void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  Serial.print("Indirizzo IP ricevuto da ESP8266: ");
  Serial.write(incomingData, len);
  Serial.println();
}

#define TARGET_DEVICE_MAC "78:02:b7:48:6e:87"  // Sostituisci con l'indirizzo MAC del dispositivo
#define SERVICE_UUID1 "0000fee7-0000-1000-8000-00805f9b34fb"  // Sostituisci con l'UUID del primo servizio
#define CHARACTERISTIC_UUID1 "0000fea1-0000-1000-8000-00805f9b34fb"  // Sostituisci con l'UUID della prima caratteristica
#define SERVICE_UUID2 "000055ff-0000-1000-8000-00805f9b34fb"  // Sostituisci con l'UUID del secondo servizio
#define CHARACTERISTIC_UUID2 "000033f2-0000-1000-8000-00805f9b34fb"  // Sostituisci con l'UUID della seconda caratteristica
#define MAX_LENGTH 10
uint8_t byteArray1[MAX_LENGTH];
uint8_t byteArray2[MAX_LENGTH];
size_t byteArrayLength1 = 0;
size_t byteArrayLength2 = 0;

uint8_t selectedBytes[2];
int PassiEffettuati = 0;
int KcalConsumate = 0;

int lastPassiEffettuati = 0;
int lastKcalConsumate = 0;
int lastBattitoCardiaco = 0;
int lastPressioneSistolica = 0;
int lastPressioneDiastolica = 0;
int BattitoC = 0;
int PressioneS = 0;
int PressioneD = 0;

String output;

NimBLEClient* pClient = nullptr;
bool isConnected = false;

// WebSocket e MQTT
WebSocketsServer webSocket = WebSocketsServer(80);
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);  // Client MQTT

class MyClientCallback : public NimBLEClientCallbacks {
  void onConnect(NimBLEClient* pclient) override {
    Serial.println("Connected to the device");
    isConnected = true;
  }

  void onDisconnect(NimBLEClient* pclient) override {
    Serial.println("Disconnected from the device");
    isConnected = false;
  }
};

void notifyCallback1(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  String hexValue = "";
  byteArrayLength1 = 0;  // Reset the length of the byte array
  for (int i = 0; i < length; i++) {
    char c = pData[i];
    if (c < 16) hexValue += '0';
    hexValue += String(c, HEX);

    // Save the characteristic value into the byte array
    if (byteArrayLength1 < MAX_LENGTH) {
      byteArray1[byteArrayLength1++] = c;
    }

    // If the current byte is the second or third byte, save it into the selected bytes array
    if (i == 1) {
      selectedBytes[1] = c;  // Save the second byte as the second element in the selected bytes array
    } else if (i == 2) {
      selectedBytes[0] = c;  // Save the third byte as the first element in the selected bytes array
    }

    // If the current byte is the 8th byte, save it into the KcalConsumate variable
    if (i == 7) {
      KcalConsumate = c;
    }
  }

  // Convert the selected bytes to a decimal value
  PassiEffettuati = (selectedBytes[0] << 8) | selectedBytes[1];

  if (PassiEffettuati != 0) {
    lastPassiEffettuati = PassiEffettuati;
  }

  if (KcalConsumate != 0) {
    lastKcalConsumate = KcalConsumate;
  }
}

void notifyCallback2(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  byteArrayLength2 = 0;  // Reset the length of the byte array
  String hexValue = "";
  int BattitoCardiaco = 0;  // Create a variable to store the decimal value of the remaining bytes
  int PressioneSistolica = 0;  // Create a variable to store the decimal value of the 4th byte
  int PressioneDiastolica = 0;  // Create a variable to store the decimal value of the 5th byte

  for (int i = 0; i < length; i++) {
    char c = pData[i];
    if (c < 16) hexValue += '0';
    hexValue += String(c, HEX);

    // Save the characteristic value into the byte array
    if (byteArrayLength2 < MAX_LENGTH) {
      byteArray2[byteArrayLength2++] = c;
    }

    // If the first two bytes are 0xE5 and 0x11, convert the remaining bytes to a decimal value
    if (i >= 2 && byteArray2[0] == 0xE5 && byteArray2[1] == 0x11) {
      BattitoCardiaco = (BattitoCardiaco << 8) | c;
    }

    // If the first two bytes are 0xC7 and 0x00, save the 4th and 5th bytes as decimal values
    if (byteArray2[0] == 0xC7 && byteArray2[1] == 0x00) {
      if (i == 3) {
        PressioneSistolica = c;
      } else if (i == 4) {
        PressioneDiastolica = c;
      }
    }
  }

  if (BattitoCardiaco != 0) {
    lastBattitoCardiaco = BattitoCardiaco;
  }

  if (PressioneSistolica != 0) {
    lastPressioneSistolica = PressioneSistolica;
  }

  if (PressioneDiastolica != 0) {
    lastPressioneDiastolica = PressioneDiastolica;
  }

  BattitoC = BattitoCardiaco;  
  PressioneS = PressioneSistolica;  
  PressioneD = PressioneDiastolica; 
}
// Credenziali di rete
const char* ssid1 = "iPhone di Andrea (6)";
const char* password1 = "Andrea10.";
const char* ssid2 = "AndroidAP2581";
const char* password2 = "Mariella";

// Informazioni per la connessione MQTT (se connesso alla seconda rete)
const char* mqttServer = "mqtt-dashboard.com";
const int mqttPort = 1883;
const char* mqttTopic = "testtopic/andrea";

void setup() {
  Serial.begin(115200);
  NimBLEDevice::init("");

  // Connessione WiFi
  WiFi.begin(ssid1, password1);
  int retryCount = 0;
  while (WiFi.status() != WL_CONNECTED && retryCount < 30) {
    delay(1000);
    Serial.println("Connessione WiFi...");
    retryCount++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connesso a Wi-Fi (prima rete) con IP: ");
    Serial.println(WiFi.localIP());

    // Inizializza ESP-NOW
    if (esp_now_init() != ESP_OK) {
      Serial.println("Errore nell'inizializzazione di ESP-NOW");
      return;
    }
  
    esp_now_register_send_cb(onDataSent);
    esp_now_register_recv_cb(onDataRecv);

    // Aggiungi il peer prima di inviare
    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, esp8266Address, 6); 
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      Serial.println("Errore nell'aggiunta del peer");
      return;
    }

    String ipStr = WiFi.localIP().toString();

    // Invia l'indirizzo IP all'ESP8266
    esp_err_t result = esp_now_send(esp8266Address, (uint8_t *)ipStr.c_str(), ipStr.length());
    if (result == ESP_OK) {
      Serial.println("Indirizzo IP inviato con successo");
    } else {
      Serial.print("Errore nell'invio dell'indirizzo IP, codice: ");
      Serial.println(result);
    }

    // Inizia la comunicazione WebSocket
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);


  } else {
    // Prova a connetterti alla seconda rete
    Serial.println("Connessione alla prima rete fallita, tentando la seconda rete...");
    WiFi.begin(ssid2, password2);
    retryCount = 0;
    while (WiFi.status() != WL_CONNECTED && retryCount < 30) {
      delay(1000);
      Serial.println("Connessione WiFi alla seconda rete...");
      retryCount++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Connesso alla seconda rete!");
      Serial.print("ESP32 IP address: ");
      Serial.println(WiFi.localIP());

      // Inizia la comunicazione MQTT
      mqttClient.setServer(mqttServer, mqttPort);
      if (!mqttClient.connect("clientId-7H6uqniSer")) {
        Serial.println("Connessione al broker MQTT fallita");
        return;
      }
      Serial.println("Connesso al broker MQTT");

    } else {
      Serial.println("Impossibile connettersi a nessuna rete WiFi");
      return;
    }
  }

  // Configura il client BLE...
  pClient = NimBLEDevice::createClient();
  MyClientCallback* pClientCallback = new MyClientCallback();
  pClient->setClientCallbacks(pClientCallback);


}

void loop() {


    if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connessione Wi-Fi persa. Tentativo di riconnessione...");

    WiFi.begin(ssid1, password1);
  int retryCount = 0;
  while (WiFi.status() != WL_CONNECTED && retryCount < 10) {
    delay(1000);
    Serial.println("Connessione WiFi...");
    retryCount++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connesso a Wi-Fi (prima rete) con IP: ");
    Serial.println(WiFi.localIP());
    // Inizializza ESP-NOW
    if (esp_now_init() != ESP_OK) {
      Serial.println("Errore nell'inizializzazione di ESP-NOW");
      return;
    }
  
    esp_now_register_send_cb(onDataSent);
    esp_now_register_recv_cb(onDataRecv);

    // Aggiungi il peer prima di inviare
    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, esp8266Address, 6); 
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;

    String ipStr = WiFi.localIP().toString();

    // Invia l'indirizzo IP all'ESP8266
    esp_err_t result = esp_now_send(esp8266Address, (uint8_t *)ipStr.c_str(), ipStr.length());
    if (result == ESP_OK) {
      Serial.println("Indirizzo IP inviato con successo");
    } else {
      Serial.print("Errore nell'invio dell'indirizzo IP, codice: ");
      Serial.println(result);
    }

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      Serial.println("Errore nell'aggiunta del peer");
      return;
    }



    // Inizia la comunicazione WebSocket
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);


  } else {
    // Prova a connetterti alla seconda rete
    Serial.println("Connessione alla prima rete fallita, tentando la seconda rete...");
    WiFi.begin(ssid2, password2);
    retryCount = 0;
    while (WiFi.status() != WL_CONNECTED && retryCount < 10) {
      delay(1000);
      Serial.println("Connessione WiFi alla seconda rete...");
      retryCount++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Connesso alla seconda rete!");
      Serial.print("ESP32 IP address: ");
      Serial.println(WiFi.localIP());

      // Inizia la comunicazione MQTT
      mqttClient.setServer(mqttServer, mqttPort);
      if (!mqttClient.connect("clientId-7H6uqniSer")) {
        Serial.println("Connessione al broker MQTT fallita");
        return;
      }
      Serial.println("Connesso al broker MQTT");

    } else {
      Serial.println("Impossibile connettersi a nessuna rete WiFi");
      return;
    }
  }

  }


  if (WiFi.SSID() == ssid1) {
    webSocket.loop();
  } else if (WiFi.SSID() == ssid2) {
    if (!mqttClient.connected()) {
      Serial.println("MQTT disconnesso, tentando la riconnessione...");
      while (!mqttClient.connect("clientId-7H6uqniSer")) {
        delay(1000);
      }
    }
    mqttClient.loop();
  }

  if (!isConnected) {
    if (pClient->connect(NimBLEAddress(TARGET_DEVICE_MAC))) {
      Serial.println("Connesso al dispositivo target");

      // Ottieni il primo servizio specificato
      NimBLERemoteService* pRemoteService1 = pClient->getService(SERVICE_UUID1);
      if (pRemoteService1) {
        // Ottieni la prima caratteristica specificata
        NimBLERemoteCharacteristic* pRemoteCharacteristic1 = pRemoteService1->getCharacteristic(CHARACTERISTIC_UUID1);
        if (pRemoteCharacteristic1) {
          // Abilita le notifiche sulla caratteristica
          if (pRemoteCharacteristic1->canNotify()) {
            pRemoteCharacteristic1->subscribe(true, notifyCallback1);
          }
        } else {
          Serial.println("Caratteristica 1 non trovata");
        }
      } else {
        Serial.println("Servizio 1 non trovato");
      }

      // Ottieni il secondo servizio specificato
      NimBLERemoteService* pRemoteService2 = pClient->getService(SERVICE_UUID2);
      if (pRemoteService2) {
        // Ottieni la seconda caratteristica specificata
        NimBLERemoteCharacteristic* pRemoteCharacteristic2 = pRemoteService2->getCharacteristic(CHARACTERISTIC_UUID2);
        if (pRemoteCharacteristic2) {
          // Abilita le notifiche sulla caratteristica
          if (pRemoteCharacteristic2->canNotify()) {
            pRemoteCharacteristic2->subscribe(true, notifyCallback2);
          }
        } else {
          Serial.println("Caratteristica 2 non trovata");
        }
      } else {
        Serial.println("Servizio 2 non trovato");
      }    } else {
      Serial.println("Connessione al dispositivo target fallita");
    }
  }

  // Crea un documento JSON
  StaticJsonDocument<200> doc;
  JsonArray records = doc.createNestedArray("records");
   JsonObject record = records.createNestedObject();
  // Aggiungi dati al documento
  record["PassiEffettuati"] = PassiEffettuati != 0 ? PassiEffettuati : lastPassiEffettuati;
  record["KcalConsumate"] = KcalConsumate != 0 ? KcalConsumate : lastKcalConsumate;
  record["BattitoCardiaco"] = BattitoC != 0 ? BattitoC : lastBattitoCardiaco;
  record["PressioneSistolica"] = PressioneS != 0 ? PressioneS : lastPressioneSistolica;
  record["PressioneDiastolica"] = PressioneD != 0 ? PressioneD : lastPressioneDiastolica;

  serializeJson(doc, output);
  Serial.println(output);

  if (WiFi.SSID() == ssid1) {
    // Invio tramite WebSocket
    //webSocket.broadcastTXT(output);
  } else if (WiFi.SSID() == ssid2) {
    // Invio tramite MQTT
    mqttClient.publish(mqttTopic, output.c_str());
    delay(4000);
  }
}

// Callback WebSocket
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_TEXT:
      Serial.printf("[%u] Messaggio ricevuto: %s\n", num, payload);
      if (strcmp((char *)payload, "messaggio_specifico") == 0) {
        webSocket.sendTXT(num, output);
      }
      break;
  }
}
