/*--------------------------------------------------------------------------
 *                           UFDA-WMEPPG v1.0
 *                          Copyright (C) 2019
 *
 *--------------------------------------------------------------------------
 *  This ESP-IDF file is part of the project UFDA-WMEPPG:
 *  a Wearable Device System (WMEPPG), developed in ESP-IDF (language C),
 *	Optimized for collection and pre-processing the vital signs data from
 *	multiple PO sensors.
 *
 *  https://github.com/ITLeiria/UFDA-WMEPPG/tree/master/Wearable_Device_System
 *
 *-CITATION-----------------------------------------------------------------
 *  If you use this software please cite one of the following papers:
 *
 *
 *-DESCRIPTION--------------------------------------------------------------
 *
 * ble_defines.h
 *
 *-DISCLAIMER---------------------------------------------------------------
 *  This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  You can use this source code without licensing fees only for NON-COMMERCIAL research
 *  and EDUCATIONAL purposes only.
 *  You cannot repost this file without prior written permission from the authors.
 *
 *-AUTHORS------------------------------------------------------------------
 *  Márcio Fernandes Calil*
 *  Ihor Koval*
 *  Luís Marcelino*
 *  Luís Conde Bento* **
 *  Sérgio Faria*
 *
 *  *School of Technology and Management - Polytechnic Institute of Leiria
 *  **Institute of Systems and Robotics   - University of Coimbra
 *--------------------------------------------------------------------------
 */

#ifndef MAIN_BLE_DEFINES_H_
#define MAIN_BLE_DEFINES_H_


#define GATTS_TAG 					"GATT_Server"
//#define TEST_DEVICE_NAME            "HM_BLE_Koval"
#define TEST_DEVICE_NAME            "HP-platform Wearable Module"

#define CHAR2_FLAGS			HEART_RATE_8BIT | SENSOR_CONTACT_NOT_DETECTED | ENERGY_EXPENDED_NOT_PRESENT | RR_INTERVAL_NOT_PRESENT
#define CHAR5_FLAGS			HEART_RATE_8BIT | SENSOR_CONTACT_NOT_DETECTED | ENERGY_EXPENDED_NOT_PRESENT | RR_INTERVAL_NOT_PRESENT

#define CHAR3_FLAGS			0x0
#define CHAR6_FLAGS			0x0

static uint32_t ble_add_char_pos;
static uint8_t n_notify = 0;
static bool notify_task_running = false;
static bool battery_task_running = false;

uint8_t *raw_ptr0_IR,*raw_ptr0_RED,*raw_ptr1_IR,*raw_ptr1_RED;
static uint8_t body_location1_str[] = {FINGER}; 									//Body Location:
static uint8_t HR1_str[] = {CHAR2_FLAGS,0,0,100,0};	//Heart Rate, value: 111bpm , ->3601 Kj expended Energy
static uint8_t PLX1_str[] = {CHAR3_FLAGS,0,0,0,0};		//Pulse Measurement , value: 99 , 11-> data for testing
static uint8_t body_location2_str[] = {WRIST};							 			//Body Location:
static uint8_t HR2_str[] = {CHAR5_FLAGS,0,3602&0x0F,3602&0xF0,0}; 	//Heart Rate, value: 111bpm , ->3602 Kj expended Energy
static uint8_t PLX2_str[] = {CHAR6_FLAGS,0,0,0,0};		//Pulse Measurement
static uint8_t BAT_lvl_str[] = {99};								//Battery level %
static uint8_t BAT_state_str[] = {BATT_STATE_PRESENT|BATT_STATE_DISCHARGING|BATT_STATE_NOT_CHARGING|BATT_STATE_GOOD_LEVEL};								//Battery state


esp_attr_value_t char1_BL_val = {
	.attr_max_len = 22,
	.attr_len		= sizeof(body_location1_str),
	.attr_value     = body_location1_str,
};

esp_attr_value_t char2_HR1_val = {
	.attr_max_len = 22,
	.attr_len		= sizeof(HR1_str),
	.attr_value     = HR1_str,
};

esp_attr_value_t char3_PLX1_val = {
	.attr_max_len = 22,
	.attr_len		= sizeof(PLX1_str),
	.attr_value     = PLX1_str,
};

esp_attr_value_t char4_BL2_val = {
	.attr_max_len = 22,
	.attr_len		= sizeof(body_location2_str),
	.attr_value     = body_location2_str,
};

