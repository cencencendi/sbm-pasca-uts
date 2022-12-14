#include "DHT.h"
#include "WiFi.h"
#include "PubSubClient.h"

#define BUTTON_PIN 23
#define LED_PIN 32
#define DHT_PIN 4
#define DHT_TYPE DHT22 

int led_state = 0;

unsigned long now;
unsigned long previous_button_time;

DHT dht(DHT_PIN, DHT_TYPE);

const char *SSID = "suastuti";
const char *PWD = "notaristutiek";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient); 

char *mqttServer = "broker.emqx.io";
int mqttPort = 1883;

char humidData[50];
char tempData[50];

void setup() {
  Serial.begin(9600);
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);

  connectToWiFi();
  setupMQTT();
  
  dht.begin();
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), button_ISR, CHANGE);
}

void loop() {
  if (!mqttClient.connected())
    reconnect();
  mqttClient.loop();
  
  float averageReading[2];
  
  float averageTemp = 0;
  float averageHumidity = 0;
  
  for (int i=0; i<10; i++) {
    averageTemp = averageTemp + dht.readTemperature();
    averageHumidity = averageHumidity + dht.readHumidity();
    
    delay(500);
  }

  averageReading[0] = averageTemp / 10;
  averageReading[1] = averageHumidity / 10;
  
  sprintf(tempData, "Temp: %f", averageReading[0]);
  sprintf(humidData, "Hum: %f", averageReading[1]);
  
  mqttClient.publish("/sbm_te_2021/tugas_2/dht/", tempData);
  mqttClient.publish("/sbm_te_2021/tugas_2/dht/", humidData);
}

void button_ISR() {
  now = millis();

  if (now - previous_button_time > 250 && digitalRead(BUTTON_PIN)==LOW) {
    led_state = !led_state;
    digitalWrite(LED_PIN, led_state);
  }
}

void connectToWiFi() {
  Serial.print("Connecting to ");
 
  WiFi.begin(SSID, PWD);
  Serial.println(SSID);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  
  Serial.print("Connected.");
  digitalWrite(LED_BUILTIN, HIGH);
}

void reconnect() {
  Serial.println("Connecting to MQTT Broker...");
  while (!mqttClient.connected()) {
      Serial.println("Reconnecting to MQTT Broker..");
      String clientId = "ESP32Client-";
      clientId += String(random(0xffff), HEX);
      
      if (mqttClient.connect(clientId.c_str())) {
        Serial.println("Connected.");
        // subscribe to topic
        mqttClient.subscribe("/sbm_te_2021/tugas_2/#");
      }
      
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Callback - ");
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  char in = payload[0];
  
  if (in == 48) {
      digitalWrite(LED_PIN, LOW);
  } else if (in == 49) {
      digitalWrite(LED_PIN, HIGH);
  }
  
  Serial.println();
}

void setupMQTT() {
  mqttClient.setServer(mqttServer, mqttPort);
  // set the callback function
  mqttClient.setCallback(callback);
}
