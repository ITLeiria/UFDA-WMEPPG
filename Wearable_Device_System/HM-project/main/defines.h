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
 *  defines.h
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

#ifndef MAIN_DEFINES_H_
#define MAIN_DEFINES_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

//#include "general_properties.h"
//#include "ble_properties.h"


#include "freertos/FreeRTOS.h"
#include "esp_freertos_hooks.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "driver/adc.h"
#include "driver/i2c.h"
#include "esp_adc_cal.h"
#include "esp_intr_alloc.h"
#include "esp_system.h"
#include "esp_pm.h"

#if defined(CONFIG_BT_ENABLED) && defined(CONFIG_BLUEDROID_ENABLED)
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gap_bt_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#endif

#include "esp_log.h"
#include "nvs_flash.h"


#define PACKS_TO_SEND 6	//max17 //packs of data to send in ble (FIFO_A_FULL*PACKS_TO SEND)
#define GATT_MAX_APPS 16//todo ver se faz algo: nao faz nada

#define REG_INTR_STATUS_1 0x00
#define REG_INTR_STATUS_2 0x01
#define REG_INTR_ENABLE_1 0x02
#define REG_INTR_ENABLE_2 0x03
#define REG_FIFO_WR_PTR 0x04
#define REG_OVF_COUNTER 0x05
#define REG_FIFO_RD_PTR 0x06
#define REG_FIFO_DATA 0x07
#define REG_FIFO_CONFIG 0x08
#define REG_MODE_CONFIG 0x09
#define REG_SPO2_CONFIG 0x0A
#define REG_LED1_PA 0x0C
#define REG_LED2_PA 0x0D
#define REG_PILOT_PA 0x10
#define REG_MULTI_LED_CTRL1 0x11
#define REG_MULTI_LED_CTRL2 0x12
#define REG_TEMP_INTR 0x1F
#define REG_TEMP_FRAC 0x20
#define REG_TEMP_CONFIG 0x21
#define REG_PROX_INT_THRESH 0x30
#define REG_REV_ID 0xFE
#define REG_PART_ID 0xFF

#define FIFO_A_FULL_EN		0x80
#define PRG_RDY_EN			0x40
#define ALC_OVF_EN			0x20
#define PROX_INT_EN			0x10



#define FIFO_A_FULL_EN		0x80
#define PRG_RDY_EN		0x40
#define ALC_OVF_EN		0x20
#define PROX_INT_EN		0x10

#define BLINK_GPIO 5
#define PRINT    1

#define I2C_SCL_IO_0           	26               //gpio number for i2c slave clock
#define I2C_SDA_IO_0           	25               //gpio number for i2c slave data
#define I2C_NUM_0			    I2C_NUM_0        //I2C port number for slave dev

#define I2C_SCL_IO_1         	19               // gpio number for I2C master clock
#define I2C_SDA_IO_1          	18               // gpio number for I2C master data
#define I2C_NUM_1            	I2C_NUM_1        // I2C port number for master dev

#define I2C_TX_BUF_DISABLE  	0                // I2C master do not need buffer
#define I2C_RX_BUF_DISABLE  	0                // I2C master do not need buffer
#define I2C_FREQ_HZ         	400000           // I2C master clock frequency

#define WRITE_BIT               I2C_MASTER_WRITE // I2C master write
#define READ_BIT                I2C_MASTER_READ  // I2C master read
#define ACK_CHECK_EN            0x1              // I2C master will check ack from slave
#define ACK_CHECK_DIS           0x0              // I2C master will not check ack from slave
#define ACK_VAL                 0x0              // I2C ack value
#define NACK_VAL                0x1              // I2C nack value

#define MAXREFDES117_ADDR       0x57             // address for MAXREFDES117 sensor

#define SPO2_RES 				16				 //SPO2 ADC resolution 18,17,16,15 bits
#define SPO2_SAMPLE_RATE		100				 //Options: 50,100,200,400,800,1000,1600,3600 default:100

#define SMP_AVE 				4				 //Options: 1,2,4,8,16,32	default:4
#define FIFO_A_FULL 			30				 //Options: 17 - 32			default:17
#define FIFO_ROLLOVER_EN 		1				 //Override data in fifo after it is full

#define LED1_CURRENT 			7				 //Red led current 0-50mA 0.2mA resolution
#define LED2_CURRENT 			LED1_CURRENT	 //IR  led current 0-50mA 0.2mA resolution