esp_attr_value_t char5_HR2_val = {
	.attr_max_len = 22,
	.attr_len		= sizeof(HR2_str),
	.attr_value     = HR2_str,
};

esp_attr_value_t char6_PLX2_val = {
	.attr_max_len = 22,
	.attr_len		= sizeof(PLX2_str),
	.attr_value     = PLX2_str,
};

esp_attr_value_t char7_RAW1_val = {
	.attr_max_len = FIFO_A_FULL*8,
	.attr_len		= FIFO_A_FULL,
	.attr_value     = NULL,
};
esp_attr_value_t char8_RAW2_val = {
	.attr_max_len = FIFO_A_FULL*8,
	.attr_len		= FIFO_A_FULL,
	.attr_value     = NULL,
};
esp_attr_value_t char9_BAT_lvl_val = {
	.attr_max_len = 22,
	.attr_len		= sizeof(BAT_lvl_str),
	.attr_value     = BAT_lvl_str,
};
esp_attr_value_t char10_BAT_state_val = {
	.attr_max_len = 22,
	.attr_len		= sizeof(BAT_state_str),
	.attr_value     = BAT_state_str,
};

static uint8_t descr1_str[] = {0,0};

struct gatts_char_inst {
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t char_perm;
	esp_gatt_char_prop_t char_property;
	esp_attr_value_t *char_val;
    esp_attr_control_t *char_control;
    uint16_t char_handle;
    esp_gatts_cb_t char_read_callback;
	esp_gatts_cb_t char_write_callback;
    esp_bt_uuid_t descr_uuid;
    esp_gatt_perm_t descr_perm;
	esp_attr_value_t *descr_val;
    esp_attr_control_t *descr_control;
    uint16_t descr_handle;
    esp_gatts_cb_t descr_read_callback;
	esp_gatts_cb_t descr_write_callback;
	bool is_notify;						//notify flag
};

//***************DESCRIPTORS**********************//
esp_attr_value_t gatts_demo_descr1_val = {
		.attr_max_len = 22,
		.attr_len		= sizeof(descr1_str),
		.attr_value     = descr1_str,
};

static uint8_t adv_config_done = 0;
#define adv_config_flag      (1 << 0)
#define scan_rsp_config_flag (1 << 1)

#ifdef CONFIG_SET_RAW_ADV_DATA
static uint8_t raw_adv_data[] = {
		0x02, 0x01, 0x06,
		0x02, 0x0a, 0xeb, 0x03, 0x03, 0xab, 0xcd
};
static uint8_t raw_scan_rsp_data[] = {
		0x0f, 0x09, 0x45, 0x53, 0x50, 0x5f, 0x47, 0x41, 0x54, 0x54, 0x53, 0x5f, 0x44,
		0x45, 0x4d, 0x4f
};
#else

static uint8_t adv_service_uuid128[32] = {
		/* LSB <--------------------------------------------------------------------------------> MSB */
		//first uuid, 16bit, [12],[13] is the value
		0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xEE, 0x00, 0x00, 0x00,
		//second uuid, 32bit, [12], [13], [14], [15] is the value
		0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,
};

static uint8_t heart_rate_service_uuid[48] = {
		/* LSB <--------------------------------------------------------------------------------> MSB */
		//first uuid, 16bit, [12],[13] is the value
		0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x0D, 0x18, 0x00, 0x00,
		//second uuid, 32bit, [12], [13], [14], [15] is the value
		0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x37, 0x2A, 0x00, 0x00,
		//third uuid, 32bit, [12], [13], [14], [15] is the value
		0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x38, 0x2A, 0x00, 0x00,
};
// The length of adv data must be less than 31 bytes
//static uint8_t test_manufacturer[TEST_MANUFACTURER_DATA_LEN] =  {0x12, 0x23, 0x45, 0x56};
//adv data
static esp_ble_adv_data_t adv_data = {
		.set_scan_rsp = false,
		.include_name = true,
		.include_txpower = true,
		.min_interval = 0x20,	//0x20
		.max_interval = 0x40, 	//0x40
		.appearance = 0x0340,//0x0C40		//http://dev.ti.com/tirex/content/simplelink_cc2640r2_sdk_1_35_00_33/docs/blestack/ble_sw_dev_guide/doxygen/group___g_a_p___appearance___values.html#gafc2f463732a098c1b42d30a766e90a6e
		.manufacturer_len = 0, //TEST_MANUFACTURER_DATA_LEN,
		.p_manufacturer_data =  NULL, //&test_manufacturer[0],
		.service_data_len = 0,
		.p_service_data = NULL,
		.service_uuid_len = sizeof(heart_rate_service_uuid),
		.p_service_uuid = heart_rate_service_uuid,
		.flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};
