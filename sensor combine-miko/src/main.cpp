#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <stdlib.h>
#include <PulseSensorPlayground.h>


BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
unsigned long previousMillis = 0;
const long interval = 150;

// TODO: add new global variables for your sensor readings and processed data

// TODO: Change the UUID to your own (any specific one works, but make sure they're different from others'). You can generate one here: https://www.uuidgenerator.net/
#define SERVICE_UUID        "64c315ca-647a-4f75-9164-b4c420e74a66"
#define CHARACTERISTIC_UUID "8f19c19f-78e1-4665-98ed-cc73f2d0f666"

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
    }
};

// TODO: add DSP algorithm functions here

// ------------------------sensor part------------------------------
const int OUTPUT_TYPE = SERIAL_PLOTTER;
const int PULSE_INPUT = A0;
const int PULSE_BLINK = LED_BUILTIN;
const int PULSE_FADE = 5;
const int THRESHOLD = 100;   // Adjust this number to avoid noise when idle
const int fsrPin = D0; 
PulseSensorPlayground pulseSensor;
// ------------------------sensor part------------------------------



void setup() {
    Serial.begin(115200);
    Serial.println("Starting BLE work!");

    // TODO: add codes for handling your sensor setup (pinMode, etc.)

    // TODO: name your device to avoid conflictions
    BLEDevice::init("miko");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    BLEService *pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    pCharacteristic->addDescriptor(new BLE2902());
    pCharacteristic->setValue("Hello World");
    pService->start();
    // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    Serial.println("Characteristic defined! Now you can read it in your phone!");

// ------------------------sensor part------------------------------
      Serial.begin(115200);

  pulseSensor.analogInput(PULSE_INPUT);
  pulseSensor.blinkOnPulse(PULSE_BLINK);
  pulseSensor.fadeOnPulse(PULSE_FADE);

  pulseSensor.setSerial(Serial);
  pulseSensor.setOutputType(OUTPUT_TYPE);
  pulseSensor.setThreshold(THRESHOLD);

  if (!pulseSensor.begin()) {
    for(;;) {
      digitalWrite(PULSE_BLINK, LOW);
      delay(50); Serial.println('!');
      digitalWrite(PULSE_BLINK, HIGH);
      delay(50);
    }
  }
  // ------------------------sensor part------------------------------
}



void loop() {
    // TODO: add codes for handling your sensor readings (analogRead, etc.)

    // TODO: use your defined DSP algorithm to process the readings

    
    if (deviceConnected) {
        // Send new readings to database
        // TODO: change the following code to send your own readings and processed data
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval) {
        pCharacteristic->setValue("Hello World");
        pCharacteristic->notify();
        Serial.println("Notify value: Hello World");
        }
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500);  // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising();  // advertise again
        Serial.println("Start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
    delay(1000);
// ------------------------sensor part------------------------------
    if(pulseSensor.UsingHardwareTimer){
  delay(20); 
  pulseSensor.outputSample();
  //int BPM = pulseSensor.getBeatsPerMinute();
} else {
  if (pulseSensor.sawNewSample()) {
    if (--pulseSensor.samplesUntilReport == (byte) 0) {
      pulseSensor.samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;
      pulseSensor.outputSample();
    }
  }
}
    if (pulseSensor.sawStartOfBeat()) {
      pulseSensor.outputBeat();
    }
if (int BPM = pulseSensor.getBeatsPerMinute()> THRESHOLD){
  pCharacteristic->setValue("Alarm");
  pCharacteristic->notify();
  Serial.println("Notify value: Alarm");
}
// ------------------------sensor part------------------------------
}