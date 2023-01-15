#include <Arduino.h>
#include <string>
#include <locale>
#include <codecvt>
#include <random>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#include "UniformRandom.cpp"

#define THERMOMETER_SERVICE_UUID "d6592058-01c0-46ef-8154-dc6075fd3318"
#define THERMOMETER_CHARACTERISTICS_UUID "921a9481-2e23-43cf-9b13-b9a7c9378236"

#define DEVINFO_UUID (uint16_t)0x180a
#define DEVINFO_MANUFACTURER_UUID (uint16_t)0x2a29
#define DEVINFO_NAME_UUID (uint16_t)0x2a24
#define DEVINFO_SERIAL_UUID (uint16_t)0x2a25

#define DEVICE_MANUFACTURER "Foobar"
#define DEVICE_NAME "BLETest"

BLECharacteristic *characteristicMessage;
BLECharacteristic *characteristicThermometer;

BLEAdvertising *advertisement;

UniformRandom unformRandom(10, 40);

using namespace std;

bool deviceConnected = false;

class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *server)
    {
        deviceConnected = true;
        Serial.println("Connected");
    };

    void onDisconnect(BLEServer *server)
    {
        deviceConnected = false;
        Serial.println("Disconnected");
        advertisement->start();
    }
};

class MySecurity : public BLESecurityCallbacks {
  
  bool onConfirmPIN(uint32_t pin){
    Serial.println("onConfirmPIN");
    return true;
  }
  
  uint32_t onPassKeyRequest(){
    Serial.println("onPassKeyRequest");
        ESP_LOGI(LOG_TAG, "PassKeyRequest");
    return 133700;
  }

  void onPassKeyNotify(uint32_t pass_key){
    Serial.println("onPassKeyNotify");
        ESP_LOGI(LOG_TAG, "On passkey Notify number:%d", pass_key);
  }

  bool onSecurityRequest(){
    Serial.println("onSecurityRequest");
      ESP_LOGI(LOG_TAG, "On Security Request");
    return true;
  }

  void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl){
    Serial.println("onAuthenticationComplete");
    ESP_LOGI(LOG_TAG, "Starting BLE work!");
    if(cmpl.success){
      Serial.println("onAuthenticationComplete -> success");
      uint16_t length;
      esp_ble_gap_get_whitelist_size(&length);
      ESP_LOGD(LOG_TAG, "size: %d", length);
    }
    else{
      Serial.println("onAuthenticationComplete -> fail");
    }
  }
};

void setup()
{
    Serial.begin(9600);
    Serial.println("Setup");

    // Setup BLE Server
    BLEDevice::init(DEVICE_NAME);
    BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);

    BLEDevice::setSecurityCallbacks(new MySecurity());
    BLEServer *server = BLEDevice::createServer();
    server->setCallbacks(new MyServerCallbacks());

    BLEService *service; 

    // Registher thermometer service
    service = server->createService(THERMOMETER_SERVICE_UUID);
    characteristicThermometer = service->createCharacteristic(THERMOMETER_CHARACTERISTICS_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    BLEDescriptor *thermometerDescriptor = new BLE2902();//(BLEUUID((uint16_t)0x2902));
    thermometerDescriptor->setValue("Temperature descriptor");
    characteristicThermometer->addDescriptor(thermometerDescriptor);
    service->start();

    // Register device info service, that contains the device's UUID, manufacturer and name.
    service = server->createService(DEVINFO_UUID);
    BLECharacteristic *characteristic = service->createCharacteristic(DEVINFO_MANUFACTURER_UUID, BLECharacteristic::PROPERTY_READ);
    characteristic->setValue(DEVICE_MANUFACTURER);
    characteristic = service->createCharacteristic(DEVINFO_NAME_UUID, BLECharacteristic::PROPERTY_READ);
    characteristic->setValue(DEVICE_NAME);
    characteristic = service->createCharacteristic(DEVINFO_SERIAL_UUID, BLECharacteristic::PROPERTY_READ);
    String chipId = String((uint32_t)(ESP.getEfuseMac() >> 24), HEX);
    characteristic->setValue(chipId.c_str());
    service->start();

    // Advertise services
    advertisement = server->getAdvertising();
    BLEAdvertisementData adv;
    adv.setName(DEVICE_NAME);
    adv.setCompleteServices(BLEUUID(THERMOMETER_SERVICE_UUID));
    advertisement->setAdvertisementData(adv);

    advertisement->addServiceUUID((uint16_t)0x1101);

    advertisement->start();

    Serial.println("Ready");
}


void loop()
{
    if (deviceConnected) {
        
        int randomValue = unformRandom.generate();
        String temperature = String(randomValue);
        string value = temperature.c_str();
        characteristicThermometer->setValue(value);
        characteristicThermometer->notify();

        Serial.println("characteristicThermometer -> setValue " + temperature);
    } else {
        Serial.println("Device not connected");
    }

    delay(1000);
}

