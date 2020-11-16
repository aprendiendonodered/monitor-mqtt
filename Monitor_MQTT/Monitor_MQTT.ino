/*
   Monitor para monitorizar temperaturas y puerta
   Manda datos cada 1 minuto
*/

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <dht.h>

#define DHT22_PIN1 8
#define DHT22_PIN2 9

// Update these with values suitable for your network.
byte mac[]    = {  0x90, 0xBB, 0x00, 0x0, 0x00, 0x00 };
IPAddress ip(192, 168, 1, 10);
IPAddress server(192, 168, 1, 11);  //mosquitto

EthernetClient ethClient;
PubSubClient client(ethClient);

long lastMsg = 0;
char msg[50];

const int pin_puerta = 7;
boolean puerta_anterior;

dht DHT1_frio;
dht DHT2_frio;

void setup() {
  Serial.begin(57600);

  client.setServer(server, 1883);
  client.setCallback(callback);

  pinMode(pin_puerta, INPUT);
  puerta_anterior = digitalRead(pin_puerta);

  Ethernet.begin(mac, ip);
  // Allow the hardware to sort itself out
  delay(3000);
  Serial.println(Ethernet.localIP());
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 60000) {
    lastMsg = now;

    // READ DATA
    Serial.println("Leo Sondas");

    Serial.println("Sonda 1");
    int chk1 = DHT1_frio.read22(DHT22_PIN1);
    if (chk1 == DHTLIB_OK) {
      float temp = DHT1_frio.temperature;
      float hum = DHT1_frio.humidity;
      Serial.println(temp);
      Serial.println(hum);
      //sprintf (msg, "Temp: %f", temp);
      dtostrf(temp, 4, 2, msg);
      Serial.print("Publish message: ");
      Serial.println(String(msg));
      client.publish("Temperatura/Sonda1", msg);
      //sprintf (msg, "Hum: %f", hum);
      dtostrf(hum, 4, 2, msg);
      Serial.print("Publish message: ");
      Serial.println(String(msg));
      client.publish("Humedad/Sonda1", msg);
    }
    else {
      Serial.println("Error Sonda 1");
      client.publish("Temperatura/Sonda1", "err");
      client.publish("Humedad/Sonda1", "err");
    }

    delay(500);
    Serial.println("Sonda 2");
    int chk2 = DHT2_frio.read22(DHT22_PIN2);
    if (chk2 == DHTLIB_OK) {
      float temp = DHT2_frio.temperature;
      float hum = DHT2_frio.humidity;
      Serial.println(temp);
      Serial.println(hum);
      //snprintf (msg, 50, " % f", temp);
      dtostrf(temp, 4, 2, msg);
      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish("Temperatura/Sonda2", msg);
      //snprintf (msg, 50, " % f", hum);
      dtostrf(hum, 4, 2, msg);
      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish("Humedad/Sonda2", msg);
    }
    else {
      Serial.println("Error Sonda 2");
      client.publish("Temperatura/Sonda2", "err");
      client.publish("Humedad/Sonda2", "err");
    }
  }

  boolean puerta = digitalRead(pin_puerta);

  if (puerta != puerta_anterior) {
    puerta_anterior = puerta;
    if (puerta == LOW) { //se ha abierto la puerta
      client.publish("Puerta", "Abierta", true);
      Serial.println("Abierta");
    }
    else { //se ha cerrado la puerta
      client.publish("Puerta", "Cerrada", true);
      Serial.println("Cerrada");
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
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient2", "user", "password")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("reset", "reset node");
      //actualizo el estado de la puerta en el reset o perdida de conexión
      boolean puerta = digitalRead(pin_puerta);
      if (puerta == LOW) { //se ha abierto la puerta
        client.publish("Puerta", "Abierta", true);
        Serial.println("Abierta");
      }
      else { //se ha cerrado la puerta
        client.publish("Puerta", "Cerrada", true);
        Serial.println("Cerrada");
      }
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc = ");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
