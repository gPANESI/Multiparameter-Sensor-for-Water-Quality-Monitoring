//Incluindo todas as bibliotecas a serem utilizadas
#include <Wire.h>
#include <EEPROM.h>
#include "GravityTDS.h"
#include <WiFi.h>


#define TRIG_PIN 17
#define ECHO_PIN 18
#define PH_PIN 19 
#define TdsSensorPin 16

GravityTDS gravityTds;

const float V_REF  = 3.3;       
const int   ADC_RANGE = 4095;   
const int KVALUE_EEPROM_ADDRESS = 4;
float temperature = 25;
float tdsValue = 0;
float calibration_value = 21.34 - 0.7;
int buffer_arr[10];
float ph_act;
unsigned long int avgval;
int temp;
float Nivel_Minimo = 1500;

void setup() {
  //Iniciar o monitor serial
  Serial.begin(115200);

  Wire.begin();
  EEPROM.begin(32);

  // Desliga WiFi para estabilidade do ADC
  WiFi.mode(WIFI_OFF);

  //Definir todas as variaveis
  //Sensor de Nivel
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  //Inicia o TDS
  gravityTds.setPin(TdsSensorPin);
  gravityTds.setAref(V_REF);
  gravityTds.setAdcRange(ADC_RANGE);
  gravityTds.setKvalueAddress(KVALUE_EEPROM_ADDRESS);
  gravityTds.begin();  

}

void loop() {
  Sensor_TDS();
  Sensor_Ph();
  Sensor_Nivel();
  Serial.println(" ");
}

void Sensor_Ph(){
  // Captura de 10 leituras para média  
  for(int i = 0; i < 10; i++) { 
    buffer_arr[i] = analogRead(PH_PIN);
    delay(30);
  }

  // Ordenação simples  
  for(int i = 0; i < 9; i++) {
    for(int j = i + 1; j < 10; j++) {
      if(buffer_arr[i] > buffer_arr[j]) {
        temp = buffer_arr[i];
        buffer_arr[i] = buffer_arr[j];
        buffer_arr[j] = temp;
      }
    }
  }

  // Média das leituras centrais  
  avgval = 0;
  for(int i = 2; i < 8; i++) avgval += buffer_arr[i];

  float volt = (float)avgval * 3.3 / 4095.0 / 6.0; // Conversão ADC ESP32
  ph_act = -5.70 * volt + calibration_value;

  // Impressão no Serial Monitor  
  Serial.print("pH atual: ");
  Serial.println(ph_act, 2);

  delay(1000);}

void Sensor_TDS(){
  gravityTds.setTemperature(temperature);
  gravityTds.update();

  tdsValue = gravityTds.getTdsValue()/2.42;

  Serial.print("TDS atual: ");
  Serial.print(tdsValue, 0); 
  Serial.println(" ppm");

  delay(1000);}

void Sensor_Nivel(){
  long duracao;
  float distancia;

  // Pulso de trigger
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Timeout ajustado para 2 metros (11600 µs)
  duracao = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duracao == 0) {
    Serial.println("Sem pulso");
    delay(200);
    return;
  }

  // Converte para cm
  distancia = duracao / 58.0;

  // Faixa válida do sensor
  if (distancia < 25) {
    Serial.println("Objeto muito próximo (zona cega)");
  }
  else if (distancia > 2000) {
    Serial.println("Fora do range (> 2 m)");
  }
  else {
    Serial.print("Distancia: ");
    Serial.print(distancia);
    Serial.println(" cm");
  }

  delay(300);}