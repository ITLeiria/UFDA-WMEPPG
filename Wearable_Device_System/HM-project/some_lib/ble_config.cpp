/*
 * ble_config.cpp
 *
 *  Created on: Jun 20, 2018
 *      Author: marciofernandescalil
 */

#include "ble_config.h"

// BLE CONFIG AQUI 1

/*
 * Heart Rate
 */
byte heart_sensor_1[8] = { 0b00001110, 0, 60, 0, 0 , 0, 0, 0};
byte heart_sensor_2[8] = { 0b00001110, 0, 60, 0, 0 , 0, 0, 0};


/*
 * Body Location
 *
 * B0       = UINT8 - Body Sensor Location
 *    0     = Other
 *    1     = Chest
 *    2     = Wrist
 *    3     = Finger
 *    4     = Hand
 *    5     = Ear Lobe
 *    6     = Foot
 */
byte body_location_sensor_1[1] = {1};
byte body_location_sensor_2[1] = {2};




/***************************************************************************************
 *						BLE (Server - Characteristics - Descriptor)
 ***************************************************************************************/

BLEServer *pServer = NULL;

// Heart Rate Characteristic
BLECharacteristic char_pulse_oximeter_continuous_measurement(BLEUUID_CHAR_PULSE_OXIMETER, BLECharacteristic::PROPERTY_NOTIFY);

// Heart Rate Characteristics (Measurement Value and Body Location) BLE
BLECharacteristic char_heart_rate_measurement(BLEUUID_CHAR_HEART_RATE, BLECharacteristic::PROPERTY_NOTIFY);
BLECharacteristic char_body_location(BLEUUID_CHAR_BODY_LOCATION, BLECharacteristic::PROPERTY_READ); //(BLEUUID_CHAR_BODY_LOCATION, BLECharacteristic::PROPERTY_READ);

BLEDescriptor char_descriptor_heart_rate_measurement(BLEUUID_CHAR_DESCRIPTOR_HEART_RATE_DEF);
BLEDescriptor char_descriptor_body_location(BLEUUID_CHAR_DESCRIPTOR_BODY_LOCATION_DEF);

// Custom Pulse Oximeter Characteristic
BLECharacteristic char_pulse_oximeter_continuous_measurement_custom(BLEUUID_CHAR_PULSE_OXIMETER_CUSTOM, BLECharacteristic::PROPERTY_NOTIFY);

// Custom Heart Rate and Body Location Characteristic
BLECharacteristic char_heart_rate_measurement_custom(BLEUUID_CHAR_HEART_RATE, BLECharacteristic::PROPERTY_NOTIFY);
BLECharacteristic char_body_location_custom(BLEUUID_CHAR_BODY_LOCATION, BLECharacteristic::PROPERTY_READ);
BLEDescriptor char_descriptor_heart_rate_measurement_custom(BLEUUID_CHAR_DESCRIPTOR_HEART_RATE_CUS);
BLEDescriptor char_descriptor_body_location_custom(BLEUUID_CHAR_DESCRIPTOR_BODY_LOCATION_CUS);


bool _BLEClientConnected = false;
bool connectedBefore = false;

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      _BLEClientConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      _BLEClientConnected = false;
//      if( Task_1 != NULL ) {
//    	  	  vTaskSuspend(Task_1);
//    	  	  vTaskSuspend(Task_2);
//      }

      vTaskSuspend( NULL );
    }
};


void start_ble_service_characteristics() {
  BLEDevice::init("ESP32_HR");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  /*
   *  Create the BLE Service Default
   */
  // Obtain a reference to the Pulse Oximeter service of the remote BLE server
  BLEService *pPulseOximeter = pServer->createService(BLEUUID_SERVICE_PULSE_OXIMETER);

  // Obtain a reference to the Heart Rate service of the remote BLE server
  BLEService *pHeart = pServer->createService(BLEUUID_SERVICE_HEART_RATE);


  /*
   *  Create the BLE Service Custom
   */
  // Obtain a reference to the Pulse Oximeter service of the remote BLE server
  BLEService *pPulseOximeter_custom = pServer->createService(BLEUUID_SERVICE_PULSE_OXIMETER_CUSTOM);

  // Obtain a reference to the Heart Rate service of the remote BLE server
  BLEService *pHeart_custom = pServer->createService(BLEUUID_SERVICE_HEART_RATE_CUSTOM);


//  BLEService *pIR = pServer->createService(irService);

  // Pulse Oximeter (SpO2)
  pPulseOximeter->addCharacteristic(&char_pulse_oximeter_continuous_measurement); // Default
  pPulseOximeter_custom->addCharacteristic(&char_pulse_oximeter_continuous_measurement_custom); // Custom

  // Sensor 1 Heart Rate
  pHeart->addCharacteristic(&char_heart_rate_measurement);
  char_descriptor_heart_rate_measurement.setValue("HR sensor 1 - Rate from 0 to 200");
  char_heart_rate_measurement.addDescriptor(&char_descriptor_heart_rate_measurement);
  char_heart_rate_measurement.addDescriptor(new BLE2902());

  // Sensor 1 Body Location
  pHeart->addCharacteristic(&char_body_location);
  char_descriptor_body_location.setValue("Position sensor 1 - 0 - 6");
  char_body_location.addDescriptor(&char_descriptor_body_location);
  char_body_location.setReadProperty(true);

  // Default
  pServer->getAdvertising()->addServiceUUID(BLEUUID_SERVICE_PULSE_OXIMETER);
  pServer->getAdvertising()->addServiceUUID(BLEUUID_SERVICE_HEART_RATE);

  pPulseOximeter->start();
  pHeart->start();


  // Sensor 2 Heart Rate
  pHeart_custom->addCharacteristic(&char_heart_rate_measurement_custom);
  char_descriptor_heart_rate_measurement_custom.setValue("HR sensor 2 - Rate from 0 to 200");
  char_heart_rate_measurement_custom.addDescriptor(&char_descriptor_heart_rate_measurement_custom);
  char_heart_rate_measurement_custom.addDescriptor(new BLE2902());

  // Sensor 2 Body Location
  pHeart_custom->addCharacteristic(&char_body_location_custom);
  char_descriptor_body_location_custom.setValue("Position sensor 2 - 0 - 6");
  char_body_location_custom.addDescriptor(&char_descriptor_body_location_custom);
  char_body_location_custom.setReadProperty(true);

  // Custom
  pServer->getAdvertising()->addServiceUUID(BLEUUID_SERVICE_PULSE_OXIMETER_CUSTOM);
  pServer->getAdvertising()->addServiceUUID(BLEUUID_SERVICE_HEART_RATE_CUSTOM);

  pPulseOximeter_custom->start();
  pHeart_custom->start();

  // Start advertising
  pServer->getAdvertising()->start();

}
