#include <NimBLEDevice.h>
#include <inttypes.h>

#define LED_PIN 2
#define BUTTON_PIN 0
int buttonState = 0;
static NimBLEServer *pServer;
static NimBLECharacteristic *pButtonCharacteristic;
static NimBLECharacteristic *pLedCharacteristic;
static NimBLECharacteristic *pSecureButtonCharacteristic;

std::string serviceUUID = "b7e6e95f-b754-421a-93ef-efb5db537daa";
std::string ledCharacteristicUUID = "b7e6e95f-b754-421a-93ef-efb5db537da1";
std::string buttonCharacteristicUUID = "b7e6e95f-b754-421a-93ef-efb5db537db1";
std::string secureButtonCharacteristicUUID = "b7e6e95f-b754-421a-93ef-efb5db537dc1";

class ServerCallbacks : public NimBLEServerCallbacks
{
    void onConnect(NimBLEServer *pServer)
    {
        Serial.println("Client connected");
    };

    void onConnect(NimBLEServer *pServer, ble_gap_conn_desc *desc)
    {
        Serial.print("Client address: ");
        Serial.println(NimBLEAddress(desc->peer_ota_addr).toString().c_str());

        pServer->updateConnParams(desc->conn_handle, 24, 48, 0, 60);
    };
    void onDisconnect(NimBLEServer *pServer)
    {
        Serial.println("Client disconnected - start advertising");
        NimBLEDevice::startAdvertising();
    };
    void onMTUChange(uint16_t MTU, ble_gap_conn_desc *desc)
    {
        Serial.printf("MTU updated: %u for connection ID: %u\n", MTU, desc->conn_handle);
    };

    /********************* Security handled here **********************
    ****** Note: these are the same return values as defaults ********/
    uint32_t onPassKeyRequest()
    {
        Serial.println("Server Passkey Request");
        /** This should return a random 6 digit number for security
         *  or make your own static passkey as done here.
         */
        return 123456;
    };

    bool onConfirmPIN(uint32_t pass_key)
    {
        Serial.print("The passkey YES/NO number: ");
        Serial.println(pass_key);
        /** Return false if passkeys don't match. */
        return true;
    };

    void onAuthenticationComplete(ble_gap_conn_desc *desc)
    {
        /** Check that encryption was successful, if not we disconnect the client */
        if (!desc->sec_state.encrypted)
        {
            NimBLEDevice::getServer()->disconnect(desc->conn_handle);
            Serial.println("Encrypt connection failed - disconnecting client");
            return;
        }
        Serial.println("BLE conneciton encrypted!");
    };
};

class CharacteristicCallbacks : public NimBLECharacteristicCallbacks
{
    void onRead(NimBLECharacteristic *pCharacteristic)
    {
        Serial.print(pCharacteristic->getUUID().toString().c_str());
        Serial.print(" - onRead() called");
    };

    void onWrite(NimBLECharacteristic *pCharacteristic)
    {
        Serial.print(pCharacteristic->getUUID().toString().c_str());
        Serial.println(" - onWrite() called");
        if (pCharacteristic->getUUID().toString() == ledCharacteristicUUID)
        {
            Serial.println("Changing LED State");
            digitalWrite(LED_PIN, *(pCharacteristic->getValue().data()));
        }
    };
    /** Called before notification or indication is sent,
     *  the value can be changed here before sending if desired.
     */
    void onNotify(NimBLECharacteristic *pCharacteristic)
    {
        Serial.println("Sending notification to clients");
    };

    /** The status returned in status is defined in NimBLECharacteristic.h.
     *  The value returned in code is the NimBLE host return code.
     */
    void onStatus(NimBLECharacteristic *pCharacteristic, Status status, int code)
    {
        String str = ("Notification/Indication status code: ");
        str += status;
        str += ", return code: ";
        str += code;
        str += ", ";
        str += NimBLEUtils::returnCodeToString(code);
        Serial.println(str);
    };

    void onSubscribe(NimBLECharacteristic *pCharacteristic, ble_gap_conn_desc *desc, uint16_t subValue)
    {
        String str = "Client ID: ";
        str += desc->conn_handle;
        str += " Address: ";
        str += std::string(NimBLEAddress(desc->peer_ota_addr)).c_str();
        if (subValue == 0)
        {
            str += " Unsubscribed to ";
        }
        else if (subValue == 1)
        {
            str += " Subscribed to notfications for ";
        }
        else if (subValue == 2)
        {
            str += " Subscribed to indications for ";
        }
        else if (subValue == 3)
        {
            str += " Subscribed to notifications and indications for ";
        }
        str += std::string(pCharacteristic->getUUID()).c_str();

        Serial.println(str);
    };
};

static CharacteristicCallbacks customCharacteristicCallback;

void setup()
{
    Serial.begin(115200);
    Serial.println("Starting NimBLE Server");

    pinMode(LED_PIN, OUTPUT);          // use the LED as an output
    pinMode(BUTTON_PIN, INPUT_PULLUP); // use button pin as an input

    NimBLEDevice::init("WebBluetooth");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */
    NimBLEDevice::setSecurityAuth(true, true, true);
    NimBLEDevice::setSecurityPasskey(123456);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_YESNO);

    pServer = NimBLEDevice::createServer();
    NimBLEService *pService = pServer->createService(serviceUUID);
    pServer->setCallbacks(new ServerCallbacks());

    pLedCharacteristic = pService->createCharacteristic(ledCharacteristicUUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    pButtonCharacteristic = pService->createCharacteristic(buttonCharacteristicUUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    pSecureButtonCharacteristic = pService->createCharacteristic(secureButtonCharacteristicUUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::READ_AUTHEN);

    pLedCharacteristic->setValue(0);
    pButtonCharacteristic->setValue(0);
    pSecureButtonCharacteristic->setValue(0);

    pLedCharacteristic->setCallbacks(&customCharacteristicCallback);
    pButtonCharacteristic->setCallbacks(&customCharacteristicCallback);
    pSecureButtonCharacteristic->setCallbacks(&customCharacteristicCallback);

    pService->start();

    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(serviceUUID);
    pAdvertising->start();
    Serial.println("Advertising Started");
}

void loop()
{
    if (digitalRead(BUTTON_PIN) != buttonState)
    {
        buttonState = !buttonState;
        Serial.println(buttonState);
        pButtonCharacteristic->setValue(buttonState);
        pButtonCharacteristic->notify(true);
        pSecureButtonCharacteristic->setValue(buttonState);
        pSecureButtonCharacteristic->notify(true);
    }
}