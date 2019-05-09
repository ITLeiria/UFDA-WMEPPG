/*
 * ble_config.h
 *
 *  Created on: Jun 20, 2018
 *      Author: marciofernandescalil
 */
#ifndef BLE_CONFIG_H_
#define BLE_CONFIG_H_

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#include "general_properties.h"


/***************************************************************************************
 *	 	 	 	 	 	 	 	 	 	 Define's
 ***************************************************************************************/

/**
 * DEFAULT
 *
 * Services and Characteristics Default from Heart Rate, Pulse Oximeter
 */

// Services
/*! BLEUUID Service Pulse Oximeter */
#define BLEUUID_SERVICE_PULSE_OXIMETER BLEUUID((uint16_t)0x1822)
/*! BLEUUID Service Heart Rate */
#define BLEUUID_SERVICE_HEART_RATE BLEUUID((uint16_t)0x180D)

// Characteristics
/*! BLEUUID Characteristic Pulse Oximeter */
#define BLEUUID_CHAR_PULSE_OXIMETER					BLEUUID((uint16_t)0x2A5F)
/*! BLEUUID Characteristic Heart Rate */
#define BLEUUID_CHAR_HEART_RATE						BLEUUID((uint16_t)0x2A37)
/*! BLEUUID Characteristic Descriptor */
#define BLEUUID_CHAR_DESCRIPTOR_HEART_RATE_DEF		BLEUUID((uint16_t)0x2902)
/*! BLEUUID Characteristic Body Location */
#define BLEUUID_CHAR_BODY_LOCATION					BLEUUID((uint16_t)0x2A38)
/*! BLEUUID Characteristic Descriptor */
#define BLEUUID_CHAR_DESCRIPTOR_BODY_LOCATION_DEF		BLEUUID((uint16_t)0x2901)


/**
 * CUSTOM
 *
 * Services and Characteristics Custom from Heart Rate, Pulse Oximeter and Infrared
 */

/*! BLEUUID Service Infrared */
#define IR_SERVICE BLEUUID("45647c51-8d3a-4d07-85d3-d4eea3afb5c7")

/*! BLEUUID Service Heart Rate */
#define BLEUUID_SERVICE_HEART_RATE_CUSTOM        BLEUUID("557f7f8b-bb1b-460a-a97a-13242126476b") //"0x180D"
/*! BLEUUID Characteristic Heart Rate */
#define BLEUUID_CHAR_HEART_RATE_CUSTOM			BLEUUID((uint16_t)0x2A37)	//"7d7bee52-6495-4e1e-9ff1-ed4e96c4a50f" //"0x2A37"
/*! BLEUUID Characteristic Descriptor */
#define BLEUUID_CHAR_DESCRIPTOR_HEART_RATE_CUS 	BLEUUID((uint16_t)0x2902)
/*! BLEUUID Characteristic Body Location */
#define BLEUUID_CHAR_BODY_LOCATION_CUSTOM		BLEUUID((uint16_t)0x2A38)	//"8baf1b1f-26b4-4d12-a91f-61272d9d1fdf" //"0x2A38"
/*! BLEUUID Characteristic Descriptor */
#define BLEUUID_CHAR_DESCRIPTOR_BODY_LOCATION_CUS BLEUUID((uint16_t)0x2901)

#define BLEUUID_SERVICE_PULSE_OXIMETER_CUSTOM 	"e8f18be8-fea6-4a1f-908e-741f91eda257" //"0x1822"
#define BLEUUID_CHAR_PULSE_OXIMETER_CUSTOM		BLEUUID((uint16_t)0x2A5F)	//"8ebfe178-cd47-4f2b-82ec-90e90765ed12" //"0x2A5F"



/***************************************************************************************
 *										Methods
 ***************************************************************************************/

void start_ble_service_characteristics(void);


/***************************************************************************************
 * BLE Properties
 ***************************************************************************************/
extern bool _BLEClientConnected;
extern bool connectedBefore;

/*
 * Heart Rate
 */
extern byte heart_sensor_1[8];
extern byte heart_sensor_2[8];


/*
 * Body Location
 */
extern byte body_location_sensor_1[1];
extern byte body_location_sensor_2[1];


/***************************************************************************************
 *						BLE (Server - Characteristics - Descriptor)
 ***************************************************************************************/

extern BLEServer *pServer;

// Heart Rate Characteristic and Descriptor
extern BLECharacteristic char_pulse_oximeter_continuous_measurement;

// Heart Rate Characteristics (Measurement Value and Body Location) BLE
extern BLECharacteristic char_heart_rate_measurement;
extern BLEDescriptor char_descriptor_heart_rate_measurement;
extern BLECharacteristic char_body_location;
extern BLEDescriptor char_descriptor_body_location;

// Custom Pulse Oximeter Characteristic
extern BLECharacteristic char_pulse_oximeter_continuous_measurement_custom;

// Custom Heart Rate and Body Location Characteristic
extern BLECharacteristic char_heart_rate_measurement_custom;
extern BLEDescriptor char_descriptor_heart_rate_measurement_custom;
extern BLECharacteristic char_body_location_custom;
extern BLEDescriptor char_descriptor_body_location_custom;

#endif /* BLE_CONFIG_H_ */
