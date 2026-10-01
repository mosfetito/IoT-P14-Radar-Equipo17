#include "config.h" 
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

//config de pantalla y hardware en general
#define ANCHO_PANTALLA 128
#define ALTO_PANTALLA 64
Adafruit_SSD1306 oled(ANCHO_PANTALLA, ALTO_PANTALLA, &Wire, -1);
Servo miServo;

//pines

const int pinTrig   = 26;
const int pinEcho   = 25;
const int pinVerde  = 32;
const int pinRojo   = 18;
const int pinBuzzer = 27;
const int pinServo  = 14;
const int pinBotonReset = 13;

enum EstadoRadar { BARRIENDO_IDA, BARRIENDO_VUELTA, MODO_ALERTA };
EstadoRadar estadoActual = BARRIENDO_IDA;

int anguloActual = 0;
float ultimaDistancia = 0.0; 

unsigned long tiempoAnterior = 0;
const unsigned long T_ASENTAMIENTO_MS = 200; 
const unsigned long T_PAUSA_ALERTA = 300;

unsigned long ultimoDebounceBoton = 0;
bool estadoBotonAnterior = HIGH;

// configuracion red wifi

WiFiClient espClient;
PubSubClient mqtt(espClient);

String clientId, topicDatos, topicEstado, topicCmd;

unsigned long tWiFi = 0;
unsigned long tReconexMQTT = 0;
unsigned long tPublicacion = 0;
const unsigned long PERIODO_PUB_MS = 5000;
const unsigned long ESPERA_RECONEX = 5000;

//Funciones del radar local

float medirDistancia() {
  digitalWrite(pinTrig, LOW);
  delayMicroseconds(2);
  digitalWrite(pinTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinTrig, LOW);

  long duracion = pulseIn(pinEcho, HIGH, 25000); 
  if (duracion == 0) return 400.0;
  return (duracion * 0.0343 / 2.0);
}

void reiniciarSistema() {
  noTone(pinBuzzer);
  digitalWrite(pinRojo, LOW);
  digitalWrite(pinVerde, LOW);

  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(10, 20);
  oled.println("REINICIANDO...");
  oled.display();

  anguloActual = 0;
  miServo.write(0);
  estadoActual = BARRIENDO_IDA;
  delay(1000); 
}

void procesarAnguloFSM(int angulo, float distancia) {
  ultimaDistancia = distancia; 

  if (distancia <= 8.0) {
    digitalWrite(pinRojo, HIGH);
    digitalWrite(pinVerde, LOW);
    oled.clearDisplay();
    oled.setCursor(0, 0);
    
    if (distancia <= 2.0) {
      oled.println("OBSTACULO MUY CERCA");
      tone(pinBuzzer, 600);
    } else {
      oled.println("! ALERTA DETECTADA !");
      tone(pinBuzzer, 2000);
    }
    
    oled.setCursor(0, 20);
    oled.print("Ang:"); oled.print(angulo); oled.print((char)247);
    oled.setCursor(0, 45);
    oled.print("Dist: "); oled.print(distancia, 1); oled.print(" cm");
    oled.display();
  } else {
    digitalWrite(pinRojo, LOW);
    digitalWrite(pinVerde, HIGH);
    noTone(pinBuzzer);
    oled.clearDisplay();
    oled.setCursor(0, 0);
    oled.println("ESCANEO RADAR 180");
    oled.setCursor(0, 20);
    oled.print("Angulo actual: "); oled.print(angulo); oled.print((char)247);
    oled.setCursor(0, 40);
    oled.print("Distancia: ");
    if (distancia >= 400) oled.print("Limpio");
    else { oled.print(distancia, 1); oled.print(" cm"); }
    oled.display();
  }
}

//Funciones de conectividad no bloqueante

void mantenerWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  unsigned long ahora = millis();
  if (ahora - tWiFi < ESPERA_RECONEX) return;
  tWiFi = ahora;
  WiFi.reconnect();
}