// scan response data
static esp_ble_adv_data_t scan_rsp_data = {
		.set_scan_rsp = true,
		.include_name = true,
		.include_txpower = true,
		.min_interval = 0x05,
		.max_interval = 0x10,
		.appearance = 0x0340,
		.manufacturer_len = 0, //TEST_MANUFACTURER_DATA_LEN,
		.p_manufacturer_data =  NULL, //&test_manufacturer[0],
		.service_data_len = 0,
		.p_service_data = NULL,
		.service_uuid_len = sizeof(heart_rate_service_uuid),
		.p_service_uuid = heart_rate_service_uuid,
		.flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT), //TODO ver as flags
};


#endif /* CONFIG_SET_RAW_ADV_DATA */

static esp_ble_adv_params_t adv_params = {
		.adv_int_min        = 0x0180,
		.adv_int_max        = 0x0580,
		.adv_type           = ADV_TYPE_IND,
		.own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
		//.peer_addr          = {0x79, 0xdb, 0xa7, 0x91, 0x13, 0x18},
		//.peer_addr_type     = BLE_ADDR_TYPE_PUBLIC,
		.channel_map        = ADV_CHNL_ALL,
		.adv_filter_policy  = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};


uint8_t char1_test_str[] = {0xE,1,123,0,13551>>8,13551&0x0F};	//Heart Rate//TODO delete this line

struct gatts_profile_inst {
	esp_gatts_cb_t gatts_cb;
	uint16_t gatts_if;
	uint16_t app_id;
	uint16_t conn_id;
	uint16_t service_handle;
	uint16_t char_num;
	uint16_t char_num_total;
	esp_gatt_srvc_id_t service_id;
	esp_bt_uuid_t char_uuid;
	esp_gatt_perm_t perm;
	esp_gatt_char_prop_t property;
	uint16_t descr_handle;
	esp_bt_uuid_t descr_uuid;
	uint16_t char_handle[2];
	int sensor_id;
};

static struct gatts_profile_inst gl_profile_tab[] = {
		[PROFILE_HR1_APP_ID] = {
				.char_num =0,
				.char_num_total = GATTS_CHAR_NUM_HR1,
				.gatts_cb = gatts_profile_a_event_handler,
				.gatts_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
				.sensor_id = 0,
		},
		[PROFILE_PLX1_APP_ID] = {
				.char_num =0,
				.char_num_total = GATTS_CHAR_NUM_PLX1,
				.gatts_cb = gatts_profile_b_event_handler,
				.gatts_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
				.sensor_id = 0,
		},
		[PROFILE_HR2_APP_ID] = {
				.char_num =0,
				.char_num_total = GATTS_CHAR_NUM_HR2,
				.gatts_cb = gatts_profile_c_event_handler,
				.gatts_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
				.sensor_id = 1,
		},
		[PROFILE_PLX2_APP_ID] = {
				.char_num =0,
				.char_num_total = GATTS_CHAR_NUM_PLX1,
				.gatts_cb = gatts_profile_d_event_handler,
				.gatts_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
				.sensor_id = 1,
		},
		[PROFILE_RAW_APP_ID] = {
				.char_num =0,
				.char_num_total = GATTS_CHAR_NUM_RAW,
				.gatts_cb = gatts_profile_e_event_handler,
				.gatts_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
				.sensor_id = -1,
		},
		[PROFILE_BATT_APP_ID] = {
				.char_num =0,
				.char_num_total = GATTS_CHAR_NUM_BATT,
				.gatts_cb = gatts_profile_f_event_handler,
				.gatts_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
				.sensor_id = -1,
		}

};


