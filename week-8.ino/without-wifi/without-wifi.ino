#include "DHT.h"

#define BUTTON_PIN 23
#define LED_PIN 32
#define DHT_PIN 4
#define DHT_TYPE DHT22 

unsigned long now;
unsigned long previous_button_time;

int led_state = 0;

DHT dht(DHT_PIN, DHT_TYPE);

void setup() {
  Serial.begin(9600);
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(LED_BUILTIN, 0);
  digitalWrite(LED_PIN, LOW);
  
  dht.begin();
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), button_ISR, CHANGE);
}

void loop() {
  float averageReading[2];
  
  float averageTemp = 0;
  float averageHumidity = 0;

  if (Serial.available()) {
    char in = Serial.read();

    if (in == 48) {
      digitalWrite(LED_PIN, LOW);
    } else {
      digitalWrite(LED_PIN, HIGH);
    }
  }
  
  for (int i=0; i<10; i++) {
    averageTemp = averageTemp + dht.readTemperature();
    averageHumidity = averageHumidity + dht.readHumidity();
    
    delay(100);
  }

  averageReading[0] = averageTemp / 10;
  averageReading[1] = averageHumidity / 10;

  Serial.print("Temp: ");
  Serial.println(averageReading[0]);

  Serial.print("Hum: ");
  Serial.println(averageReading[1]);
}

void button_ISR() {
  now = millis();

  if (now - previous_button_time > 250 && digitalRead(BUTTON_PIN)==LOW) {
    led_state = !led_state;
    digitalWrite(LED_PIN, led_state);
  }
}