#define INT_PIN_0     			34				 //RTC GPIO used for interruptions
#define INT_PIN_1     			35				 //RTC GPIO used for interruptions
#define INT_PIN_2     			37				 //RTC GPIO used for interruption when usb is connected
#define INT_PIN_3     			0				 //RTC GPIO used for button (sleep or wake up esp32)
#define GPIO_INPUT_PIN_SEL  ((1ULL<<INT_PIN_0) | (1ULL<<INT_PIN_1) | (1ULL<<INT_PIN_2) | (1ULL<<INT_PIN_3))
#define GPIO_WAKEUP_PIN_SEL	((1ULL<<INT_PIN_0) | (1ULL<<INT_PIN_1) | (1ULL<<INT_PIN_3))
#define ESP_INTR_FLAG_DEFAULT 0




static void gatts_profile_a_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
static void gatts_profile_b_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
static void gatts_profile_c_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
static void gatts_profile_d_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
static void gatts_profile_e_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
static void gatts_profile_f_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

void char1_read_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
void char1_write_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
void char2_read_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
void char2_write_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
void char3_read_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
void char3_write_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
void char4_read_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
void char4_write_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
void char5_read_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
void char5_write_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
void char6_read_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
void char6_write_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

void descr1_read_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
void descr1_write_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
void descr2_read_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
void descr2_write_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
void descr3_read_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
void descr3_write_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
void descr4_read_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
void descr4_write_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
void descr5_read_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
void descr5_write_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
void descr6_read_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
void descr6_write_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

uint8_t get_n_notify();


//****Body Location defines****//
#define OTHER		0
#define CHEST		1
#define WRIST		2
#define FINGER		3
#define HAND		4
#define EAR_LOBE	5
#define FOOT		6

//****Heart rate Measurment Service FLAG defines****//
#define HEART_RATE_8BIT 						0x00	//
#define HEART_RATE_16BIT 						0x01	//Heart rate value with 16bit
#define SENSOR_CONTACT_NOT_SUPPORTED 			0x02	//
#define SENSOR_CONTACT_NOT_DETECTED 			0x04	//
#define SENSOR_CONTACT_DETECTED		 			0x06	//
#define ENERGY_EXPENDED_NOT_PRESENT 			0x00	//
#define ENERGY_EXPENDED_PRESENT 				0x08	//in Kilojoule
#define RR_INTERVAL_NOT_PRESENT 				0x00	//
#define RR_INTERVAL_PRESENT 					0x016	//16bit for RR-interval 1/1024 Resolution


//*****Pulse oximeter FLAG defines *****//
#define SPO2PS_FAST_PRESENT						0x01	//
#define SPO2PS_SLOW_PRESENT						0x02	//
#define MEASURMENT_STATUS_PRESENT 				0x04	//
#define DEVICE_AND_SENSOR_STATUS_PRESENT		0x08	//
#define PULSE_AMPLITUDE_INDEX_PRESENT			0x10	//

//*****Battery state defines *****//
#define BATT_STATE_UNKNOWN 						0x00
#define BATT_STATE_NOT_SUPPORTED				0x01
#define BATT_STATE_NOT_PRESENT 					0x02
#define BATT_STATE_PRESENT						0x03

#define BATT_STATE_NOT_DISCHARGING				0x02<<2
#define BATT_STATE_DISCHARGING					0x03<<2

#define BATT_STATE_NOT_CHARGEABLE				0x01<<4
#define BATT_STATE_NOT_CHARGING					0x02<<4
#define BATT_STATE_CHARGING						0x03<<4

#define BATT_STATE_GOOD_LEVEL					0x02<<6
#define BATT_STATE_CRITICALY_LOW_LEVEL			0x03<<6

//defined 1 profile for each service
#define PROFILE_TOTAL_NUM 6		//TOTAL Profile number
#define PROFILE_HR1_APP_ID 	0	// 	HR1
#define PROFILE_PLX1_APP_ID 1	//	PLX1
#define PROFILE_HR2_APP_ID 	2	//	HR2
#define PROFILE_PLX2_APP_ID 3	//	PLX2
#define PROFILE_RAW_APP_ID 	4	//	RAW data 1
#define PROFILE_BATT_APP_ID 5	//	Battery level

#define GATTS_CHAR_NUM_HR1		2	//HR 	CHAR
#define GATTS_CHAR_NUM_PLX1		1	//PULSE CHAR
#define GATTS_CHAR_NUM_HR2		2	//HR 	CHAR
#define GATTS_CHAR_NUM_PLX2		1	//PULSE CHAR
#define GATTS_CHAR_NUM_RAW		2	//HR 	Raw data
#define GATTS_CHAR_NUM_BATT		2	//Battery level
#define GATTS_CHAR_NUM_TOTAL			(GATTS_CHAR_NUM_HR1 + GATTS_CHAR_NUM_PLX1)+ (GATTS_CHAR_NUM_HR2 + GATTS_CHAR_NUM_PLX2) + GATTS_CHAR_NUM_RAW + GATTS_CHAR_NUM_BATT	//TOTAL CHAR x2 sensors


