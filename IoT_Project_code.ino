#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include "DHT.h"
#include <Wire.h>
#include <SPI.h>
#include <ACROBOTIC_SSD1306.h>

#define DHTPIN 4     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

int motor1Pin3 = 35; 
int motor1Pin4 = 32; 
int enable1Pin = 14; 
const int freq = 5000;
const int pwmChannel = 0;
const int resolution = 10;
int dutyCycle = 0;
int fanSpeedPercent = 0;

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";

const char* ssid = "****";
const char* password = "***";

String serverName = "https://api.thingspeak.com/update?api_key=9YNTT4KW3AIL4615";

unsigned long lastTime = 0;
unsigned long timerDelay = 60000;

void setup() {

  // put your setup code here, to run once:
  pinMode(motor1Pin3, OUTPUT);
  pinMode(motor1Pin4, OUTPUT);
  pinMode(enable1Pin, OUTPUT);
  
  // configure LED PWM functionalitites
  ledcSetup(pwmChannel, freq, resolution);
  
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(enable1Pin, pwmChannel);

  Serial.begin(9600);
  Serial.println(F("DHTxx test!"));
  Wire.begin();  
  oled.init();                      // Initialze SSD1306 OLED display
  oled.clearDisplay();

  Blynk.begin(auth, ssid, pass);
  dht.begin();

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // put your main code here, to run repeatedly:
  float hmdt = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float temp = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(hmdt) || isnan(temp) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  Serial.print(F("Humidity: "));
  Serial.print(hmdt);
  
  Serial.print(F("%  Temperature: "));
  Serial.print(temp);

  oled.setTextXY(1,0);// Clear screen
  oled.putString("Humidity");
  oled.setTextXY(2,1);
  oled.putNumber(hmdt);
  
  oled.setTextXY(2,0);// Clear screen
  oled.putString("Temperature");
  oled.setTextXY(3,1);
  oled.putNumber(temp); 

  digitalWrite(enable1Pin, HIGH);
  
  //Move DC motor forward with increasing speed
  digitalWrite(motor1Pin3, LOW);
  digitalWrite(motor1Pin4, HIGH);

  if(temp<27) {
    digitalWrite(enable1Pin, LOW);
    delay(1000);  
    oled.setTextXY(5,1);// Clear screen
    oled.putString("Fan Speed:");
    oled.setTextXY(6,1);
    oled.putNumber(fanSpeedPercent);
  }
  
  if(temp>=27 && t<30) {
    dutyCycle = 520;
    fanSpeedPercent = 20;

    ledcWrite(pwmChannel, dutyCycle);
    Serial.println(" The fan is rotating at " + String(fanSpeedPercent) + "%");  // I have a doubt regarding this
    
    delay(1000);
    oled.setTextXY(5,1);// Clear screen
    oled.putString("Fan Speed:");
    oled.setTextXY(6,1);
    oled.putNumber(fanSpeedPercent);

  }

  if(temp>=30 && temp<33) {
    dutyCycle = 640;
    fanSpeedPercent = 40;
    ledcWrite(pwmChannel, dutyCycle);
    Serial.println(" The fan is rotating at " + String(fanSpeedPercent) + "%");
    
    delay(1000);
    oled.setTextXY(5,1);// Clear screen
    oled.putString("Fan Speed:");
    oled.setTextXY(6,1);
    oled.putNumber(fanSpeedPercent);
  }

  if(temp>=33 && temp<36) {
    dutyCycle = 760;
    fanSpeedPercent = 60;
    ledcWrite(pwmChannel, dutyCycle);
    Serial.println(" The fan is rotating at " + String(fanSpeedPercent) + "%");
   
    delay(1000);
    oled.setTextXY(5,1);// Clear screen
    oled.putString("Fan Speed:");
    oled.setTextXY(6,1);
    oled.putNumber(fanSpeedPercent);
  }

  if(temp>=36 && temp<39) {
    dutyCycle = 880;
    fanSpeedPercent = 80;
    ledcWrite(pwmChannel, dutyCycle);
    Serial.println(" The fan is rotating at " + String(fanSpeedPercent) + "%");
   
    delay(1000);
    oled.setTextXY(5,1);// Clear screen
    oled.putString("Fan Speed:");
    oled.setTextXY(6,1);
    oled.putNumber(fanSpeedPercent);
  }

  if(temp>=39) {
    dutyCycle = 1023;
    fanSpeedPercent = 100;
    ledcWrite(pwmChannel, dutyCycle);
    Serial.println(" The fan is rotating at " + String(fanSpeedPercent) + "%");
   
    delay(1000);
    oled.setTextXY(5,1);// Clear screen
    oled.putString("Fan Speed:");
    oled.setTextXY(6,1);
    oled.putNumber(fanSpeedPercent);
  }

  Blynk.run();

  Blynk.virtualWrite(V5, temp);
  Blynk.virtualWrite(V6, fanSpeedPercent);

  Serial.println("ESP32 Alert - Temperature = " + String(temp) + " °C," + " Fan Speed = " + String(fanSpeedPercent)+ "%")
  Blynk.notify("ESP32 Alert - Temperature = " + String(temp) + " °C," + " Fan Speed = " + String(fanSpeedPercent)+ "%" );    // I have a doubt regarding this
  

  if ((millis() - lastTime) > timerDelay) {
    if(WiFi.status()== WL_CONNECTED){
    // Read data from our dht object and pass it to our sendData function 
        sendData(temp, fanSpeedPercent, hmdt);
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    
   lastTime = millis();
  }
  
  
}

void sendData(double temp, int fanSpeedPercent, double hmdt){
    HTTPClient http;


    String url = serverName + "&field1=" + temp + "&field2=" + fanSpeedPercent + "&field3=" + hmdt ;
      
    http.begin(url.c_str());
      
    int httpResponseCode = http.GET();
      
    if (httpResponseCode > 0){
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }else{
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
}
