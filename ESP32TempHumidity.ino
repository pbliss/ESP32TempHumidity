#include "DHTesp.h"
#include "SSD1306.h"
#include<stdlib.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID        "a441bfd5-0c59-484a-a7c3-dfa6d03eba10"
#define CHARACTERISTIC_UUID "a341b2c6-817f-4ce5-be54-3374c6ff9af5"

DHTesp dht;
SSD1306  display(0x3c, 5, 4);
BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

class BLEServerCallBacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class BLECallBacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
    }
};

void setup() {
  Serial.begin(115200);

  //setup the screen
  display.init();
  display.flipScreenVertically();

  display.clear();
  
  display.setColor(WHITE);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_16);

  //start the sensor
  dht.setup(25, DHTesp::DHT11);

  //setup ble server
  BLEDevice::init("HumidityTempSensor");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new BLEServerCallBacks());;
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
                                       
 
  pCharacteristic->setCallbacks(new BLECallBacks());                             
  pService->start();                                     
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void loop() {
  display.clear();
  
  display.setColor(WHITE);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_16);

  //draw temp
  float temperature = dht.getTemperature();
  char t[10];
  String temp = "Temp ";
  temperature = temperature * 1.8 + 32;
  dtostrf(temperature,5,1,t);
  display.drawString(64, 0, temp + t + 'F');
  Serial.print("Temperature: ");
  Serial.println(temperature);

  //draw humidity
  float humidity = dht.getHumidity();
  char h[10];
  String hum = "Humidity: ";
  dtostrf(humidity,5,1,h);
  display.drawString(64, 26, hum + h);
  Serial.print("Humidity: ");
  Serial.println(humidity);
  
  display.display();
  
  if(deviceConnected) {
      Serial.println("Device Connected");
      
      char message[200];
      sprintf (message, "%s:%s", h, t);

      pCharacteristic->setValue(message);
      
      pCharacteristic->notify(); // Send the value to the app!
      Serial.print("*** Sent Value: ");
      //Serial.print(message);
      Serial.println(" ***");
  }
 
  delay(1000);
}