#define GATTS_SERVICE_UUID_HEART_RATE   0x180D 			//Heart Rate Service uuid
#define GATTS_CHAR_UUID_HR      	0x2A37					//Heart Rate Measurement characteristic uuid
#define GATTS_CHAR_UUID_BODY_LOCATION      0x2A38					//Body sensor location   characteristic uuid
#define GATTS_DESCR_UUID_A     	0x2901					//
#define GATTS_NUM_HANDLE_HR		1+GATTS_CHAR_NUM_HR1*3

#define GATTS_SERVICE_UUID_PULSE_OXIMETER   0x1822 		//Pulse Oximeter Service uuid
#define GATTS_CHAR_UUID_PLX       	0x2A5F					//PLX Spot-Check Measurement characteristic uuid
#define GATTS_NUM_HANDLE_PLX     	1+GATTS_CHAR_NUM_PLX1*3

#define GATTS_SERVICE_UUID_RAW_DATA_SERVICE   0x18FF 	//Raw data Service uuid
#define GATTS_CHAR_UUID_RAW       0x2AFF					//Raw data characteristic uuid
#define GATTS_NUM_HANDLE_RAW     	1+GATTS_CHAR_NUM_RAW*3

#define GATTS_SERVICE_UUID_BATTERY_SERVICE   	0x180F 	//Battery level Service uuid
#define GATTS_CHAR_UUID_BAT_LVL    				0x2A19					//Battery level characteristic uuid
#define GATTS_CHAR_UUID_BAT_POWER_STATE       	0x2A1A					//Battery Power State characteristic uuid
#define GATTS_NUM_HANDLE_BAT     	1+GATTS_CHAR_NUM_BATT*3

#define TEST_MANUFACTURER_DATA_LEN  17

#define PREPARE_BUF_MAX_SIZE 1024
//end ble defines
//******ADC DEFINES******//
#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   64          //Multisampling
static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_0;     //pin GPIO 36
static const gpio_num_t adc_pin = GPIO_NUM_37;
static const gpio_num_t adc_en_pin = GPIO_NUM_27;
//GPIO34 if ADC1, GPIO14 if ADC2
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;
static bool is_charging = false;
//end ADC defines
void i2c_task_0(void* arg);
void i2c_task_1(void* arg);
void blink_task(void* arg);
void notify_task(void* arg);
void notify_task_optimized(void* arg);
void sensor_task_manager(void* arg);
void batt_state_task(void* arg);
void batt_level_task(void* arg);
void standby_task(void* arg);
void check_ret(esp_err_t ret, uint8_t sensor_data_h);
esp_err_t max30102_read_reg (uint8_t uch_addr,i2c_port_t i2c_num, uint8_t* data);
esp_err_t max30102_write_reg(uint8_t uch_addr,i2c_port_t i2c_num, uint8_t puch_data);
esp_err_t max30102_read_fifo(i2c_port_t i2c_num, uint16_t sensorDataRED[],uint16_t sensorDataIR[]);
static esp_err_t max30102_init(i2c_port_t i2c_num);
static esp_err_t max30102_reset(i2c_port_t i2c_num);
static esp_err_t max30102_shutdown(i2c_port_t i2c_num);
void check_fifo(esp_err_t ret,uint8_t sensor_data_h, uint8_t sensor_data_m, uint8_t sensor_data_l);
uint8_t get_SPO2_CONF_REG();
uint8_t get_FIFO_CONF_REG();
uint8_t get_LED1_PA();
uint8_t get_LED2_PA();
void intr_init();
double process_data(uint16_t RAWsensorDataRED[],uint16_t RAWsensorDataIR[],double *mean1, double *mean2);
void data_rms(uint16_t RAWsensorDataRED[],uint16_t RAWsensorDataIR[],double *rms_l1, double *rms_l2);
void data_mean(uint16_t RAWsensorDataRED[],uint16_t RAWsensorDataIR[],double *mean1, double *mean2);
void print_array(uint8_t *array,uint16_t size);
void bt_main();

void max30102_blink(int n_times, int period, i2c_port_t port0 );
void idle_task_print(void* arg);
//void vApplicationIdleHook(void* arg);
void idle_task_0(void* arg);
void idle_task_1(void* arg);

static double core0_idle_time = 0,core0_idle_time_last = 0;
static double core1_idle_time = 0,core1_idle_time_last = 0;

#endif /* MAIN_DEFINES_H_ */
