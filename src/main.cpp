#include <Arduino.h>
#include <string>
#include <locale>
#include <codecvt>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define MESSAGE_SERVICE_UUID "ab0828b1-198e-4351-b779-901fa0e0371e"
#define MESSAGE_CHARACTERISTICS_UUID "4ac8a682-9736-4e5d-932b-e9b31405049c"

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

// BLEDescriptor thermometerDescriptor(BLEUUID((uint16_t)0x2902));

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

class MessageCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        std::string data = characteristic->getValue();
        Serial.println(data.c_str());
    }

    void onRead(BLECharacteristic *characteristic)
    {
        characteristic->setValue("Foobar");
    }
};

void setup()
{
    Serial.begin(9600);
    Serial.println("Setup");

    // Setup BLE Server
    BLEDevice::init(DEVICE_NAME);
    BLEServer *server = BLEDevice::createServer();
    server->setCallbacks(new MyServerCallbacks());

    BLEService *service; // = server->createService(MESSAGE_SERVICE_UUID);

    // Register message service that can receive messages and reply with a static message.
    // characteristicMessage = service->createCharacteristic(MESSAGE_CHARACTERISTICS_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE);
    // characteristicMessage->setCallbacks(new MessageCallbacks());
    // characteristicMessage->addDescriptor(new BLE2902());
    // service->start();

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
    // adv.setCompleteServices(BLEUUID(MESSAGE_SERVICE_UUID));
    advertisement->setAdvertisementData(adv);

    advertisement->addServiceUUID((uint16_t)0x1101); 
    // advertisement->addServiceUUID((uint16_t)0x1812); // iOS fix
    // advertisement->addServiceUUID((uint16_t)0x181A);
   

    advertisement->start();

    Serial.println("Ready");
}

int loopCounter = 0;
void loop()
{
    // String message = "Loop " + String(loopCounter);
    // Serial.println(message);
    loopCounter += 1;

    if (deviceConnected) {
        String temperature = String(loopCounter);
        string value = temperature.c_str();
        //Set temperature Characteristic value and notify connected client
        characteristicThermometer->setValue(value);
        characteristicThermometer->notify();
        Serial.print("Temperature Celsius: ");
        Serial.print(temperature);
        Serial.print(" ºC\n\r");
    } else {
        Serial.println("Device not connected");
    }

    delay(1000);
}