static struct gatts_char_inst gl_char[] = {
		{		//0 Body Location Characteristic
				.char_uuid.len = ESP_UUID_LEN_16,  // TX
				.char_uuid.uuid.uuid16 =  GATTS_CHAR_UUID_BODY_LOCATION,
				.char_perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
				.char_property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE,
				.char_val = &char1_BL_val,
				.char_control=NULL,
				.char_handle = 0,
				.char_read_callback=char1_read_handler,
				.char_write_callback=char1_write_handler,
				.descr_uuid.len = 0,
				.descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG, // ESP_GATT_UUID_CHAR_DESCRIPTION,  0x2902
				.descr_perm=ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
				.descr_val = &gatts_demo_descr1_val,
				.descr_control=NULL,
				.descr_handle=0,
				.descr_read_callback=descr1_read_handler,
				.descr_write_callback=descr1_write_handler,
				.is_notify = false
		},
		{		//1 Hearth rate Measurement Characteristic
				.char_uuid.len = ESP_UUID_LEN_16, // RX
				.char_uuid.uuid.uuid16 =  GATTS_CHAR_UUID_HR,
				.char_perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
				.char_property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
				.char_val = &char2_HR1_val,
				.char_control = NULL,
				.char_handle = 0,
				.char_read_callback=char2_read_handler,
				.char_write_callback=char2_write_handler,
				.descr_uuid.len = ESP_UUID_LEN_16,
				.descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG,
				.descr_perm=ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
				.descr_val = &gatts_demo_descr1_val,
				.descr_control=NULL,
				.descr_handle=0,
				.descr_read_callback=descr2_read_handler,
				.descr_write_callback=descr2_write_handler,
				.is_notify = false
		},
		{		//2 PLX Spot-check measurement
				.char_uuid.len = ESP_UUID_LEN_16,  // TX
				.char_uuid.uuid.uuid16 =   GATTS_CHAR_UUID_PLX,
				.char_perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
				.char_property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
				.char_val = &char3_PLX1_val,
				.char_control=NULL,
				.char_handle =0,
				.char_read_callback=char3_read_handler,
				.char_write_callback=char3_write_handler,
				.descr_uuid.len = ESP_UUID_LEN_16,
				.descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG, // ESP_GATT_UUID_CHAR_DESCRIPTION,
				.descr_perm=ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
				.descr_val = &gatts_demo_descr1_val,
				.descr_control=NULL,
				.descr_handle=0,
				.descr_read_callback=descr3_read_handler,
				.descr_write_callback=descr3_write_handler,
				.is_notify = false
		},
		{		//3 Body Location Characteristic
				.char_uuid.len = ESP_UUID_LEN_16,  // TX
				.char_uuid.uuid.uuid16 =  GATTS_CHAR_UUID_BODY_LOCATION,
				.char_perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
				.char_property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE ,
				.char_val = &char4_BL2_val,
				.char_control=NULL,
				.char_handle =0,
				.char_read_callback=char4_read_handler,
				.char_write_callback=char4_write_handler,
				.descr_uuid.len = 0,
				.descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG, // ESP_GATT_UUID_CHAR_DESCRIPTION,
				.descr_perm=ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
				.descr_val = &gatts_demo_descr1_val,
				.descr_control=NULL,
				.descr_handle=0,
				.descr_read_callback=descr4_read_handler,
				.descr_write_callback=descr4_write_handler,
				.is_notify = false
		},
		{		//4 Hearth rate Measurement Characteristic
				.char_uuid.len = ESP_UUID_LEN_16, // RX
				.char_uuid.uuid.uuid16 =  GATTS_CHAR_UUID_HR,
				.char_perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
				.char_property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
				.char_val = &char5_HR2_val,
				.char_control = NULL,
				.char_handle = 0,
				.char_read_callback=char5_read_handler,
				.char_write_callback=char5_write_handler,
				.descr_uuid.len = ESP_UUID_LEN_16,
				.descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG,
				.descr_perm=ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
				.descr_val = &gatts_demo_descr1_val,
				.descr_control=NULL,
				.descr_handle=0,
				.descr_read_callback=descr5_read_handler,
				.descr_write_callback=descr5_write_handler,
				.is_notify = false
		},
		{		//5 PLX Spot-check measurement
				.char_uuid.len = ESP_UUID_LEN_16,  // TX
				.char_uuid.uuid.uuid16 =   GATTS_CHAR_UUID_PLX,
				.char_perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
				.char_property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
				.char_val = &char6_PLX2_val,
				.char_control=NULL,
				.char_handle =0,
				.char_read_callback=char6_read_handler,
				.char_write_callback=char6_write_handler,
				.descr_uuid.len = ESP_UUID_LEN_16,
				.descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG, // ESP_GATT_UUID_CHAR_DESCRIPTION,
				.descr_perm=ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
				.descr_val = &gatts_demo_descr1_val,
				.descr_control=NULL,
				.descr_handle=0,
				.descr_read_callback=descr6_read_handler,
				.descr_write_callback=descr6_write_handler,
				.is_notify = false
		},
		{		//6 HR RAW data
				.char_uuid.len = ESP_UUID_LEN_16,  // TX
				.char_uuid.uuid.uuid16 =   GATTS_CHAR_UUID_RAW,
				.char_perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
				.char_property = ESP_GATT_CHAR_PROP_BIT_NOTIFY,//
				.char_val = &char7_RAW1_val,
				.char_control=NULL,
				.char_handle =0,
				.char_read_callback=char6_read_handler,
				.char_write_callback=char6_write_handler,
				.descr_uuid.len = ESP_UUID_LEN_16,
				.descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG, // ESP_GATT_UUID_CHAR_DESCRIPTION,
				.descr_perm=ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
				.descr_val = &gatts_demo_descr1_val,
				.descr_control=NULL,
				.descr_handle=0,
				.descr_read_callback=descr6_read_handler,
				.descr_write_callback=descr6_write_handler,
				.is_notify = false
		},
		{		//7 HR RAW data
				.char_uuid.len = ESP_UUID_LEN_16,  // TX
				.char_uuid.uuid.uuid16 =   GATTS_CHAR_UUID_RAW,
				.char_perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
				.char_property = ESP_GATT_CHAR_PROP_BIT_NOTIFY,//
				.char_val = &char8_RAW2_val,
				.char_control=NULL,
				.char_handle =0,
				.char_read_callback=char6_read_handler,
				.char_write_callback=char6_write_handler,
				.descr_uuid.len = ESP_UUID_LEN_16,
				.descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG, // ESP_GATT_UUID_CHAR_DESCRIPTION,
				.descr_perm=ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
				.descr_val = &gatts_demo_descr1_val,
				.descr_control=NULL,
				.descr_handle=0,
				.descr_read_callback=descr6_read_handler,
				.descr_write_callback=descr6_write_handler,
				.is_notify = false
		},
		{		//8 Battery level
				.char_uuid.len = ESP_UUID_LEN_16,  // TX
				.char_uuid.uuid.uuid16 =   GATTS_CHAR_UUID_BAT_LVL,
				.char_perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
				.char_property = ESP_GATT_CHAR_PROP_BIT_NOTIFY,//
				.char_val = &char9_BAT_lvl_val,
				.char_control=NULL,
				.char_handle =0,
				.char_read_callback=char6_read_handler,
				.char_write_callback=char6_write_handler,
				.descr_uuid.len = ESP_UUID_LEN_16,
				.descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG, // ESP_GATT_UUID_CHAR_DESCRIPTION,
				.descr_perm=ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
				.descr_val = &gatts_demo_descr1_val,
				.descr_control=NULL,
				.descr_handle=0,
				.descr_read_callback=descr6_read_handler,
				.descr_write_callback=descr6_write_handler,
				.is_notify = false
		},
		{		//9 Battery State
				.char_uuid.len = ESP_UUID_LEN_16,  // TX
				.char_uuid.uuid.uuid16 =   GATTS_CHAR_UUID_BAT_POWER_STATE,
				.char_perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
				.char_property = ESP_GATT_CHAR_PROP_BIT_NOTIFY,//
				.char_val = &char10_BAT_state_val,
				.char_control=NULL,
				.char_handle =0,
				.char_read_callback=char6_read_handler,
				.char_write_callback=char6_write_handler,
				.descr_uuid.len = ESP_UUID_LEN_16,
				.descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG, // ESP_GATT_UUID_CHAR_DESCRIPTION,
				.descr_perm=ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
				.descr_val = &gatts_demo_descr1_val,
				.descr_control=NULL,
				.descr_handle=0,
				.descr_read_callback=descr6_read_handler,
				.descr_write_callback=descr6_write_handler,
				.is_notify = false
		}
};



#endif /* MAIN_BLE_DEFINES_H_ */
