#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Crypto.h>
#include <ChaCha.h>
#include "esp_timer.h"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// Define your service and characteristic UUIDs
#define SERVICE_UUID           "36b29c3e-5fd5-488e-b915-4d04e4d61e7f"
#define CHARACTERISTIC_UUID_RX "2d3c4928-5e6f-4f93-9d83-08a278a4b729"
#define CHARACTERISTIC_UUID_TX "2d3c4928-5e6f-4f93-9d83-08a278a4b729"

// ChaCha20 encryption key and nonce
const uint8_t key[32] = { /* 32-byte key */ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                          0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                          0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                          0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F };
const uint8_t nonce[12] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++)
          Serial.print(rxValue[i]);

        Serial.println();
        Serial.println("*********");
      }
    }
};

void setup() {
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init("UART Service");

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic for TX
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
                    );

  pCharacteristic->addDescriptor(new BLE2902());

  // Create a BLE Characteristic for RX
  BLECharacteristic *pCharacteristicRx = pService->createCharacteristic(
                                           CHARACTERISTIC_UUID_RX,
                                           BLECharacteristic::PROPERTY_WRITE
                                         );

  pCharacteristicRx->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting for a client connection to notify...");
}

void loop() {
  if (deviceConnected) {
    // Generate random temperature between 38 and 41
    float temperature = 38 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (41 - 38)));
    
    // Generate random steps, with most values between 150 and 300
    int steps = 150 + rand() % 151; // Random value between 150 and 300

    // Convert the temperature and steps to a string
    String datastring = "Id: 1, Temp: " + String(temperature, 1) + " C, Steps: " + String(steps);

    // Print the temperature and steps
    Serial.printf("*** Sent Value: Temp: %.1f C, Steps: %d ***\n", temperature, steps);

    // Measure encryption time
    int64_t startEncrypt = esp_timer_get_time();
    
    // Encrypt the data
    ChaCha chacha;
    chacha.setKey(key, sizeof(key));
    chacha.setIV(nonce, sizeof(nonce));
    
    // Prepare the buffer for encryption
    size_t len = datastring.length();
    uint8_t plainText[len];
    uint8_t cipherText[len];
    memcpy(plainText, datastring.c_str(), len);

    // Encrypt the data
    chacha.encrypt(cipherText, plainText, len);

    int64_t endEncrypt = esp_timer_get_time();
    Serial.printf("Encryption Time: %lld us\n", endEncrypt - startEncrypt);

    // Measure notification time
    int64_t startNotify = esp_timer_get_time();
    
    // Set the value of the characteristic
    pCharacteristic->setValue(cipherText, len);

    // Notify the connected device
    pCharacteristic->notify();

    int64_t endNotify = esp_timer_get_time();
    Serial.printf("Notification Time: %lld us\n", endNotify - startNotify);
  }
  
  // Wait for 5 seconds before sending the next random value
  delay(1000*60*5);
}
