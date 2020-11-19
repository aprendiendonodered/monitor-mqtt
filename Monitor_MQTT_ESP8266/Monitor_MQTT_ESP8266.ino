/*
   Monitor para monitorizar temperaturas y humedad
   Montado en tres sondas basadas en Wemos D1 Mini con shield DHT22
   Sensores: 3 sondas distribuidas
   - Sonda 1 MAC: ¿?
   - Sonda 2 MAC: ¿?
   - Sonda 3 MAC: ¿?

   Control por MQTT y Node-RED. Topics:
   SondaX/Temperatura
   SondaX/Humedad
   Manda datos cada 1 minuto

   Basado en: https://github.com/jecrespo/Curso-Node-RED/blob/master/Remote%20Nodes%20Firmware/relay-node/relay-node.ino
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <dhtnew.h>  //compilado con https://github.com/RobTillaart/DHTNew

#define sonda 3 //valores: 1,2,3 - Cambiar también el id en la linea 142
#define DHT22_PIN D3

#define STASSID "SSID" //Oculto y acceso por MAC
#define STAPSK  "password"

const char* ssid     = STASSID;
const char* password = STAPSK;

// Use WiFiClient class to create TCP connections
WiFiClient wificlient;

// Update these with values suitable for your network.
IPAddress server(10, 1, 1, 11);  //mosquitto

PubSubClient client(wificlient);

long lastMsg = 0;
char msg[50];

DHTNEW DHT(D3);

void setup() {
  Serial.begin(57600);

  Serial.println();
  Serial.println();
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    Serial.println("Reconectando cliente MQTT...");
    reconnect();
  }

  client.loop();

  long now = millis();
  if (now - lastMsg > 60000) {  //envio datos cada minuto
    lastMsg = now;

    // READ DATA
    Serial.println("Leo Sonda");
    String topic_t = "Sonda" + String(sonda) + "/Temperatura";
    String topic_h = "Sonda" + String(sonda) + "/Humedad";
    int chk1 = DHT.read();
    if (chk1 == DHTLIB_OK) {
      float temp = DHT.getTemperature();
      float hum = DHT.getHumidity();
      Serial.println(temp);
      Serial.println(hum);
      //sprintf (msg, "Temp: %f", temp);
      dtostrf(temp, 4, 2, msg);
      Serial.print("Publish message: ");
      Serial.println(String(msg));
      Serial.print("Topic: ");
      Serial.println(topic_t.c_str());
      client.publish(topic_t.c_str(), msg);
      //sprintf (msg, "Hum: %f", hum);
      dtostrf(hum, 4, 2, msg);
      Serial.print("Publish message: ");
      Serial.println(String(msg));
      Serial.print("Topic: ");
      Serial.println(topic_h.c_str());
      client.publish(topic_h.c_str(), msg);
    }
    else {
      Serial.println("Error Sonda");
      client.publish(topic_t.c_str(), "err");
      client.publish(topic_h.c_str(), "err");
    }
  }
}

void callback(char* topic, byte * payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected

  while (!client.connected()) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Wifi Desconectado");
      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("Wifi Conectado");
    }

    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("clientid", "user", "password")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      String topic_r = "Sonda" + String(sonda) + "/reset";
      client.publish(topic_r.c_str(), "reset node");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