void mantenerMQTT() {
  if (mqtt.connected()) return;
  if (WiFi.status() != WL_CONNECTED) return;
  
  unsigned long ahora = millis();
  if (ahora - tReconexMQTT < ESPERA_RECONEX) return;
  tReconexMQTT = ahora;

  if (mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASS, topicEstado.c_str(), 1, true, "offline")) {
    mqtt.publish(topicEstado.c_str(), "online", true); 
    mqtt.subscribe(topicCmd.c_str());                  
  }
}

void publicarDatosJson() {
  if (!mqtt.connected()) return;

  JsonDocument doc;
  doc["angulo"] = anguloActual;
  
  if (ultimaDistancia >= 400.0) {
      doc["distancia"] = 400.0;
  } else {
      doc["distancia"] = roundf(ultimaDistancia * 10.0f) / 10.0f;
  }

  char payload[128];
  size_t n = serializeJson(doc, payload, sizeof(payload));

  mqtt.publish(topicDatos.c_str(), (uint8_t*)payload, n, true);
}

//setup y loop
void setup() {
  Serial.begin(115200);

  pinMode(pinTrig, OUTPUT);
  pinMode(pinEcho, INPUT);
  pinMode(pinVerde, OUTPUT);
  pinMode(pinRojo, OUTPUT);
  pinMode(pinBuzzer, OUTPUT);
  pinMode(pinBotonReset, INPUT_PULLUP);

  ESP32PWM::allocateTimer(0);
  miServo.setPeriodHertz(50);
  miServo.attach(pinServo, 500, 2400);

  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for(;;);
  }
  
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(0, 10);
  oled.println("Iniciando...");
  oled.setCursor(0, 30);
  oled.println("Conectando WiFi");
  oled.display();

  clientId    = String(MQTT_USER) + "-" + NODO; 
  topicDatos  = String("curso/") + MQTT_USER + "/" + PROYECTO + "/" + NODO;
  topicEstado = topicDatos + "/estado";
  topicCmd    = topicDatos + "/cmd";

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long inicio = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - inicio < 10000) {
    delay(200);
  }

  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setBufferSize(512);

  oled.clearDisplay();
  oled.setCursor(0, 30);
  oled.println("Radar Listo!");
  oled.display();
  delay(1000);
}

void loop() {
  unsigned long tiempoActual = millis();

  mantenerWiFi();
  mantenerMQTT();
  mqtt.loop(); 

  if (tiempoActual - tPublicacion >= PERIODO_PUB_MS) {
    tPublicacion = tiempoActual;
    publicarDatosJson();
  }

  bool lecturaBoton = digitalRead(pinBotonReset);
  if (lecturaBoton == LOW && estadoBotonAnterior == HIGH && (millis() - ultimoDebounceBoton > 200)) {
    ultimoDebounceBoton = millis();
    reiniciarSistema();
  }
  estadoBotonAnterior = lecturaBoton;

  switch (estadoActual) {
    case BARRIENDO_IDA:
      if (tiempoActual - tiempoAnterior >= T_ASENTAMIENTO_MS) {
        tiempoAnterior = tiempoActual;
        miServo.write(anguloActual);
        float distancia = medirDistancia();
        procesarAnguloFSM(anguloActual, distancia);

        if (distancia <= 8.0) estadoActual = MODO_ALERTA;
        else if (anguloActual >= 180) estadoActual = BARRIENDO_VUELTA;
        else anguloActual += 2;
      }
      break;

    case BARRIENDO_VUELTA:
      if (tiempoActual - tiempoAnterior >= T_ASENTAMIENTO_MS) {
        tiempoAnterior = tiempoActual;
        miServo.write(anguloActual);
        float distancia = medirDistancia();
        procesarAnguloFSM(anguloActual, distancia);

        if (distancia <= 8.0) estadoActual = MODO_ALERTA;
        else if (anguloActual <= 0) estadoActual = BARRIENDO_IDA;
        else anguloActual -= 2;
      }
      break;

    case MODO_ALERTA:
      if (tiempoActual - tiempoAnterior >= T_PAUSA_ALERTA) {
        tiempoAnterior = tiempoActual;
        estadoActual = BARRIENDO_IDA;
      }
      break;
  }
}