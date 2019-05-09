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
 *  HM.c
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


#define EN_MAX30102_READING_TASK	//enable i2c sensor readings
#define EN_SENSOR0		//enable i2c sensor0	SDA=25	SCL=26	INT=34
#define EN_SENSOR1		//enable i2c sensor1	SDA=18 	SCL=19	INT=35

#define EN_BLE_TASK					//enable bluetooth
#define EN_BLE_BOND_TASK			//enable bond in bluetooth
#define EN_NOTIFY					//enable notify task
#define EN_BATTERY_MEASURMENT_TASK	//enable the battery measurment value
//#define EN_CHECK_BAT_PRESENCE		//enable battery presence check, (getting wrong values)
//#define EN_SLEEP_BUTTON				//enable GPIO0 sleep button (makes esp sleep or wake up)
#define PLOT 						//disables other printf
//#define PRINT_ALL_SENSOR_DATA 	//prints all fifo data

//#include "algorithm_by_RF.h"
//#include "algorithm_IK_C.h"
//#include "algorithm_by_RF.cpp"

#include "defines.h"
#include "algorithm_IK_C.h"
#define TRESHOLD_ON 15000

static esp_err_t sensor_detected[2] = {ESP_FAIL,ESP_FAIL}; //initialise as not detected
static uint16_t RAW0_str[FIFO_A_FULL/2],RAW1_str[FIFO_A_FULL/2];				//RAW1,RAW2
static bool sensor_have_finger[2]; //flag for finger presence on sensor
static bool battery_lvl_changed=true;
static bool battery_status_changed=true;
static int buffer_pos_s0=-1,buffer_pos_s1=-1;
static bool standby_state = false;

TaskHandle_t notify_TaskHandle = NULL;
TaskHandle_t battery_TaskHandle = NULL;

/*********************BLE DEFINES**************************************************************************************/
#include "ble_defines.h"

////end ble part


static xQueueHandle gpio_evt_queue = NULL;

static float getBattPercentage(int32_t vbati){
	float battLvl =0.0;// ((vbat-MIN_BATT_V)/(MAX_BATT_V - MIN_BATT_V))*100;
	float vbat = (float)vbati/1000;
	int plist[] = {100,95,84,74,60,51,37,18,9,3,1};
	float vbat_aux=4.2;
	if(vbat>4.3){
		battLvl = 100.0;
	}else if(vbat>4.2){
		battLvl = 101.0;
	}else if(vbat<3.1){
		battLvl = 0;
	}else{
		for (int i=0;vbat_aux>3.1;vbat_aux-=0.1,i++){
//			printf("vbat_aux = %f\t vbat = %f\n",vbat_aux,vbat);
//			printf("plist[i] = %d\t plist[i+1] = %d\n",plist[i],plist[i+1]);

			if(vbat<vbat_aux && vbat>=vbat_aux-0.1){
				battLvl = plist[i+1]+(vbat-(vbat_aux-.1))*(plist[i]-plist[i+1])*10;
				break;
			}
		}
	}

	/*if(vbat>4.2){
		battLvl = 100.0;
	}
	else if(vbat<4.2 && vbat >=4.1){

	}else if(vbat<4.1 && vbat >=4.0){

	}else if(vbat<4.0 && vbat >=3.9){

	}else if(vbat<3.9 && vbat >=3.8){

	}else if(vbat<3.8 && vbat >=3.7){

	}else if(vbat<3.7 && vbat >=3.6){

	}else if(vbat<3.6 && vbat >=3.5){

	}else if(vbat<3.5 && vbat >=3.4){

	}else{

	}*/

	return  battLvl;
}
static float getBattVoltage(){
	uint32_t adc_reading = 0;
	//Multisampling
	for (int i = 0; i < NO_OF_SAMPLES; i++) {
		adc_reading += adc1_get_raw((adc1_channel_t)channel); //GPIO36
		vTaskDelay(1 / portTICK_RATE_MS);
	}
	adc_reading /= NO_OF_SAMPLES;
	//Convert adc_reading to voltage in mV
	uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);

	return (float) voltage/.72; //divide by resistence proporcion 1/(100k/(100k+39k))
}
static bool getBattState(){
	//read GPIO37 - 1 if charging, 0 if not
	return gpio_get_level(GPIO_NUM_37);
}
static void i2c_init(i2c_port_t i2c_num){
	i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = I2C_FREQ_HZ;
	if (i2c_num==I2C_NUM_1){
		conf.sda_io_num = I2C_SDA_IO_1;
		conf.scl_io_num = I2C_SCL_IO_1;
	} else {
		conf.sda_io_num = I2C_SDA_IO_0;
		conf.scl_io_num = I2C_SCL_IO_0;
	}
	i2c_param_config(i2c_num, &conf);
	i2c_driver_install(i2c_num, conf.mode,
			I2C_RX_BUF_DISABLE,
			I2C_TX_BUF_DISABLE, 0);
}
static esp_err_t max30102_shutdown(i2c_port_t i2c_num){
	esp_err_t ret;
	uint_fast8_t data_h=0x00;
	uint_fast8_t i=0;
	do{
		max30102_read_reg(REG_MODE_CONFIG,i2c_num, &data_h);
		ret = max30102_write_reg(REG_MODE_CONFIG,i2c_num,0x80 | data_h);	//shutdown and keep the same mode
		i++;
	}while(ret != ESP_OK && i<10); //try to connect 50 times
	return ret;
}
static esp_err_t max30102_reset(i2c_port_t i2c_num){
	uint_fast8_t data_h=0x00;
	esp_err_t ret;
	uint_fast8_t i=0;
	do{
		max30102_read_reg(REG_INTR_STATUS_1,i2c_num,&data_h);//clear interrupt pin
		max30102_read_reg(REG_MODE_CONFIG, i2c_num, &data_h);
		ret = max30102_write_reg(REG_MODE_CONFIG,i2c_num,0x40 | data_h);	//reset and keep the same mode
		i++;
	}while(ret != ESP_OK && i<10); //try to connect 100 times
	return ret;
}
static esp_err_t max30102_init(i2c_port_t i2c_num){
	printf("***MAX30102 initialization***\n");
	esp_err_t ret;
	uint_fast8_t i=0;
	do{
		ret = max30102_write_reg(REG_INTR_ENABLE_1,i2c_num, PROX_INT_EN );	//0b1001 0000
		max30102_write_reg(REG_INTR_ENABLE_2,i2c_num,0x00);		//Temperature interrupt
		max30102_write_reg(REG_FIFO_WR_PTR,i2c_num,0x00);		//
		max30102_write_reg(REG_OVF_COUNTER,i2c_num,0x00);		//
		max30102_write_reg(REG_FIFO_RD_PTR,i2c_num,0x00);		//
		max30102_write_reg(REG_FIFO_CONFIG,i2c_num,get_FIFO_CONF_REG());		//4 average sample + FIFO_A_FULL 15
		max30102_write_reg(REG_MODE_CONFIG,i2c_num,0x03);		//0x07 -> Multi-LED  //0x03 -> SpO2 mode //0x02 -> HR mode
		max30102_write_reg(REG_SPO2_CONFIG,i2c_num,get_SPO2_CONF_REG());		//01 adc range + 100 samples/sec + 18  bit resolution
		max30102_write_reg(REG_LED1_PA,i2c_num,get_LED1_PA());			//RED LED current
		max30102_write_reg(REG_LED2_PA,i2c_num,get_LED2_PA());			//IR  LED current
		max30102_write_reg(REG_PILOT_PA,i2c_num,get_LED1_PA());		//Multimode registers (not used)
		max30102_write_reg(REG_PROX_INT_THRESH,i2c_num,0x30);
		i++;
	}while(ret != ESP_OK && i<10); //try to connect 10 times
	return ret;
}

static void adc_init(){
	/*Initialize the ADC used in GPIO37
	 * and GPIO27 used to enable/disable the ADC measurment (en/dis voltage divider)*/
	//Configure ADC

	if (unit == ADC_UNIT_1) {
		adc1_config_width(ADC_WIDTH_BIT_12);
		adc1_config_channel_atten(channel, atten);
	}

	//Characterize ADC
	adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
	esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
	//print_char_val_type(val_type);
	gpio_set_direction(adc_pin,GPIO_MODE_INPUT); //gpio used to read charging state

	gpio_pad_select_gpio(adc_en_pin);
	gpio_set_direction(adc_en_pin, GPIO_MODE_OUTPUT);
	gpio_set_level(adc_en_pin, 0);
}

static void IRAM_ATTR gpio_isr_handler(void* arg){
	uint32_t gpio_num = (uint32_t) arg;
	//printf("INTERRUPTION on pin %d",(int)arg);
	switch ((uint32_t)arg) {
	case INT_PIN_0:
	case INT_PIN_1:
		xTaskCreate(sensor_task_manager, "isr_task_manager", 1024 * 4, (void* ) arg, 10, NULL);
		break;
	case INT_PIN_2:
		xTaskCreate(batt_state_task,"batt_task",	1024*4, (void*) arg, 10 , NULL);
		break;
	case INT_PIN_3:
		gpio_set_intr_type(INT_PIN_3, GPIO_INTR_DISABLE);
		gpio_set_level(5,!gpio_get_level(5));
		xTaskCreate(standby_task,"standby_task",	1024*4, (void*) arg, 10, NULL);
		break;
	default:
		break;
	}
}

void app_main()
{
	int min_cpu_freq = 26, max_cpu_freq = 160;//160MHz is optimal tradeoff freq/power

	esp_pm_config_esp32_t pm_config = {
	            .max_cpu_freq = max_cpu_freq,
				.max_freq_mhz = max_cpu_freq,
	            .min_cpu_freq = 80,
				.min_freq_mhz = min_cpu_freq,
				.light_sleep_enable = false
	    };

	    ESP_ERROR_CHECK( esp_pm_configure(&pm_config) );
	//define idle task hooks
	esp_register_freertos_idle_hook_for_cpu(idle_task_0,0);
	esp_register_freertos_idle_hook_for_cpu(idle_task_1,1);
	xTaskCreate(idle_task_print, "idle_task_print", 1024 * 2, (void* ) 0, 10, NULL);

	raw_ptr0_IR = malloc(FIFO_A_FULL);
	raw_ptr0_RED =malloc(FIFO_A_FULL);
	raw_ptr1_IR = malloc(FIFO_A_FULL);
	raw_ptr1_RED =malloc(FIFO_A_FULL);
	char7_RAW1_val.attr_value = raw_ptr0_IR;
	char8_RAW2_val.attr_value = raw_ptr1_IR;

	/*float SPO2_value=99.87654321;
	int32_t spo2_32;
	spo2_32 = memcpy(&spo2_32,&SPO2_value,sizeof(float));
	short spo_aux;
	spo_aux    =  ((spo2_32 & 0x7fffffff) >> 13) - (0x38000000 >> 13);
	spo_aux    |= ((spo2_32 & 0x80000000) >> 16);
	printf("spo_aux = %d\t= 0x%x\n",spo_aux,spo_aux);

	spo_aux    =  ((spo2_32 & 0x7fffffff) >> 13) - (0x38000000 >> 13);
	spo_aux    |= ((spo2_32 & 0x80000000) >> 16);
	//printf("spo_aux = 0x%x\n",spo_aux);
	return;*/

#ifdef EN_BATTERY_MEASURMENT_TASK
	adc_init();
	//initialize battery state and battery level task
#endif

#ifdef EN_MAX30102_READING_TASK
	printf("Start app_main\n");
	intr_init();
#ifdef EN_SENSOR0
	i2c_port_t port0 = I2C_NUM_0;
	i2c_init(port0);
	max30102_init(port0);
	max30102_blink(5, 200, port0);
	sensor_detected[0] = max30102_shutdown(port0);
	ESP_LOGW("\tSensor 0","%s",sensor_detected[0]==ESP_OK ? "Detected" : "NOT Detected");
#endif
#ifdef EN_SENSOR1
	i2c_port_t port1 = I2C_NUM_1;
	i2c_init(port1);
	max30102_init(port1);
	max30102_blink(5, 200, port1);
	sensor_detected[1] = max30102_shutdown(port1);
	ESP_LOGW("\tSensor 1","%s",sensor_detected[1]==ESP_OK ? "Detected" : "NOT Detected");
#endif
#endif

#ifdef EN_BLE_TASK
	bt_main();
#ifndef CONFIG_BT_ENABLED
	esp_light_sleep_start();
#endif //CONFIG_BT_ENABLED
#endif //EN_BLE_TASK
	//xTaskCreate(i2c_task_0, "i2c_test_task_0", 1024 * 2, (void* ) 0, 10, NULL);
	//xTaskCreate(i2c_task_1, "i2c_test_task_1", 1024 * 2, (void* ) 0, 10, NULL);
	//xTaskCreate(blink_task, "blink_task"	 , 1024 * 2, (void* ) 0, 10, NULL);

}

void i2c_task_0(void* arg)
{
	printf("Start task 0!\n");
	int i = 0;
	int ret;
	/*uint32_t task_idx = (uint32_t) arg;
	//uint8_t* data = (uint8_t*) malloc(DATA_LENGTH);
	//uint8_t* data_wr = (uint8_t*) malloc(DATA_LENGTH);
	//uint8_t* data_rd = (uint8_t*) malloc(DATA_LENGTH);*/

	uint8_t sensor_data_h;
	while(1){
		ret = max30102_read_reg(REG_LED1_PA, I2C_NUM_1, &sensor_data_h);
		check_ret(ret,sensor_data_h);
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
	vTaskDelete(NULL);

}

void i2c_task_1(void* arg)
{	double mean1, mean2;
#ifndef PLOT
printf("Start i2c_task_1!\n");
#endif
int *buffer_pos;
uint8_t *raw_pptr_IR,*raw_pptr_RED;
raw_pptr_IR =  malloc(FIFO_A_FULL);
raw_pptr_RED = malloc(FIFO_A_FULL);
i2c_port_t port = (i2c_port_t)arg;	//I2C_NUM_0 or I2C_NUM_1
uint16_t RAWsensorDataRED[FIFO_A_FULL/2], RAWsensorDataIR[FIFO_A_FULL/2];
max30102_read_fifo(port, RAWsensorDataRED,RAWsensorDataIR);
if (port == I2C_NUM_0){ //if sensor0
	buffer_pos = &buffer_pos_s0;
	raw_pptr_IR  = raw_ptr0_IR;
	raw_pptr_RED = raw_ptr0_RED;
	if(*buffer_pos >= 0){
		raw_ptr0_IR = realloc(raw_ptr0_IR,sizeof(RAWsensorDataIR)*(*buffer_pos+1));
		raw_ptr0_RED = realloc(raw_ptr0_RED,sizeof(RAWsensorDataRED)*(*buffer_pos+1));
		ESP_LOGI("reallocated"," %d bytes\tsensor: %d\n",sizeof(RAWsensorDataIR)*(*buffer_pos+1),port);
		memcpy(raw_ptr0_IR+(sizeof(RAWsensorDataIR)*(*buffer_pos)),RAWsensorDataIR,sizeof(RAWsensorDataIR));
		memcpy(raw_ptr0_RED+(sizeof(RAWsensorDataRED)*(*buffer_pos)),RAWsensorDataRED,sizeof(RAWsensorDataRED));
		//print_array(raw_ptr0,(sizeof(RAWsensorDataIR)/2)*(*buffer_pos+1));
	}
}else{
	buffer_pos = &buffer_pos_s1;
	raw_pptr_IR  = raw_ptr1_IR;
	raw_pptr_RED = raw_ptr1_RED;
	if(*buffer_pos >= 0){
		raw_ptr1_IR = realloc(raw_ptr1_IR,sizeof(RAWsensorDataIR)*(*buffer_pos+1));
		raw_ptr1_RED = realloc(raw_ptr1_RED,sizeof(RAWsensorDataRED)*(*buffer_pos+1));
		ESP_LOGI("reallocated","%d bytes\tsensor: %d\n",sizeof(RAWsensorDataIR)*(*buffer_pos+1),port);
		memcpy(raw_ptr1_IR+(sizeof(RAWsensorDataIR)*(*buffer_pos)),RAWsensorDataIR,sizeof(RAWsensorDataIR));
		memcpy(raw_ptr1_RED+(sizeof(RAWsensorDataRED)*(*buffer_pos)),RAWsensorDataRED,sizeof(RAWsensorDataRED));
		//print_array(raw_ptr1,(sizeof(RAWsensorDataIR)/2)*(*buffer_pos+1));
	}
}

/*if(*buffer_pos >= 0){
	raw_pptr_IR = realloc(raw_ptr0_IR,sizeof(RAWsensorDataIR)*(*buffer_pos+1));
	raw_pptr_RED = realloc(raw_ptr0_RED,sizeof(RAWsensorDataRED)*(*buffer_pos+1));
	ESP_LOGI("reallocated"," %d bytes\tsensor: %d\n",sizeof(RAWsensorDataIR)*(*buffer_pos+1),port);
	memcpy(raw_pptr_IR+(sizeof(RAWsensorDataIR)*(*buffer_pos)),RAWsensorDataIR,sizeof(RAWsensorDataIR));
	memcpy(raw_pptr_RED+(sizeof(RAWsensorDataRED)*(*buffer_pos)),RAWsensorDataRED,sizeof(RAWsensorDataRED));
}*/
if (notify_TaskHandle != NULL){
	*buffer_pos +=1;
}else{
	*buffer_pos = -1;
}
if(*buffer_pos > 0){//ignore 1st read

	//double SPO2 = process_data(RAWsensorDataRED,RAWsensorDataIR,&mean1,&mean2);
	data_mean(RAWsensorDataRED,RAWsensorDataIR,&mean1,&mean2);
	printf("Mean:\t%f,\t%f\n",mean1,mean2);
	//fprintf(stdout,"\tSPO2: %02f%%\n\n",SPO2);
	if(mean1<TRESHOLD_ON && mean2<TRESHOLD_ON){	//IF NO FINGER
		if (port == I2C_NUM_0){
			sensor_have_finger[0] = false;
		}else {
			sensor_have_finger[1] = false;
		}
		*buffer_pos = -1;
		max30102_write_reg(REG_INTR_ENABLE_1,port, PROX_INT_EN);	//Enable proximity interruption
	}
	if(notify_TaskHandle != NULL && *buffer_pos >= PACKS_TO_SEND){//Packet size
		float SPO2_value=0;

		int32_t HR_value=0,data_len= (sizeof(RAW0_str)*(int)(*buffer_pos)/2); //data lenght
		float ratio,correl;
		int8_t spo2_valid, hr_valid;

		uint32_t IRdata_to_process[data_len],REDdata_to_process[data_len];
		uint16_t *raw_ptr_aux0_IR=raw_ptr0_IR,*raw_ptr_aux0_RED = raw_ptr0_RED
				,*raw_ptr_aux1_IR=raw_ptr1_IR,*raw_ptr_aux1_RED = raw_ptr1_RED;
		for(int i=0;i<data_len;i++){ //convert data to 32bit pointer
			if (port == I2C_NUM_0){//todo optimizar com ponteiros estes ifs
				IRdata_to_process [i]= raw_ptr_aux0_IR[i];
				REDdata_to_process[i]= raw_ptr_aux0_RED[i];
			}else{
				IRdata_to_process [i]= raw_ptr_aux1_IR[i];
				REDdata_to_process[i]= raw_ptr_aux1_RED[i];
			}
		}

		//process data and get HR and spo2 value if data is valid
		heart_rate_and_oxygen_saturation(IRdata_to_process,data_len,REDdata_to_process,&SPO2_value,&spo2_valid,&HR_value,&hr_valid,&ratio,&correl);

		if(spo2_valid && hr_valid){
			ESP_LOGW("Processamento","HR: %d\tSPO2: %f\t Sensor: %d \n",HR_value,SPO2_value,port == I2C_NUM_0?0:1);
			uint32_t spo2_32;
			uint16_t spo2_16;
			uint8_t *spo2_8;
			memcpy(&spo2_32,&SPO2_value,sizeof(float));

			//passing from float32 to float16 and assigning in the array
			spo2_16    =  ((spo2_32 & 0x7fffffff) >> 13) - (0x38000000 >> 13);
			spo2_16    |= ((spo2_32 & 0x80000000) >> 16);
			spo2_8 = &spo2_16;

			//PLX2_str[1]=SPO2_value;
			if (port == I2C_NUM_0){
				HR1_str[1]=HR_value;
				PLX1_str[1]=spo2_8[0];
				PLX1_str[2]=spo2_8[1];
			}else{
				HR2_str[1]=HR_value;
				PLX2_str[1]=spo2_8[0];
				PLX2_str[2]=spo2_8[1];
			}
		}else{
			ESP_LOGE("Processamento","HR: %d\tSPO2: %f\t Sensor: %d \n",HR_value,SPO2_value,port == I2C_NUM_0?0:1);
		}
		char7_RAW1_val.attr_value=raw_ptr0_IR;
		char8_RAW2_val.attr_value=raw_ptr1_IR;
		char7_RAW1_val.attr_len = sizeof(RAW0_str)*(int)(*buffer_pos);	//attribute lenght in bytes
		char8_RAW2_val.attr_len = sizeof(RAW1_str)*(int)(*buffer_pos);
		//esp_ble_gatt_set_local_mtu(sizeof(RAW1_str) *buffer_pos + 3);
		vTaskResume(notify_TaskHandle);
		*buffer_pos = 0;
	}
}else{
	printf("******Ignoring first read!*****\n\n");
}

#ifndef CONFIG_BT_ENABLED
esp_light_sleep_start();
#endif
vTaskDelete(NULL);
}

void blink_task(void* arg)
{
	gpio_pad_select_gpio(BLINK_GPIO);
	gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
	printf("Start blinking\n");
	int led = 1,i=0;
	int delay = 1000/((int)arg);
	while(1){
		//printf("Blink\t%d\n",i++);
		gpio_set_level(BLINK_GPIO, led=!led);
		vTaskDelay(pdMS_TO_TICKS(delay));
	}
	vTaskDelete(NULL);
}

void sensor_task_manager(void* arg)
{
	ESP_LOGI("Sensor_TASK_MANAGER","on core %d\tpin: %d",xPortGetCoreID(),(int)arg);
	uint8_t data=0x00;
	i2c_port_t port = (i2c_port_t)arg == INT_PIN_0 ? I2C_NUM_0: I2C_NUM_1;

	max30102_read_reg(REG_INTR_STATUS_1,port,&data); //clear interruption in the sensor

	bool fifo_a_full_int = data>>7 & 0x01;
	bool prox_int = data>>4 & 0x01;
	bool alc_ovf = data>>5 & 0x01;
#ifndef PLOT
	printf("\tINT Reason: 0x%02x\t",data);
	fifo_a_full_int ? printf("\tFIFO A FULL\n") : NULL;
	prox_int 		? printf("\tPROX INT\n") : NULL;
#endif
	if (prox_int){
		if ((i2c_port_t)arg == INT_PIN_0){
			sensor_have_finger[0] = true ;
		}else{
			sensor_have_finger[1] = true ;
		}
		//clear buffer
		max30102_write_reg(REG_INTR_ENABLE_1,port, FIFO_A_FULL_EN);	//0b1000 0000 //disable prox interrupt and enable fifo_a_full
	}

	//xTaskCreate(i2c_task_0, "i2c_test_task_0", 1024 * 4, (void* ) 0, 10, NULL);
	xTaskCreate(i2c_task_1, "i2c_task_1", 1024 * 4, (void* ) port, 10, NULL);	//4kB stack size
	vTaskDelete(NULL);
}

void batt_state_task(void* arg)
{	/* Runs when interruption on pin 37 is generated
 	 * Reads GPIO37 and get state - Charging or not Charging*/
	gpio_set_intr_type(INT_PIN_2, GPIO_INTR_DISABLE);	//disable interrupt to prevent (de)bouncing
	printf("Batt_task int_pin: %d on core %d\n",(int)arg,xPortGetCoreID());
	battery_status_changed=true;
	is_charging = gpio_get_level(GPIO_NUM_37);
	ESP_LOGI("Bat_state","%s",is_charging?"Charging":"NOT Charging");
	uint_fast8_t CHARGING_FLAG = is_charging ? BATT_STATE_CHARGING:BATT_STATE_NOT_CHARGING;
	BAT_state_str[0] = BATT_STATE_PRESENT|BATT_STATE_DISCHARGING|CHARGING_FLAG|BATT_STATE_GOOD_LEVEL;
	//BAT_state_str[0] = 0x03|(0x03<<2)|(0x02<<4)|(0x00<<6) | (is_charging << 5);

	if(is_charging){
		gpio_set_intr_type(INT_PIN_2, GPIO_INTR_NEGEDGE);	//interrupt on falling edge
	}else{
		gpio_set_intr_type(INT_PIN_2, GPIO_INTR_POSEDGE);	//interrupt on rising edge
	}

	if(battery_status_changed){
		uint_fast8_t profile = PROFILE_BATT_APP_ID, ch = 9; //profile and char id for bat_state char
		if(gl_char[ch].is_notify && notify_TaskHandle != NULL){ //if char is notify and notify task was created
			esp_ble_gatts_send_indicate(gl_profile_tab[profile].gatts_if, 0, gl_profile_tab[profile].char_handle[1],
					gl_char[ch].char_val->attr_len,gl_char[ch].char_val->attr_value , false);
			battery_status_changed=false;
#ifdef EN_BATTERY_MEASURMENT_TASK
			if(!battery_task_running){
				xTaskCreate(batt_level_task, "batt_level_task", 1024 * 2, (void*) 0, 10, &battery_TaskHandle);
			}else{
				//notify batt_lvl
			}
#endif
		}
	}
#ifdef EN_CHECK_BAT_PRESENCE
	float batt_v = getBattVoltage();
	if(batt_v < 500 || batt_v > 4500){ //Battery not detected
		gpio_set_intr_type(INT_PIN_2, GPIO_INTR_DISABLE);
		ESP_LOGE("Battery","Not detected %f",batt_v);
	}
#endif
	vTaskDelete(NULL);

}

void batt_level_task(void* arg)
{
	battery_task_running = true;
	uint_fast8_t profile = PROFILE_BATT_APP_ID, ch = 8; //profile and char id for bat_lvl char
	do{
		gpio_set_level(adc_en_pin, 0); //enable
		vTaskDelay(100 / portTICK_RATE_MS);
		int sum=0;
		float battV,battLvl;
		for (int i = 0; i < 1; i++) {
			battV   = getBattVoltage();			//battery voltage
			battLvl = getBattPercentage(battV); 	//battery percentage
			sum+= (uint_fast8_t) battLvl;
		}
		battLvl = sum/1;
		ESP_LOGW("Batt","BattV: %.2fmV\tLvl: %.2f%%\t",battV,battLvl);
		if(BAT_lvl_str[0] != (int)battLvl){
			battery_lvl_changed=true;
			BAT_lvl_str[0] = (int)battLvl;
		}

		if(battery_lvl_changed){
			if(gl_char[ch].is_notify && notify_TaskHandle != NULL){ //if char is notify and notify task was created
				esp_ble_gatts_send_indicate(gl_profile_tab[profile].gatts_if, 0, gl_profile_tab[profile].char_handle[0],
						gl_char[ch].char_val->attr_len,gl_char[ch].char_val->attr_value , false);
			}
			battery_lvl_changed=false;
		}
		gpio_set_level(adc_en_pin, 1); //disable
		if(gl_char[ch].is_notify){
			vTaskDelay(20*1000 / portTICK_RATE_MS);
		}else{
			battery_task_running = false;
			vTaskDelete(battery_TaskHandle);
			battery_TaskHandle = NULL;
		}

	}while(1);
	battery_task_running = false;
	vTaskDelete(battery_TaskHandle);
	battery_TaskHandle = NULL;
}

void standby_task(void* arg){
	gpio_set_intr_type(INT_PIN_3, GPIO_INTR_DISABLE);
	ESP_LOGI("StandBy","%s",!standby_state ? "Enable" : "Wake up");
	standby_state = !standby_state;
	if(standby_state){
		 esp_bluedroid_disable();
		 esp_bluedroid_deinit();
		 esp_bt_controller_disable();
		 esp_bt_controller_deinit();
		 vTaskDelay(pdMS_TO_TICKS(100));
		 gpio_set_intr_type(INT_PIN_3, GPIO_INTR_LOW_LEVEL);
		 gpio_wakeup_enable(0,GPIO_INTR_LOW_LEVEL);
		 esp_deep_sleep_start();
		//vTaskSuspendAll();
	}else{
//not working yet
		bt_main(); //enable bluedroid and bt controller
		gpio_set_intr_type(INT_PIN_3, GPIO_INTR_LOW_LEVEL);
		//app_main();

	}

	vTaskDelete(NULL);
}


esp_err_t max30102_write_reg(uint8_t uch_addr, i2c_port_t i2c_num, uint8_t puch_data){

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, MAXREFDES117_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, uch_addr, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, puch_data, ACK_CHECK_EN);
	i2c_master_stop(cmd);
	int ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	//printf("writing to 0x%02x\n",uch_addr);
	if (ret != ESP_OK) {
		printf("ESP NOT OK\n");
		return ret;
	}

	return ret;
}

esp_err_t max30102_read_reg(uint8_t uch_addr,i2c_port_t i2c_num, uint8_t* data_h)
{
	int ret;
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, MAXREFDES117_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, uch_addr, ACK_CHECK_EN);
	i2c_master_stop(cmd);	//send the stop bit
	ret = i2c_master_cmd_begin(i2c_num, cmd, 100 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	if (ret != ESP_OK) {
		printf("ESP NOT OK READING\n");
		return ret;
	}
	vTaskDelay(10 / portTICK_RATE_MS);
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, MAXREFDES117_ADDR << 1 | READ_BIT, ACK_CHECK_EN);
	i2c_master_read_byte(cmd, data_h, NACK_VAL);
	i2c_master_stop(cmd);
	ret = i2c_master_cmd_begin(i2c_num, cmd, 100 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	return ret;
}

esp_err_t max30102_read_fifo(i2c_port_t i2c_num, uint16_t sensorDataRED[],uint16_t sensorDataIR[])
{

	uint8_t LED_1[FIFO_A_FULL/2][3],LED_2[FIFO_A_FULL/2][3];

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, MAXREFDES117_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, REG_FIFO_DATA, ACK_CHECK_EN);
	i2c_master_stop(cmd);	//send the stop bit
	int ret = i2c_master_cmd_begin(i2c_num, cmd, 100 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	if (ret != ESP_OK) {
		printf("ESP NOT OK!!\n");
		return ret;
	}
	vTaskDelay(25 / portTICK_RATE_MS);

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, MAXREFDES117_ADDR << 1 | READ_BIT, ACK_CHECK_EN);

	for(int i=0; i < FIFO_A_FULL/2; i++){
		i2c_master_read_byte(cmd, &LED_1[i][0], ACK_VAL);
		i2c_master_read_byte(cmd, &LED_1[i][1], ACK_VAL);
		i2c_master_read_byte(cmd, &LED_1[i][2], ACK_VAL);

		i2c_master_read_byte(cmd, &LED_2[i][0], ACK_VAL);
		i2c_master_read_byte(cmd, &LED_2[i][1], ACK_VAL);
		i2c_master_read_byte(cmd, &LED_2[i][2], i==FIFO_A_FULL/2-1? NACK_VAL: ACK_VAL);	//NACK_VAL on the last iteration
	}

	i2c_master_stop(cmd);
	ret = i2c_master_cmd_begin(i2c_num, cmd, 100 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);

	for(int i=0; i < FIFO_A_FULL/2; i++){
		/*sensorDataRED[i] = (((LED_1[i][0] &  0b00000011) <<16) + (LED_1[i][1] <<8) + LED_1[i][2])>>(18-SPO2_RES)<<(18-SPO2_RES);
		sensorDataIR[i]= (((LED_2[i][0] &  0b00000011) <<16) + (LED_2[i][1] <<8) + LED_2[i][2])>>(18-SPO2_RES)<<(18-SPO2_RES);	//*/
		//printf("\t%x %x %x\n",LED_1[i][0]&0x03,LED_1[i][1],LED_1[i][2]);

		sensorDataRED[i] = 	((((LED_1[i][0] &  0b00000011) <<16) + (LED_1[i][1] <<8) + LED_1[i][2])>>(18-SPO2_RES)<<(18-SPO2_RES))>>2;
		sensorDataIR[i]= 	((((LED_2[i][0] &  0b00000011) <<16) + (LED_2[i][1] <<8) + LED_2[i][2])>>(18-SPO2_RES)<<(18-SPO2_RES))>>2;	//
		/*#if SPO2_RES <= 16
		sensorDataRED[i] =sensorDataRED[i] >>2;
		sensorDataIR[i]  =sensorDataIR[i]  >>2;
#endif*/
		//fprintf(stdout,"%x, %x, %x\t%x, %x, %x\n",LED_1[i][0],LED_1[i][1],LED_1[i][2],LED_2[i][0],LED_2[i][1],LED_2[i][2]);
#ifdef PRINT_ALL_SENSOR_DATA
		fprintf(stdout,"0x%x\t0x%x\n",sensorDataRED[i],sensorDataIR[i]);
#endif
	}


	/*int data1 = (((LED_1[i][0] && 0b00000011) <<16) + (LED_1[i][1] <<8) + LED_1[i][2])>>(18-SPO2_RES);
		/int data2 = (((LED_2[i][0] && 0b00000011) <<16) + (LED_2[i][1] <<8) + LED_2[i][2])>>(18-SPO2_RES);	*/

	//vTaskDelay(6000 / portTICK_RATE_MS);//magic delay

	return ret;
}

void check_ret(esp_err_t ret,uint8_t sensor_data_h){
	if(ret == ESP_ERR_TIMEOUT) {
		printf("I2C timeout\n");
	} else if(ret == ESP_OK) {
		printf("******************* \n");
		printf("TASK[%d]  MASTER READ SENSOR( EN_MAX30102_READING_TASK )\n", 0);
		printf("*******************\n");
		printf("data: %02x\n", sensor_data_h);
	} else {
		printf("%s: No ack, sensor not connected...skip...\n", esp_err_to_name(ret));
	}
}

void check_fifo(int ret,uint8_t sensor_data_h, uint8_t sensor_data_m, uint8_t sensor_data_l){
	if(ret == ESP_ERR_TIMEOUT) {
		printf("I2C timeout\n");
	} else if(ret == ESP_OK) {
		printf("******************* \n");
		printf("READING FIFO ( EN_MAX30102_READING_TASK )\n");
		printf("*******************\n");
		printf("data_h: %02x\n", sensor_data_h);
		printf("data_m: %02x\n", sensor_data_m);
		printf("data_l: %02x\n", sensor_data_l);

		printf("sensor val: %d\n", (sensor_data_h << 16 |sensor_data_m << 8 | sensor_data_l));
	} else {
		printf("%s: No ack, sensor not connected...skip...\n", esp_err_to_name(ret));
	}
}

uint8_t get_SPO2_CONF_REG(){
	uint8_t spo2_adc_rge, spo2_sr, spo2_res;
#ifndef SPO2_SAMPLE_RATE
#define SPO2_SAMPLE_RATE 100
#endif
	switch (SPO2_SAMPLE_RATE){
	case 50:
		spo2_sr =  0x00;
		break;
	case 100:
		spo2_sr =  0x01;
		break;
	case 200:
		spo2_sr =  0x02;
		break;
	case 400:
		spo2_sr =  0x03;
		break;
	case 800:
		spo2_sr =  0x04;
		break;
	case 1000:
		spo2_sr =  0x05;
		break;
	case 1600:
		spo2_sr =  0x06;
		break;
	case 3600:
		spo2_sr =  0x07;
		break;
	default:
		spo2_sr =  0x01;
	}
#ifndef SPO2_RES
#define SPO2_RES 18
#endif
	switch (SPO2_RES){
	case 15:
		spo2_res =  0x00;
		break;
	case 16:
		spo2_res =  0x01;
		break;
	case 17:
		spo2_res =  0x02;
		break;
	case 18:
		spo2_res =  0x03;
		break;
	default:
		spo2_res =  0x00;
	}
#ifndef SPO2_ADC_RGE
#define SPO2_ADC_RGE 0x01
#endif
	switch (SPO2_ADC_RGE){
	case 0x00:
		spo2_adc_rge =  0x00;
		break;
	case 0x01:
		spo2_adc_rge =  0x01;
		break;
	case 0x02:
		spo2_adc_rge =  0x02;
		break;
	case 0x03:
		spo2_adc_rge =  0x03;
		break;
	default:
		spo2_adc_rge =  0x00;
	}

	//printf("REGISTER is: 0x%x\n",spo2_adc_rge<<5 | spo2_sr<<2 | spo2_res);
	return spo2_adc_rge<<5 | spo2_sr<<2 | spo2_res;
}

uint8_t get_FIFO_CONF_REG(){
	uint8_t smp_ave,fifo_rollover_en,fifo_a_full;
#ifndef SMP_AVE
#define SMP_AVE 8
#endif
	switch (SMP_AVE){
	case 1:
		smp_ave =  0x00;
		break;
	case 2:
		smp_ave =  0x01;
		break;
	case 4:
		smp_ave =  0x02;
		break;
	case 8:
		smp_ave =  0x03;
		break;
	case 16:
		smp_ave =  0x04;
		break;
	case 32:
		smp_ave =  0x05;
		break;
	default:
		smp_ave =  0x03;
	}

#ifndef FIFO_ROLLOVER_EN
#define FIFO_ROLLOVER_EN 0
#endif
	fifo_rollover_en = FIFO_ROLLOVER_EN;

#ifndef FIFO_A_FULL
#define FIFO_A_FULL 30
#endif
	fifo_a_full = FIFO_A_FULL >32 ? 2: 32-FIFO_A_FULL;

	//printf("REGISTER is: 0x%x\n",smp_ave<<5 | fifo_rollover_en<<4 | fifo_a_full);
	return smp_ave<<5 | fifo_rollover_en<<4 | fifo_a_full;
}

uint8_t get_LED1_PA(){	//RED LED CURRENT

#if  !defined(LED1_CURRENT) && !defined(LED1_PA)
#define LED1_CURRENT 7	//0-50 mA	0.2mA resolution
#define LED1_PA 0x24	//0-255 brightness
	return LED1_CURRENT*5;
#elif	defined(LED1_CURRENT)
	return LED1_CURRENT*5;
#elif	defined(LED1_PA)
	return LED1_PA;
#else
	return 35;
#endif
}

uint8_t get_LED2_PA(){ //IR LED CURRENT
#if  !defined(LED2_CURRENT) && !defined(LED2_PA)
#define LED2_CURRENT 7	//0-50 mA	0.2mA resolution
#define LED2_PA 0x24	//0-255 brightness
	return LED2_CURRENT*5;
#elif	defined(LED2_CURRENT)
	return LED2_CURRENT*5;
#elif	defined(LED2_PA)
	return LED2_PA;
#else
	return 35;
#endif
}

void intr_init(){
	gpio_config_t io_conf;
	io_conf.intr_type = GPIO_PIN_INTR_NEGEDGE;		//interrupt ofn falling edge
	io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;		//bit mask of the pins
	io_conf.mode = GPIO_MODE_INPUT;					//set as input mode
	io_conf.pull_down_en = 0;						//disable pull-down mode
	io_conf.pull_up_en = 1;							//enable pull-up mode
	gpio_config(&io_conf);
	//gpio_set_intr_type(INT_PIN_0, GPIO_INTR_NEGEDGE);	//interrupt on falling edge
	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);		//install gpio isr service
	rtc_gpio_pullup_en(INT_PIN_0);
	rtc_gpio_pullup_en(INT_PIN_1);
	gpio_isr_handler_add(INT_PIN_0, gpio_isr_handler, (void*) INT_PIN_0);	//hook isr handler for specific gpio pin
	gpio_isr_handler_add(INT_PIN_1, gpio_isr_handler, (void*) INT_PIN_1);	//hook isr handler for specific gpio pin
#ifdef EN_BATTERY_MEASURMENT_TASK
	gpio_set_intr_type(INT_PIN_2, GPIO_INTR_ANYEDGE);	//interrupt on falling edge
	gpio_isr_handler_add(INT_PIN_2, gpio_isr_handler, (void*) INT_PIN_2);	//hook isr handler for specific gpio pin
	rtc_gpio_pullup_dis(INT_PIN_2);
	rtc_gpio_pulldown_en(INT_PIN_2);
#endif
#ifdef EN_SLEEP_BUTTON
	gpio_set_intr_type(INT_PIN_3, GPIO_INTR_NEGEDGE);	//interrupt on falling edge
	gpio_isr_handler_add(INT_PIN_3, gpio_isr_handler, (void*) INT_PIN_3);	//hook isr handler for specific gpio pin
	rtc_gpio_pullup_dis(INT_PIN_3);
	rtc_gpio_pulldown_en(INT_PIN_3);
	//esp_sleep_enable_ext0_wakeup(INT_PIN_3, GPIO_PIN_INTR_LOLEVEL);
	//gpio_wakeup_enable(0,GPIO_PIN_INTR_LOLEVEL);
	//gpio_pin_wakeup_enable(GPIO_NUM_0,GPIO_PIN_INTR_LOLEVEL);
#endif
	esp_sleep_enable_ext1_wakeup(GPIO_WAKEUP_PIN_SEL,ESP_EXT1_WAKEUP_ALL_LOW); //enable external wake up
}

double process_data(uint16_t RAWsensorDataRED[],uint16_t RAWsensorDataIR[],double *mean1, double *mean2){
	double rms1, rms2, dc1, dc2, R,SPO2;
		data_rms(RAWsensorDataRED,RAWsensorDataIR,&rms1,&rms2);
		data_mean(RAWsensorDataRED,RAWsensorDataIR,&dc1,&dc2);
		R = (rms1/dc1)/(rms2/dc2);
		SPO2 = 110-(25*R);
	#ifndef PLOT
		//printf("\tVALOR RMS: %f e %f\n",rms1,rms2);
		//printf("\tVALOR DC: %f e %f\n",dc1,dc2);
		//printf("\tVALOR R: %f \n",R);
		printf("\tVALOR SPO2: %f \n",SPO2);
	#endif
		*mean1 = dc1;
		*mean2 = dc2;
	return SPO2;
}

void data_rms(uint16_t RAWsensorDataRED[],uint16_t RAWsensorDataIR[],double *rms_1, double *rms_2){
	double dataLength = sizeof(RAWsensorDataRED)/sizeof(int);
	double sum_led1=0 , sum_led2 =0;
	for (int i = 0; i < dataLength; ++i) {
		sum_led1 +=(RAWsensorDataRED[i])^2;
		sum_led2 +=(RAWsensorDataIR[i])^2;
	}

	*rms_1= sqrt((1/dataLength)*sum_led1);
	*rms_2= sqrt((1/dataLength)*sum_led2);
	return;
}

void data_mean(uint16_t RAWsensorDataRED[],uint16_t RAWsensorDataIR[],double *mean1, double *mean2){
	double dataLength = FIFO_A_FULL/2;
	double sum1=0 ,sum2=0 ;
	for (int i = 0; i < dataLength; ++i) {
		sum1 += RAWsensorDataRED[i];
		sum2 += RAWsensorDataIR[i];
	}
	*mean1 = sum1/dataLength;
	*mean2 = sum2/dataLength;
}

/*********************BLE PART**************************************************************************************/


void notify_task_optimized( void* arg) {
	notify_task_running = true;
	do{
		n_notify=0;
		uint8_t ch=0;
		for (int profile=0;profile<PROFILE_TOTAL_NUM;profile++){	//for each profile
			for (int j=0;j<gl_profile_tab[profile].char_num_total;j++,ch++){ //for each char in this profile
				if(gl_char[ch].is_notify){	//is char notifying
					n_notify++;
					switch	(gl_profile_tab[profile].sensor_id){
					case -1:
						if(gl_char[ch].char_uuid.uuid.uuid16 == GATTS_CHAR_UUID_RAW){ //if raw characteristic
							if(!sensor_have_finger[j]){ //if not detecting finger, do not notify
								printf("No finger on sensor %d\n",j);
								//continue; //do not notify
								//notify if 1st time
							}else{
								esp_ble_gatts_send_indicate(gl_profile_tab[profile].gatts_if, 0, gl_profile_tab[profile].char_handle[j],
										gl_char[ch].char_val->attr_len,gl_char[ch].char_val->attr_value , false);
								//printf("Notified this: \n");
								//print_array(gl_char[ch].char_val->attr_value ,gl_char[ch].char_val->attr_len/2);
								//realloc(j==0 ? raw_ptr0:raw_ptr1,0);
								//free(j==0 ? raw_ptr0:raw_ptr1_IR);
							}
						}else if(gl_char[ch].char_uuid.uuid.uuid16 == GATTS_CHAR_UUID_BAT_LVL){ //if battery characteristic
							//verify if battery level changed and notify
							if(battery_lvl_changed){
								esp_ble_gatts_send_indicate(gl_profile_tab[profile].gatts_if, 0, gl_profile_tab[profile].char_handle[0],
										gl_char[ch].char_val->attr_len,gl_char[ch].char_val->attr_value , false);
								battery_lvl_changed=false;
							}
						}else if(gl_char[ch].char_uuid.uuid.uuid16 == GATTS_CHAR_UUID_BAT_POWER_STATE){
							//verify if battery status changed and notify
							//battery power state is notified in the interruption (for faster feedback)
							/*if(battery_status_changed){
								esp_ble_gatts_send_indicate(gl_profile_tab[profile].gatts_if, 0, gl_profile_tab[profile].char_handle[1],
										gl_char[ch].char_val->attr_len,gl_char[ch].char_val->attr_value , false);
								battery_status_changed=false;
							}*/
						}
						break;
					default:
						if(sensor_have_finger[gl_profile_tab[profile].sensor_id]){
							if(gl_char[ch].char_uuid.uuid.uuid16 == GATTS_CHAR_UUID_HR){ //if heart rate
								if((gl_char[ch].char_val->attr_value[0] & SENSOR_CONTACT_DETECTED) == SENSOR_CONTACT_NOT_DETECTED){//SENSOR NOT DETECTED
									gl_char[ch].char_val->attr_value[0] |=  0b00000110;//todo corrigir isto
									esp_ble_gatts_send_indicate(gl_profile_tab[profile].gatts_if, 0, gl_profile_tab[profile].char_handle[j],
											gl_char[ch].char_val->attr_len,gl_char[ch].char_val->attr_value , false);
								}
							}
							esp_ble_gatts_send_indicate(gl_profile_tab[profile].gatts_if, 0, gl_profile_tab[profile].char_handle[j],
									gl_char[ch].char_val->attr_len,gl_char[ch].char_val->attr_value , false);
						}else{//if no finger
							if(gl_char[ch].char_uuid.uuid.uuid16 == GATTS_CHAR_UUID_HR){
								if((gl_char[ch].char_val->attr_value[0] & SENSOR_CONTACT_DETECTED) == SENSOR_CONTACT_DETECTED){//SENSOR DETECTED
									gl_char[ch].char_val->attr_value[0] &= 0b11111101; //set as sensor not detected
									esp_ble_gatts_send_indicate(gl_profile_tab[profile].gatts_if, 0, gl_profile_tab[profile].char_handle[j],
											gl_char[ch].char_val->attr_len,gl_char[ch].char_val->attr_value , false);
								}
							}
						}
						break;
					}
				}
			}
		}

		printf("Have %d chars to notify\n",n_notify);
		//vTaskDelay(2500 / portTICK_RATE_MS); // delay 1s
		vTaskSuspend(notify_TaskHandle);
	}while(n_notify>0);//there is any char to notify
	notify_task_running = false;
	vTaskDelete(NULL);
}

static uint8_t notify_pos=0;

void char1_read_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	uint8_t char_num = 0;
	ESP_LOGI(GATTS_TAG, "char%d_read_handler %d\n",char_num ,param->read.handle);
	esp_gatt_rsp_t rsp;
	memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
	rsp.attr_value.handle = param->read.handle;
	if (gl_char[char_num].char_val!=NULL) {
		ESP_LOGI(GATTS_TAG, "char%d_read_handler char_val %d\n",char_num,gl_char[char_num].char_val->attr_len);
		rsp.attr_value.len = gl_char[char_num].char_val->attr_len;
		for (uint32_t pos=0;pos<gl_char[char_num].char_val->attr_len&&pos<gl_char[char_num].char_val->attr_max_len;pos++) {
			rsp.attr_value.value[pos] = gl_char[char_num].char_val->attr_value[pos];
		}
	}
	ESP_LOGI(GATTS_TAG, "char%d_read_handler esp_gatt_rsp_t\n",char_num);
	esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id, ESP_GATT_OK, &rsp);
}
void char2_read_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	uint8_t char_num = 1;
	ESP_LOGI(GATTS_TAG, "char%d_read_handler %d\n",char_num ,param->read.handle);
	esp_gatt_rsp_t rsp;
	memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
	rsp.attr_value.handle = param->read.handle;
	if (gl_char[char_num].char_val!=NULL) {
		ESP_LOGI(GATTS_TAG, "char%d_read_handler char_val %d\n",char_num,gl_char[char_num].char_val->attr_len);
		rsp.attr_value.len = gl_char[char_num].char_val->attr_len;
		for (uint32_t pos=0;pos<gl_char[char_num].char_val->attr_len&&pos<gl_char[char_num].char_val->attr_max_len;pos++) {
			rsp.attr_value.value[pos] = gl_char[char_num].char_val->attr_value[pos];
		}
	}
	ESP_LOGI(GATTS_TAG, "char%d_read_handler esp_gatt_rsp_t\n",char_num);
	esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id, ESP_GATT_OK, &rsp);
}
void char3_read_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	uint8_t char_num = 2;
	ESP_LOGI(GATTS_TAG, "char%d_read_handler %d\n",char_num ,param->read.handle);
	esp_gatt_rsp_t rsp;
	memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
	rsp.attr_value.handle = param->read.handle;
	if (gl_char[char_num].char_val!=NULL) {
		ESP_LOGI(GATTS_TAG, "char%d_read_handler char_val %d\n",char_num,gl_char[char_num].char_val->attr_len);
		rsp.attr_value.len = gl_char[char_num].char_val->attr_len;
		for (uint32_t pos=0;pos<gl_char[char_num].char_val->attr_len&&pos<gl_char[char_num].char_val->attr_max_len;pos++) {
			rsp.attr_value.value[pos] = gl_char[char_num].char_val->attr_value[pos];
		}
	}
	ESP_LOGI(GATTS_TAG, "char%d_read_handler esp_gatt_rsp_t\n",char_num);
	esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id, ESP_GATT_OK, &rsp);
}
void char4_read_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	uint8_t char_num = 3;
	ESP_LOGI(GATTS_TAG, "char%d_read_handler %d\n",char_num ,param->read.handle);
	esp_gatt_rsp_t rsp;
	memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
	rsp.attr_value.handle = param->read.handle;
	if (gl_char[char_num].char_val!=NULL) {
		ESP_LOGI(GATTS_TAG, "char%d_read_handler char_val %d\n",char_num,gl_char[char_num].char_val->attr_len);
		rsp.attr_value.len = gl_char[char_num].char_val->attr_len;
		for (uint32_t pos=0;pos<gl_char[char_num].char_val->attr_len&&pos<gl_char[char_num].char_val->attr_max_len;pos++) {
			rsp.attr_value.value[pos] = gl_char[char_num].char_val->attr_value[pos];
		}
	}
	ESP_LOGI(GATTS_TAG, "char%d_read_handler esp_gatt_rsp_t\n",char_num);
	esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id, ESP_GATT_OK, &rsp);
}
void char5_read_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	uint8_t char_num = 4;
	ESP_LOGI(GATTS_TAG, "char%d_read_handler %d\n",char_num ,param->read.handle);
	esp_gatt_rsp_t rsp;
	memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
	rsp.attr_value.handle = param->read.handle;
	if (gl_char[char_num].char_val!=NULL) {
		ESP_LOGI(GATTS_TAG, "char%d_read_handler char_val %d\n",char_num,gl_char[char_num].char_val->attr_len);
		rsp.attr_value.len = gl_char[char_num].char_val->attr_len;
		for (uint32_t pos=0;pos<gl_char[char_num].char_val->attr_len&&pos<gl_char[char_num].char_val->attr_max_len;pos++) {
			rsp.attr_value.value[pos] = gl_char[char_num].char_val->attr_value[pos];
		}
	}
	ESP_LOGI(GATTS_TAG, "char%d_read_handler esp_gatt_rsp_t\n",char_num);
	esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id, ESP_GATT_OK, &rsp);
}
void char6_read_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	uint8_t char_num = 5;
	ESP_LOGI(GATTS_TAG, "char%d_read_handler %d\n",char_num ,param->read.handle);
	esp_gatt_rsp_t rsp;
	memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
	rsp.attr_value.handle = param->read.handle;
	if (gl_char[char_num].char_val!=NULL) {
		ESP_LOGI(GATTS_TAG, "char%d_read_handler char_val %d\n",char_num,gl_char[char_num].char_val->attr_len);
		rsp.attr_value.len = gl_char[char_num].char_val->attr_len;
		for (uint32_t pos=0;pos<gl_char[char_num].char_val->attr_len&&pos<gl_char[char_num].char_val->attr_max_len;pos++) {
			rsp.attr_value.value[pos] = gl_char[char_num].char_val->attr_value[pos];
		}
	}
	ESP_LOGI(GATTS_TAG, "char%d_read_handler esp_gatt_rsp_t\n",char_num);
	esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id, ESP_GATT_OK, &rsp);
}

void descr1_read_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	uint8_t char_num = 0;
	ESP_LOGI(GATTS_TAG, "descr%d_read_handler %d\n",char_num, param->read.handle);
	esp_gatt_rsp_t rsp;
	memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
	rsp.attr_value.handle = param->read.handle;
	if (gl_char[char_num].descr_val!=NULL) {
		ESP_LOGI(GATTS_TAG, "descr%d_read_handler descr_val %d\n",char_num,gl_char[char_num].descr_val->attr_len);
		rsp.attr_value.len = gl_char[char_num].descr_val->attr_len;
		for (uint32_t pos=0;pos<gl_char[char_num].descr_val->attr_len&&pos<gl_char[char_num].descr_val->attr_max_len;pos++) {
			rsp.attr_value.value[pos] = gl_char[char_num].descr_val->attr_value[pos];
		}
	}
	ESP_LOGI(GATTS_TAG, "descr%d_read_handler esp_gatt_rsp_t \n",char_num);
	esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
			ESP_GATT_OK, &rsp);
}
void descr2_read_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	uint8_t char_num = 1;
	ESP_LOGI(GATTS_TAG, "descr%d_read_handler %d\n",char_num, param->read.handle);
	esp_gatt_rsp_t rsp;
	memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
	rsp.attr_value.handle = param->read.handle;
	if (gl_char[char_num].descr_val!=NULL) {
		ESP_LOGI(GATTS_TAG, "descr%d_read_handler descr_val %d\n",char_num,gl_char[char_num].descr_val->attr_len);
		rsp.attr_value.len = gl_char[char_num].descr_val->attr_len;
		for (uint32_t pos=0;pos<gl_char[char_num].descr_val->attr_len&&pos<gl_char[char_num].descr_val->attr_max_len;pos++) {
			rsp.attr_value.value[pos] = gl_char[char_num].descr_val->attr_value[pos];
		}
	}
	ESP_LOGI(GATTS_TAG, "descr%d_read_handler esp_gatt_rsp_t \n",char_num);
	esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
			ESP_GATT_OK, &rsp);
}
void descr3_read_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	uint8_t char_num = 2;
	ESP_LOGI(GATTS_TAG, "descr%d_read_handler %d\n",char_num, param->read.handle);
	esp_gatt_rsp_t rsp;
	memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
	rsp.attr_value.handle = param->read.handle;
	if (gl_char[char_num].descr_val!=NULL) {
		ESP_LOGI(GATTS_TAG, "descr%d_read_handler descr_val %d\n",char_num,gl_char[char_num].descr_val->attr_len);
		rsp.attr_value.len = gl_char[char_num].descr_val->attr_len;
		for (uint32_t pos=0;pos<gl_char[char_num].descr_val->attr_len&&pos<gl_char[char_num].descr_val->attr_max_len;pos++) {
			rsp.attr_value.value[pos] = gl_char[char_num].descr_val->attr_value[pos];
		}
	}
	ESP_LOGI(GATTS_TAG, "descr%d_read_handler esp_gatt_rsp_t \n",char_num);
	esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
			ESP_GATT_OK, &rsp);
}
void descr4_read_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	uint8_t char_num = 3;
	ESP_LOGI(GATTS_TAG, "descr%d_read_handler %d\n",char_num, param->read.handle);
	esp_gatt_rsp_t rsp;
	memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
	rsp.attr_value.handle = param->read.handle;
	if (gl_char[char_num].descr_val!=NULL) {
		ESP_LOGI(GATTS_TAG, "descr%d_read_handler descr_val %d\n",char_num,gl_char[char_num].descr_val->attr_len);
		rsp.attr_value.len = gl_char[char_num].descr_val->attr_len;
		for (uint32_t pos=0;pos<gl_char[char_num].descr_val->attr_len&&pos<gl_char[char_num].descr_val->attr_max_len;pos++) {
			rsp.attr_value.value[pos] = gl_char[char_num].descr_val->attr_value[pos];
		}
	}
	ESP_LOGI(GATTS_TAG, "descr%d_read_handler esp_gatt_rsp_t \n",char_num);
	esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
			ESP_GATT_OK, &rsp);
}
void descr5_read_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	uint8_t char_num = 4;
	ESP_LOGI(GATTS_TAG, "descr%d_read_handler %d\n",char_num, param->read.handle);
	esp_gatt_rsp_t rsp;
	memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
	rsp.attr_value.handle = param->read.handle;
	if (gl_char[char_num].descr_val!=NULL) {
		ESP_LOGI(GATTS_TAG, "descr%d_read_handler descr_val %d\n",char_num,gl_char[char_num].descr_val->attr_len);
		rsp.attr_value.len = gl_char[char_num].descr_val->attr_len;
		for (uint32_t pos=0;pos<gl_char[char_num].descr_val->attr_len&&pos<gl_char[char_num].descr_val->attr_max_len;pos++) {
			rsp.attr_value.value[pos] = gl_char[char_num].descr_val->attr_value[pos];
		}
	}
	ESP_LOGI(GATTS_TAG, "descr%d_read_handler esp_gatt_rsp_t \n",char_num);
	esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
			ESP_GATT_OK, &rsp);
}
void descr6_read_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	uint8_t char_num = 5;
	ESP_LOGI(GATTS_TAG, "descr%d_read_handler %d\n",char_num, param->read.handle);
	esp_gatt_rsp_t rsp;
	memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
	rsp.attr_value.handle = param->read.handle;
	if (gl_char[char_num].descr_val!=NULL) {
		ESP_LOGI(GATTS_TAG, "descr%d_read_handler descr_val %d\n",char_num,gl_char[char_num].descr_val->attr_len);
		rsp.attr_value.len = gl_char[char_num].descr_val->attr_len;
		for (uint32_t pos=0;pos<gl_char[char_num].descr_val->attr_len&&pos<gl_char[char_num].descr_val->attr_max_len;pos++) {
			rsp.attr_value.value[pos] = gl_char[char_num].descr_val->attr_value[pos];
		}
	}
	ESP_LOGI(GATTS_TAG, "descr%d_read_handler esp_gatt_rsp_t \n",char_num);
	esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
			ESP_GATT_OK, &rsp);
}

void char1_notify_handle(esp_gatt_if_t gatts_if, uint16_t conn_id) {
	uint8_t char_num = 0;
	ESP_LOGI(GATTS_TAG, "char%d_notify_handler \n",char_num);
	if (gl_char[char_num].is_notify==1) {
		notify_pos='0';
		for (uint32_t i=0;i<5;i++) {
			ESP_LOGI(GATTS_TAG, "char%d_notify_handle esp_ble_gatts_send_indicate\n",char_num);
			vTaskDelay(100 / portTICK_RATE_MS); // delay 1s
			esp_ble_gatts_send_indicate(gatts_if, conn_id, gl_char[char_num].char_handle,1,&notify_pos,false);
			notify_pos++;
		}
	}
}
void char2_notify_handle(esp_gatt_if_t gatts_if, uint16_t conn_id) {
	uint8_t char_num = 1;
	ESP_LOGI(GATTS_TAG, "char%d_notify_handler \n",char_num);
	if (gl_char[char_num].is_notify==1) {
		notify_pos='0';
		/*for (uint32_t i=0;i<10;i++) {
			ESP_LOGI(GATTS_TAG, "char%d_notify_handle esp_ble_gatts_send_indicate\n",char_num);
			vTaskDelay(1000 / portTICK_RATE_MS); // delay 1s
			esp_ble_gatts_send_indicate(gatts_if, conn_id, gl_char[char_num].char_handle,1,&notify_pos,false);
			notify_pos++;
		}*/
	}
}
void char3_notify_handle(esp_gatt_if_t gatts_if, uint16_t conn_id) {
	uint8_t char_num = 2;
	ESP_LOGI(GATTS_TAG, "char%d_notify_handler \n",char_num);
	if (gl_char[char_num].is_notify==1) {
		notify_pos='0';
		for (uint32_t i=0;i<10;i++) {
			ESP_LOGI(GATTS_TAG, "char%d_notify_handle esp_ble_gatts_send_indicate\n",char_num);
			vTaskDelay(1000 / portTICK_RATE_MS); // delay 1s
			esp_ble_gatts_send_indicate(gatts_if, conn_id, gl_char[char_num].char_handle,1,&notify_pos,false);
			notify_pos++;
		}
	}
}
void char4_notify_handle(esp_gatt_if_t gatts_if, uint16_t conn_id) {
	uint8_t char_num = 3;
	ESP_LOGI(GATTS_TAG, "char%d_notify_handler \n",char_num);
	if (gl_char[char_num].is_notify==1) {
		notify_pos='0';
		for (uint32_t i=0;i<10;i++) {
			ESP_LOGI(GATTS_TAG, "char%d_notify_handle esp_ble_gatts_send_indicate\n",char_num);
			vTaskDelay(1000 / portTICK_RATE_MS); // delay 1s
			esp_ble_gatts_send_indicate(gatts_if, conn_id, gl_char[char_num].char_handle,1,&notify_pos,false);
			notify_pos++;
		}
	}
}
void char5_notify_handle(esp_gatt_if_t gatts_if, uint16_t conn_id) {
	uint8_t char_num = 4;
	ESP_LOGI(GATTS_TAG, "char%d_notify_handler \n",char_num);
	if (gl_char[char_num].is_notify==1) {
		notify_pos='0';
		for (uint32_t i=0;i<10;i++) {
			ESP_LOGI(GATTS_TAG, "char%d_notify_handle esp_ble_gatts_send_indicate\n",char_num);
			vTaskDelay(1000 / portTICK_RATE_MS); // delay 1s
			esp_ble_gatts_send_indicate(gatts_if, conn_id, gl_char[char_num].char_handle,1,&notify_pos,false);
			notify_pos++;
		}
	}
}
void char6_notify_handle(esp_gatt_if_t gatts_if, uint16_t conn_id) {
	uint8_t char_num = 5;
	ESP_LOGI(GATTS_TAG, "char%d_notify_handler \n",char_num);
	if (gl_char[char_num].is_notify==1) {
		notify_pos='0';
		for (uint32_t i=0;i<10;i++) {
			ESP_LOGI(GATTS_TAG, "char%d_notify_handle esp_ble_gatts_send_indicate\n",char_num);
			vTaskDelay(1000 / portTICK_RATE_MS); // delay 1s
			esp_ble_gatts_send_indicate(gatts_if, conn_id, gl_char[char_num].char_handle,1,&notify_pos,false);
			notify_pos++;
		}
	}
}

void char1_write_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	uint8_t char_num = 0;
	ESP_LOGI(GATTS_TAG, "char%d_write_handler %d\n", char_num, param->write.handle);
	if (gl_char[char_num].char_val!=NULL) {
		ESP_LOGI(GATTS_TAG, "char%d_write_handler char_val %d\n",char_num,param->write.len);
		gl_char[char_num].char_val->attr_len = param->write.len;
		for (uint32_t pos=0;pos<param->write.len;pos++) {
			gl_char[char_num].char_val->attr_value[pos]=param->write.value[pos];
		}
		ESP_LOGI(GATTS_TAG, "char%d_write_handler %.*s",char_num ,gl_char[char_num].char_val->attr_len, (char*)gl_char[char_num].char_val->attr_value);
	}
	ESP_LOGI(GATTS_TAG, "char%d_write_handler esp_gatt_rsp_t\n",char_num);
	esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
	if (strncmp((const char *)gl_char[char_num].char_val->attr_value,"LED ON",6)==0) {
	} else if (strncmp((const char *)gl_char[char_num].char_val->attr_value,"LED OFF",7)==0) {
	} else if (strncmp((const char *)gl_char[char_num].char_val->attr_value,"LED SWITCH",10)==0) {
	} else {
		char1_notify_handle(gatts_if, param->write.conn_id);
	}
}
void char2_write_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	uint8_t char_num = 1;
	ESP_LOGI(GATTS_TAG, "char%d_write_handler %d\n", char_num, param->write.handle);
	if (gl_char[char_num].char_val!=NULL) {
		ESP_LOGI(GATTS_TAG, "char%d_write_handler char_val %d\n",char_num,param->write.len);
		gl_char[char_num].char_val->attr_len = param->write.len;
		for (uint32_t pos=0;pos<param->write.len;pos++) {
			gl_char[char_num].char_val->attr_value[pos]=param->write.value[pos];
		}
		ESP_LOGI(GATTS_TAG, "char%d_write_handler %.*s", char_num,gl_char[char_num].char_val->attr_len, (char*)gl_char[char_num].char_val->attr_value);
	}
	ESP_LOGI(GATTS_TAG, "char%d_write_handler esp_gatt_rsp_t\n",char_num);
	esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
	if (strncmp((const char *)gl_char[char_num].char_val->attr_value,"LED ON",6)==0) {
	} else if (strncmp((const char *)gl_char[char_num].char_val->attr_value,"LED OFF",7)==0) {
	} else if (strncmp((const char *)gl_char[char_num].char_val->attr_value,"LED SWITCH",10)==0) {
	} else {
		char2_notify_handle(gatts_if, param->write.conn_id);
	}
}
void char3_write_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	uint8_t char_num = 2;
	ESP_LOGI(GATTS_TAG, "char%d_write_handler %d\n", char_num, param->write.handle);
	if (gl_char[char_num].char_val!=NULL) {
		ESP_LOGI(GATTS_TAG, "char%d_write_handler char_val %d\n",char_num,param->write.len);
		gl_char[char_num].char_val->attr_len = param->write.len;
		for (uint32_t pos=0;pos<param->write.len;pos++) {
			gl_char[char_num].char_val->attr_value[pos]=param->write.value[pos];
		}
		ESP_LOGI(GATTS_TAG, "char%d_write_handler %.*s",char_num, gl_char[char_num].char_val->attr_len, (char*)gl_char[char_num].char_val->attr_value);
	}
	ESP_LOGI(GATTS_TAG, "char%d_write_handler esp_gatt_rsp_t\n",char_num);
	esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
	if (strncmp((const char *)gl_char[char_num].char_val->attr_value,"LED ON",6)==0) {
	} else if (strncmp((const char *)gl_char[char_num].char_val->attr_value,"LED OFF",7)==0) {
	} else if (strncmp((const char *)gl_char[char_num].char_val->attr_value,"LED SWITCH",10)==0) {
	} else {
		char3_notify_handle(gatts_if, param->write.conn_id);
	}
}
void char4_write_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	uint8_t char_num = 3;
	ESP_LOGI(GATTS_TAG, "char%d_write_handler %d\n", char_num, param->write.handle);
	if (gl_char[char_num].char_val!=NULL) {
		ESP_LOGI(GATTS_TAG, "char%d_write_handler char_val %d\n",char_num,param->write.len);
		gl_char[char_num].char_val->attr_len = param->write.len;
		for (uint32_t pos=0;pos<param->write.len;pos++) {
			gl_char[char_num].char_val->attr_value[pos]=param->write.value[pos];
		}
		ESP_LOGI(GATTS_TAG, "char%d_write_handler %.*s", char_num,gl_char[char_num].char_val->attr_len, (char*)gl_char[char_num].char_val->attr_value);
	}
	ESP_LOGI(GATTS_TAG, "char%d_write_handler esp_gatt_rsp_t\n",char_num);
	esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
	if (strncmp((const char *)gl_char[char_num].char_val->attr_value,"LED ON",6)==0) {
	} else if (strncmp((const char *)gl_char[char_num].char_val->attr_value,"LED OFF",7)==0) {
	} else if (strncmp((const char *)gl_char[char_num].char_val->attr_value,"LED SWITCH",10)==0) {
	} else {
		char4_notify_handle(gatts_if, param->write.conn_id);
	}
}
void char5_write_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	uint8_t char_num = 4;
	ESP_LOGI(GATTS_TAG, "char%d_write_handler %d\n", char_num, param->write.handle);
	if (gl_char[char_num].char_val!=NULL) {
		ESP_LOGI(GATTS_TAG, "char%d_write_handler char_val %d\n",char_num,param->write.len);
		gl_char[char_num].char_val->attr_len = param->write.len;
		for (uint32_t pos=0;pos<param->write.len;pos++) {
			gl_char[char_num].char_val->attr_value[pos]=param->write.value[pos];
		}
		ESP_LOGI(GATTS_TAG, "char%d_write_handler %.*s",char_num ,gl_char[char_num].char_val->attr_len, (char*)gl_char[char_num].char_val->attr_value);
	}
	ESP_LOGI(GATTS_TAG, "char%d_write_handler esp_gatt_rsp_t\n",char_num);
	esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
	if (strncmp((const char *)gl_char[char_num].char_val->attr_value,"LED ON",6)==0) {
	} else if (strncmp((const char *)gl_char[char_num].char_val->attr_value,"LED OFF",7)==0) {
	} else if (strncmp((const char *)gl_char[char_num].char_val->attr_value,"LED SWITCH",10)==0) {
	} else {
		char5_notify_handle(gatts_if, param->write.conn_id);
	}
}
void char6_write_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	uint8_t char_num = 5;
	ESP_LOGI(GATTS_TAG, "char%d_write_handler %d\n", char_num, param->write.handle);
	if (gl_char[char_num].char_val!=NULL) {
		ESP_LOGI(GATTS_TAG, "char%d_write_handler char_val %d\n",char_num,param->write.len);
		gl_char[char_num].char_val->attr_len = param->write.len;
		for (uint32_t pos=0;pos<param->write.len;pos++) {
			gl_char[char_num].char_val->attr_value[pos]=param->write.value[pos];
		}
		ESP_LOGI(GATTS_TAG, "char%d_write_handler %.*s",char_num ,gl_char[char_num].char_val->attr_len, (char*)gl_char[char_num].char_val->attr_value);
	}
	ESP_LOGI(GATTS_TAG, "char%d_write_handler esp_gatt_rsp_t\n",char_num);
	esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
	if (strncmp((const char *)gl_char[char_num].char_val->attr_value,"LED ON",6)==0) {
	} else if (strncmp((const char *)gl_char[char_num].char_val->attr_value,"LED OFF",7)==0) {
	} else if (strncmp((const char *)gl_char[char_num].char_val->attr_value,"LED SWITCH",10)==0) {
	} else {
		char6_notify_handle(gatts_if, param->write.conn_id);
	}
}

void descr1_write_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	uint8_t char_num = 0;
	ESP_LOGI(GATTS_TAG, "descr%d_write_handler %d\n",char_num,param->write.handle);

	if (gl_char[char_num].descr_val!=NULL) {
		ESP_LOGI(GATTS_TAG, "descr%d_write_handler descr_val %d\n",char_num,param->write.len);
		gl_char[char_num].descr_val->attr_len = param->write.len;
		for (uint32_t pos=0;pos<param->write.len;pos++) {
			gl_char[char_num].descr_val->attr_value[pos]=param->write.value[pos];
		}
	}
	ESP_LOGI(GATTS_TAG, "descr%d_write_handler esp_gatt_rsp_t\n",char_num);
	esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
}
void descr2_write_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	uint8_t char_num = 1;
	ESP_LOGI(GATTS_TAG, "descr%d_write_handler %d\n",char_num,param->write.handle);

	if (gl_char[char_num].descr_val!=NULL) {
		ESP_LOGI(GATTS_TAG, "descr%d_write_handler descr_val %d\n",char_num,param->write.len);
		gl_char[char_num].descr_val->attr_len = param->write.len;
		for (uint32_t pos=0;pos<param->write.len;pos++) {
			gl_char[char_num].descr_val->attr_value[pos]=param->write.value[pos];
		}
	}
	ESP_LOGI(GATTS_TAG, "descr%d_write_handler esp_gatt_rsp_t\n",char_num);
	esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
}
void descr3_write_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	uint8_t char_num = 2;
	ESP_LOGI(GATTS_TAG, "descr%d_write_handler %d\n",char_num,param->write.handle);

	if (gl_char[char_num].descr_val!=NULL) {
		ESP_LOGI(GATTS_TAG, "descr%d_write_handler descr_val %d\n",char_num,param->write.len);
		gl_char[char_num].descr_val->attr_len = param->write.len;
		for (uint32_t pos=0;pos<param->write.len;pos++) {
			gl_char[char_num].descr_val->attr_value[pos]=param->write.value[pos];
		}
	}
	ESP_LOGI(GATTS_TAG, "descr%d_write_handler esp_gatt_rsp_t\n",char_num);
	esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
}
void descr4_write_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	uint8_t char_num = 3;
	ESP_LOGI(GATTS_TAG, "descr%d_write_handler %d\n",char_num,param->write.handle);

	if (gl_char[char_num].descr_val!=NULL) {
		ESP_LOGI(GATTS_TAG, "descr%d_write_handler descr_val %d\n",char_num,param->write.len);
		gl_char[char_num].descr_val->attr_len = param->write.len;
		for (uint32_t pos=0;pos<param->write.len;pos++) {
			gl_char[char_num].descr_val->attr_value[pos]=param->write.value[pos];
		}
	}
	ESP_LOGI(GATTS_TAG, "descr%d_write_handler esp_gatt_rsp_t\n",char_num);
	esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
}
void descr5_write_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	uint8_t char_num = 4;
	ESP_LOGI(GATTS_TAG, "descr%d_write_handler %d\n",char_num,param->write.handle);

	if (gl_char[char_num].descr_val!=NULL) {
		ESP_LOGI(GATTS_TAG, "descr%d_write_handler descr_val %d\n",char_num,param->write.len);
		gl_char[char_num].descr_val->attr_len = param->write.len;
		for (uint32_t pos=0;pos<param->write.len;pos++) {
			gl_char[char_num].descr_val->attr_value[pos]=param->write.value[pos];
		}
	}
	ESP_LOGI(GATTS_TAG, "descr%d_write_handler esp_gatt_rsp_t\n",char_num);
	esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
}
void descr6_write_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	uint8_t char_num = 5;
	ESP_LOGI(GATTS_TAG, "descr%d_write_handler %d\n",char_num,param->write.handle);

	if (gl_char[char_num].descr_val!=NULL) {
		ESP_LOGI(GATTS_TAG, "descr%d_write_handler descr_val %d\n",char_num,param->write.len);
		gl_char[char_num].descr_val->attr_len = param->write.len;
		for (uint32_t pos=0;pos<param->write.len;pos++) {
			gl_char[char_num].descr_val->attr_value[pos]=param->write.value[pos];
		}
	}
	ESP_LOGI(GATTS_TAG, "descr%d_write_handler esp_gatt_rsp_t\n",char_num);
	esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
}

/* One gatt-based profile one app_id and one gatts_if, this array will store the gatts_if returned by ESP_GATTS_REG_EVT */



typedef struct {
	uint8_t                 *prepare_buf;
	int                     prepare_len;
} prepare_type_env_t;

static prepare_type_env_t a_prepare_write_env,b_prepare_write_env,c_prepare_write_env,d_prepare_write_env;

void example_write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param);
void example_exec_write_event_env(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param);

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
	switch (event) {
#ifdef CONFIG_SET_RAW_ADV_DATA
	case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
		adv_config_done &= (~adv_config_flag);
		if (adv_config_done==0){
			esp_ble_gap_start_advertising(&adv_params);
		}
		break;
	case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
		adv_config_done &= (~scan_rsp_config_flag);
		if (adv_config_done==0){
			esp_ble_gap_start_advertising(&adv_params);
		}
		break;
#else
	case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
		adv_config_done &= (~adv_config_flag);
		if (adv_config_done == 0){
			esp_ble_gap_start_advertising(&adv_params);
		}
		break;
	case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
		adv_config_done &= (~scan_rsp_config_flag);
		if (adv_config_done == 0){
			esp_ble_gap_start_advertising(&adv_params);
		}
		break;
#endif
	case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
		//advertising start complete event to indicate advertising start successfully or failed
		if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
			ESP_LOGE(GATTS_TAG, "Advertising start failed\n");
		}
		break;
	case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
		if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
			ESP_LOGE(GATTS_TAG, "Advertising stop failed\n");
		}
		else {
			ESP_LOGI(GATTS_TAG, "Stop adv successfully\n");
		}
		break;
	case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
		ESP_LOGI(GATTS_TAG, "update connection params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
				param->update_conn_params.status,
				param->update_conn_params.min_int,
				param->update_conn_params.max_int,
				param->update_conn_params.conn_int,
				param->update_conn_params.latency,
				param->update_conn_params.timeout);
		break;
	default:
		break;
	}
}

void example_write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param){
	esp_gatt_status_t status = ESP_GATT_OK;
	if (param->write.need_rsp){
		if (param->write.is_prep){
			if (prepare_write_env->prepare_buf == NULL) {
				prepare_write_env->prepare_buf = (uint8_t *)malloc(PREPARE_BUF_MAX_SIZE*sizeof(uint8_t));
				prepare_write_env->prepare_len = 0;
				if (prepare_write_env->prepare_buf == NULL) {
					ESP_LOGE(GATTS_TAG, "Gatt_server prep no mem\n");
					status = ESP_GATT_NO_RESOURCES;
				}
			} else {
				if(param->write.offset > PREPARE_BUF_MAX_SIZE) {
					status = ESP_GATT_INVALID_OFFSET;
				} else if ((param->write.offset + param->write.len) > PREPARE_BUF_MAX_SIZE) {
					status = ESP_GATT_INVALID_ATTR_LEN;
				}
			}

			esp_gatt_rsp_t *gatt_rsp = (esp_gatt_rsp_t *)malloc(sizeof(esp_gatt_rsp_t));
			gatt_rsp->attr_value.len = param->write.len;
			gatt_rsp->attr_value.handle = param->write.handle;
			gatt_rsp->attr_value.offset = param->write.offset;
			gatt_rsp->attr_value.auth_req = ESP_GATT_AUTH_REQ_NONE;
			memcpy(gatt_rsp->attr_value.value, param->write.value, param->write.len);
			esp_err_t response_err = esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, gatt_rsp);
			if (response_err != ESP_OK){
				ESP_LOGE(GATTS_TAG, "Send response error\n");
			}
			free(gatt_rsp);
			if (status != ESP_GATT_OK){
				return;
			}
			memcpy(prepare_write_env->prepare_buf + param->write.offset,
					param->write.value,
					param->write.len);
			prepare_write_env->prepare_len += param->write.len;

		}else{
			esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, NULL);
		}
	}
}

void example_exec_write_event_env(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param){
	if (param->exec_write.exec_write_flag == ESP_GATT_PREP_WRITE_EXEC){
		esp_log_buffer_hex(GATTS_TAG, prepare_write_env->prepare_buf, prepare_write_env->prepare_len);
	}else{
		ESP_LOGI(GATTS_TAG,"ESP_GATT_PREP_WRITE_CANCEL");
	}
	if (prepare_write_env->prepare_buf) {
		free(prepare_write_env->prepare_buf);
		prepare_write_env->prepare_buf = NULL;
	}
	prepare_write_env->prepare_len = 0;
}

void gatts_add_char(uint8_t profile) {

	ESP_LOGI(GATTS_TAG, "gatts_add_char %d in profile %d\n", gl_profile_tab[profile].char_num, profile);

	for (uint32_t pos=0;pos<GATTS_CHAR_NUM_TOTAL;pos++) {
		if (gl_char[pos].char_handle==0) {
			ble_add_char_pos=pos;
			//ESP_LOGI(GATTS_TAG, "ADD pos %d handle %d service %d\n", pos,gl_char[pos].char_handle,gl_profile_tab[profile].service_handle);
			esp_ble_gatts_add_char(gl_profile_tab[profile].service_handle, &gl_char[pos].char_uuid,gl_char[pos].char_perm,gl_char[pos].char_property,gl_char[pos].char_val, gl_char[pos].char_control);
			gl_profile_tab[profile].char_num++;
			break;
		}
	}
}

void gatts_check_add_char(esp_bt_uuid_t char_uuid, uint16_t attr_handle,uint8_t profile) {
	ESP_LOGI(GATTS_TAG, "gatts_check_add_char %d\n", attr_handle);
	if (attr_handle != 0) {
		if (char_uuid.len == ESP_UUID_LEN_16) {
			ESP_LOGI(GATTS_TAG, "Char UUID16: %x", char_uuid.uuid.uuid16);
		} else if (char_uuid.len == ESP_UUID_LEN_32) {
			ESP_LOGI(GATTS_TAG, "Char UUID32: %x", char_uuid.uuid.uuid32);
		} else if (char_uuid.len == ESP_UUID_LEN_128) {
			ESP_LOGI(GATTS_TAG, "Char UUID128: %x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x", char_uuid.uuid.uuid128[0],
					char_uuid.uuid.uuid128[1], char_uuid.uuid.uuid128[2], char_uuid.uuid.uuid128[3],
					char_uuid.uuid.uuid128[4], char_uuid.uuid.uuid128[5], char_uuid.uuid.uuid128[6],
					char_uuid.uuid.uuid128[7], char_uuid.uuid.uuid128[8], char_uuid.uuid.uuid128[9],
					char_uuid.uuid.uuid128[10], char_uuid.uuid.uuid128[11], char_uuid.uuid.uuid128[12],
					char_uuid.uuid.uuid128[13], char_uuid.uuid.uuid128[14], char_uuid.uuid.uuid128[15]);
		} else {
			ESP_LOGE(GATTS_TAG, "Char UNKNOWN LEN %d\n", char_uuid.len);
		}

		ESP_LOGI(GATTS_TAG, "FOUND Char pos %d handle %d\n", ble_add_char_pos,attr_handle);
		gl_char[ble_add_char_pos].char_handle=attr_handle;


		// is there a descriptor to add ?
		if (gl_char[ble_add_char_pos].descr_uuid.len!=0 && gl_char[ble_add_char_pos].descr_handle==0) {
			ESP_LOGI(GATTS_TAG, "ADD Descr pos %d handle %d service %d\n", ble_add_char_pos,gl_char[ble_add_char_pos].descr_handle,gl_profile_tab[profile].service_handle);
			esp_ble_gatts_add_char_descr(gl_profile_tab[profile].service_handle, &gl_char[ble_add_char_pos].descr_uuid,
					gl_char[ble_add_char_pos].descr_perm, gl_char[ble_add_char_pos].descr_val, gl_char[ble_add_char_pos].descr_control);
		} else {
			gatts_add_char(profile);
		}
	}
}

void gatts_check_callback(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	uint16_t handle=0;
	uint8_t read=1;

	switch (event) {
	case ESP_GATTS_READ_EVT: {
		read=1;
		handle=param->read.handle;
		break;
	}
	case ESP_GATTS_WRITE_EVT: {
		read=0;
		handle=param->write.handle;
		break;
	}
	default:
		break;
	}

	ESP_LOGI(GATTS_TAG, "gatts_check_callback read %d num %d handle %d\n", read, GATTS_CHAR_NUM_TOTAL, handle);
	for (uint32_t pos=0;pos<GATTS_CHAR_NUM_TOTAL;pos++) {
		//ESP_LOGI(GATTS_TAG, "gatts_check_callback (%d) in pos %d \n\t\tLooking for handle %d\n", gl_char[pos].char_handle,pos, handle);
		if (gl_char[pos].char_handle==handle) {
			if (read==1) {
				if (gl_char[pos].char_read_callback!=NULL) {
					gl_char[pos].char_read_callback(event, gatts_if, param);
				}
			} else {
				if (gl_char[pos].char_write_callback!=NULL) {
					gl_char[pos].char_write_callback(event, gatts_if, param);
				}
			}
			break;
		}
		//ESP_LOGI(GATTS_TAG, "descr_handle (%d) in pos %d \n\t\tLooking for handle %d\n", gl_char[pos].descr_handle ,pos, handle);
		if (gl_char[pos].char_handle+1==handle) {
			if (read==1) {
				if (gl_char[pos].descr_read_callback!=NULL) {
					gl_char[pos].descr_read_callback(event, gatts_if, param);
				}
			} else {
				if (gl_char[pos].descr_write_callback!=NULL) {
					gl_char[pos].descr_write_callback(event, gatts_if, param);
				}
			}
			break;
		}
	}
}

static void gatts_profile_a_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	uint8_t profile = PROFILE_HR1_APP_ID;
	switch (event) {
	case ESP_GATTS_REG_EVT:

		ESP_LOGI(GATTS_TAG, "REGISTER_APP_EVT, status %d, app_id %d\n", param->reg.status, param->reg.app_id);
		gl_profile_tab[profile].service_id.is_primary = true;
		gl_profile_tab[profile].service_id.id.inst_id = 0x00;
		gl_profile_tab[profile].service_id.id.uuid.len = ESP_UUID_LEN_16;
		gl_profile_tab[profile].service_id.id.uuid.uuid.uuid16 = GATTS_SERVICE_UUID_HEART_RATE;

		esp_ble_gatts_create_service(gatts_if, &gl_profile_tab[profile].service_id, GATTS_NUM_HANDLE_HR);



		break;

	case ESP_GATTS_READ_EVT: {
		ESP_LOGI(GATTS_TAG, "GATT_READ_EVT, conn_id %d, trans_id %d, handle %d\n", param->read.conn_id, param->read.trans_id, param->read.handle);
		/*esp_gatt_rsp_t rsp;
        memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
        rsp.attr_value.len = 4;
        rsp.attr_value.value[0] = 0x11;
        rsp.attr_value.value[1] = 0x11;
        rsp.attr_value.value[2] = 0xbe;
        rsp.attr_value.value[3] = 0xef;
        esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
                                    ESP_GATT_OK, &rsp);*/
		gatts_check_callback(event, gatts_if, param);
		break;
	}
	case ESP_GATTS_WRITE_EVT: {
		ESP_LOGI(GATTS_TAG, "GATT_WRITE_EVT, conn_id %d, trans_id %d, handle %d", param->write.conn_id, param->write.trans_id, param->write.handle);
		ESP_LOGI(GATTS_TAG, "GATT_WRITE_EVT, value len %d, value %08x\n", param->write.len, *(uint32_t *)param->write.value);
		if (!param->write.is_prep){
			esp_log_buffer_hex(GATTS_TAG, param->write.value, param->write.len);

			if (gl_profile_tab[profile].descr_handle == param->write.handle && param->write.len == 2){
				uint16_t descr_value = param->write.value[1]<<8 | param->write.value[0];
				if (descr_value == 0x0001){
					if (ESP_GATT_CHAR_PROP_BIT_NOTIFY){
						ESP_LOGI(GATTS_TAG, "notify enable");
						gl_char[1].is_notify = true;	//notify flag
#ifdef EN_MAX30102_READING_TASK
uint16_t RAWsensorDataRED[FIFO_A_FULL/2], RAWsensorDataIR[FIFO_A_FULL/2];
#ifdef EN_SENSOR0
		max30102_read_fifo(I2C_NUM_0, RAWsensorDataRED,RAWsensorDataIR);//clean fifo
		if(sensor_detected[0]==ESP_OK)
		sensor_detected[0]=max30102_init(I2C_NUM_0);	//start sensors
#endif
#ifdef EN_SENSOR1
		max30102_read_fifo(I2C_NUM_1, RAWsensorDataRED,RAWsensorDataIR);//clean fifo
		if(sensor_detected[1]==ESP_OK)
		sensor_detected[1]=max30102_init(I2C_NUM_1);
#endif
#endif

#ifdef EN_NOTIFY
						if (!notify_task_running){
							xTaskCreate(notify_task_optimized, "notify_task_optimized", 1024 * 4, (void*) 0, 10, &notify_TaskHandle);
						}
#endif
						esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, gl_profile_tab[profile].char_handle[0],
								sizeof(body_location1_str), body_location1_str, false); //false for notify*/
					}
				}else if (descr_value == 0x0002){
					if (ESP_GATT_CHAR_PROP_BIT_INDICATE){
						ESP_LOGI(GATTS_TAG, "indicate enable");
						uint8_t indicate_data[15];
						for (int i = 0; i < sizeof(indicate_data); ++i)
						{
							indicate_data[i] = i%0xff;
						}
						//the size of indicate_data[] need less than MTU size
						esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, gl_profile_tab[profile].char_handle[0],
								sizeof(indicate_data), indicate_data, true);	//true for indicate ()
					}
				}
				else if (descr_value == 0x0000){
					ESP_LOGI(GATTS_TAG, "notify/indicate disable ");
					gl_char[1].is_notify = false;	//notify flag
					//vTaskDelete(notify_TaskHandle);
				}else{
					ESP_LOGE(GATTS_TAG, "unknown descr value");
					esp_log_buffer_hex(GATTS_TAG, param->write.value, param->write.len);
				}
			}
		}
		//example_write_event_env(gatts_if, &a_prepare_write_env, param);
		gatts_check_callback(event, gatts_if, param);
		break;
	}
	case ESP_GATTS_EXEC_WRITE_EVT:
		ESP_LOGI(GATTS_TAG,"ESP_GATTS_EXEC_WRITE_EVT");
		esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
		example_exec_write_event_env(&a_prepare_write_env, param);
		break;
	case ESP_GATTS_MTU_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_MTU_EVT, MTU %d", param->mtu.mtu);
		break;
	case ESP_GATTS_UNREG_EVT:
		break;
	case ESP_GATTS_CREATE_EVT:
		ESP_LOGI(GATTS_TAG, "CREATE_SERVICE_EVT, status %d,  service_handle %d\n", param->create.status, param->create.service_handle);
		gl_profile_tab[profile].service_handle = param->create.service_handle;
		gl_profile_tab[profile].char_uuid.len = ESP_UUID_LEN_16;
		gl_profile_tab[profile].char_uuid.uuid.uuid16 = gl_char[1].char_uuid.uuid.uuid16;

		esp_ble_gatts_start_service(gl_profile_tab[profile].service_handle);
		gatts_add_char(profile);
		break;
	case ESP_GATTS_ADD_INCL_SRVC_EVT:
		break;
	case ESP_GATTS_ADD_CHAR_EVT: {

		ESP_LOGI(GATTS_TAG, "ADD_CHAR_EVT, status 0x%X,  attr_handle %d, service_handle %d\n",
				param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);

		if (param->add_char.status==ESP_GATT_OK) {
			gl_profile_tab[profile].char_handle[0] = param->add_char.attr_handle;
			gatts_check_add_char(param->add_char.char_uuid,param->add_char.attr_handle,profile);
		}

		if (gl_profile_tab[profile].char_num < gl_profile_tab[profile].char_num_total) { //there are more chars to add
			gatts_add_char(profile);
		}else{//organize the char handlers
			for (int i = gl_profile_tab[profile].char_num_total-1; i >=0; i--) {

				uint8_t base_handler = gl_profile_tab[profile].char_handle[0];
				gl_profile_tab[profile].char_handle[i]= base_handler-((gl_profile_tab[profile].char_num_total-i-1)*3);
				//printf("profile: %d\ti=%d\tchar_handler[i]=%d\n",profile,i,gl_profile_tab[profile].char_handle[i]);
			}
		}
		break;
	}
	case ESP_GATTS_ADD_CHAR_DESCR_EVT:
		gl_profile_tab[profile].descr_handle = param->add_char_descr.attr_handle;
		ESP_LOGI(GATTS_TAG, "ADD_DESCR_EVT, status %d, attr_handle %d, service_handle %d\n",
				param->add_char_descr.status, param->add_char_descr.attr_handle, param->add_char_descr.service_handle);
		break;
	case ESP_GATTS_DELETE_EVT:
		break;
	case ESP_GATTS_START_EVT:
		ESP_LOGI(GATTS_TAG, "SERVICE_START_EVT, status %d, service_handle %d\n",
				param->start.status, param->start.service_handle);
		break;
	case ESP_GATTS_STOP_EVT:
		break;
	case ESP_GATTS_CONNECT_EVT: {
#ifdef EN_MAX30102_READING_TASK
#ifdef EN_SENSOR0
		sensor_detected[0]=max30102_reset(I2C_NUM_0);
#endif
#ifdef EN_SENSOR1
		sensor_detected[1]=max30102_reset(I2C_NUM_1);
#endif
#endif

		esp_ble_conn_update_params_t conn_params = {0};
		memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
		/* For the IOS system, please reference the apple official documents about the ble connection parameters restrictions. */
		conn_params.latency = 0;
		conn_params.max_int = 0x20;    // max_int = 0x20*1.25ms = 40ms
		conn_params.min_int = 0x10;    // min_int = 0x10*1.25ms = 20ms
		conn_params.timeout = 3000;    // timeout = 400*10ms = 4000ms
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_CONNECT_EVT, conn_id %d, remote %02x:%02x:%02x:%02x:%02x:%02x:",
				param->connect.conn_id,
				param->connect.remote_bda[0], param->connect.remote_bda[1], param->connect.remote_bda[2],
				param->connect.remote_bda[3], param->connect.remote_bda[4], param->connect.remote_bda[5]);
		gl_profile_tab[profile].conn_id = param->connect.conn_id;
		//start sent the update connection parameters to the peer device.
		esp_ble_gap_update_conn_params(&conn_params);
		//esp_ble_gap_stop_advertising();//todo stop advertising or not?
		break;
	}
	case ESP_GATTS_DISCONNECT_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_DISCONNECT_EVT");
		for (int i = 0; i < GATTS_CHAR_NUM_TOTAL; i++) {//disable all notify
			gl_char[i].is_notify = false;
			if(gl_char[i].char_uuid.uuid.uuid16 == GATTS_CHAR_UUID_HR ||
			   gl_char[i].char_uuid.uuid.uuid16 == GATTS_CHAR_UUID_PLX
			){
				gl_char[i].char_val->attr_value[1]=0;
				gl_char[i].char_val->attr_value[2]=0;
			}
		}
		if(battery_task_running){
			battery_task_running = false;
			vTaskDelete(battery_TaskHandle);
			battery_TaskHandle = NULL;
		}
		BAT_lvl_str[0] = 200;
		//battery_lvl_changed=true;
		//battery_status_changed=true;
		//vTaskDelete(notify_TaskHandle);
		esp_ble_gap_start_advertising(&adv_params);

#ifdef EN_SENSOR0
		if(sensor_detected[0]==ESP_OK)
			max30102_shutdown(I2C_NUM_0);	//shutdown sensor
#endif
#ifdef EN_SENSOR1
		if(sensor_detected[1]==ESP_OK)
			max30102_shutdown(I2C_NUM_1);	//shutdown sensor
#endif

		break;
	case ESP_GATTS_CONF_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_CONF_EVT, status %d", param->conf.status);
		if (param->conf.status != ESP_GATT_OK){
			esp_log_buffer_hex(GATTS_TAG, param->conf.value, param->conf.len);
		}
		break;
	case ESP_GATTS_OPEN_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_OPEN_EVT");
		break;
	case ESP_GATTS_CANCEL_OPEN_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_CANCEL_OPEN_EVT");
		break;
	case ESP_GATTS_CLOSE_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_CLOSE_EVT");
		break;
	case ESP_GATTS_LISTEN_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_LISTEN_EVT");
		break;
	case ESP_GATTS_CONGEST_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_CONGEST_EVT");
		break;
	default:
		break;
	}
}
static void gatts_profile_b_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	uint8_t profile = PROFILE_PLX1_APP_ID;
	switch (event) {
	case ESP_GATTS_REG_EVT:

		ESP_LOGI(GATTS_TAG, "REGISTER_APP_EVT, status %d, app_id %d\n", param->reg.status, param->reg.app_id);
		gl_profile_tab[profile].service_id.is_primary = true;
		gl_profile_tab[profile].service_id.id.inst_id = 0x00;
		gl_profile_tab[profile].service_id.id.uuid.len = ESP_UUID_LEN_16;
		gl_profile_tab[profile].service_id.id.uuid.uuid.uuid16 = GATTS_SERVICE_UUID_PULSE_OXIMETER;

		esp_ble_gatts_create_service(gatts_if, &gl_profile_tab[profile].service_id, GATTS_NUM_HANDLE_PLX);
		break;

	case ESP_GATTS_READ_EVT: {
		ESP_LOGI(GATTS_TAG, "GATT_READ_EVT, conn_id %d, trans_id %d, handle %d\n", param->read.conn_id, param->read.trans_id, param->read.handle);
		/*esp_gatt_rsp_t rsp;
        memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
        rsp.attr_value.len = 4;
        rsp.attr_value.value[0] = 0x11;
        rsp.attr_value.value[1] = 0x11;
        rsp.attr_value.value[2] = 0xbe;
        rsp.attr_value.value[3] = 0xef;
        esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
                                    ESP_GATT_OK, &rsp);*/
		gatts_check_callback(event, gatts_if, param);
		break;
	}
	case ESP_GATTS_WRITE_EVT: {
		ESP_LOGI(GATTS_TAG, "GATT_WRITE_EVT, conn_id %d, trans_id %d, handle %d", param->write.conn_id, param->write.trans_id, param->write.handle);
		ESP_LOGI(GATTS_TAG, "GATT_WRITE_EVT, value len %d, value %08x\n", param->write.len, *(uint32_t *)param->write.value);
		if (!param->write.is_prep){
			ESP_LOGI(GATTS_TAG, "GATT_WRITE_EVT, value len %d, value :", param->write.len);
			esp_log_buffer_hex(GATTS_TAG, param->write.value, param->write.len);
			if (gl_profile_tab[profile].descr_handle == param->write.handle && param->write.len == 2){
				uint16_t descr_value = param->write.value[1]<<8 | param->write.value[0];
				if (descr_value == 0x0001){
					if (ESP_GATT_CHAR_PROP_BIT_NOTIFY){
						ESP_LOGI(GATTS_TAG, "notify enable");
						gl_char[2].is_notify = true;
#ifdef EN_NOTIFY
						if (!notify_task_running){
							xTaskCreate(notify_task_optimized, "notify_task_optimized", 1024 * 4, (void*) 0, 10, &notify_TaskHandle);
						}
#endif
					}
				}else if (descr_value == 0x0002){
					if (ESP_GATT_CHAR_PROP_BIT_INDICATE){
						ESP_LOGI(GATTS_TAG, "indicate enable");
						uint8_t indicate_data[15];
						for (int i = 0; i < sizeof(indicate_data); ++i)
						{
							indicate_data[i] = i%0xff;
						}
						//the size of indicate_data[] need less than MTU size
						esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, gl_profile_tab[profile].char_handle[0],
								sizeof(indicate_data), indicate_data, true);
					}
				}
				else if (descr_value == 0x0000){
					ESP_LOGI(GATTS_TAG, "notify/indicate disable ");
					gl_char[2].is_notify = false;
				}else{
					ESP_LOGE(GATTS_TAG, "unknown descr value");
					esp_log_buffer_hex(GATTS_TAG, param->write.value, param->write.len);
				}

			}
		}
		//example_write_event_env(gatts_if, &b_prepare_write_env, param);
		gatts_check_callback(event, gatts_if, param);
		break;
	}
	case ESP_GATTS_EXEC_WRITE_EVT:
		ESP_LOGI(GATTS_TAG,"ESP_GATTS_EXEC_WRITE_EVT");
		esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
		example_exec_write_event_env(&b_prepare_write_env, param);
		break;
	case ESP_GATTS_MTU_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_MTU_EVT, MTU %d", param->mtu.mtu);
		break;
	case ESP_GATTS_UNREG_EVT:
		break;
	case ESP_GATTS_CREATE_EVT:
		ESP_LOGI(GATTS_TAG, "CREATE_SERVICE_EVT, status %d,  service_handle %d\n", param->create.status, param->create.service_handle);
		gl_profile_tab[profile].service_handle = param->create.service_handle;
		gl_profile_tab[profile].char_uuid.len = ESP_UUID_LEN_16;
		gl_profile_tab[profile].char_uuid.uuid.uuid16 = gl_char[2].char_uuid.uuid.uuid16;

		esp_ble_gatts_start_service(gl_profile_tab[profile].service_handle);
		gatts_add_char(profile);

		break;
	case ESP_GATTS_ADD_INCL_SRVC_EVT:
		break;
	case ESP_GATTS_ADD_CHAR_EVT: {

		ESP_LOGI(GATTS_TAG, "ADD_CHAR_EVT, status 0x%X,  attr_handle %d, service_handle %d\n",
				param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);


		if (param->add_char.status==ESP_GATT_OK) {
			gl_profile_tab[profile].char_handle[0] = param->add_char.attr_handle;
			gatts_check_add_char(param->add_char.char_uuid,param->add_char.attr_handle,profile);
		}
		if (gl_profile_tab[profile].char_num < gl_profile_tab[profile].char_num_total) {
			//gatts_add_char(profile);
		}
		break;
	}
	case ESP_GATTS_ADD_CHAR_DESCR_EVT:
		gl_profile_tab[profile].descr_handle = param->add_char_descr.attr_handle;
		ESP_LOGI(GATTS_TAG, "ADD_DESCR_EVT, status %d, attr_handle %d, service_handle %d\n",
				param->add_char_descr.status, param->add_char_descr.attr_handle, param->add_char_descr.service_handle);
		break;
	case ESP_GATTS_DELETE_EVT:
		break;
	case ESP_GATTS_START_EVT:
		ESP_LOGI(GATTS_TAG, "SERVICE_START_EVT, status %d, service_handle %d\n",
				param->start.status, param->start.service_handle);
		break;
	case ESP_GATTS_STOP_EVT:
		break;
	case ESP_GATTS_CONNECT_EVT: {
		break;
	}
	case ESP_GATTS_DISCONNECT_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_DISCONNECT_EVT");
		break;
	case ESP_GATTS_CONF_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_CONF_EVT, status %d",  param->conf.status);
		if (param->conf.status != ESP_GATT_OK){
			esp_log_buffer_hex(GATTS_TAG, param->conf.value, param->conf.len);
		}
		break;
	case ESP_GATTS_OPEN_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_OPEN_EVT");
		break;
	case ESP_GATTS_CANCEL_OPEN_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_CANCEL_OPEN_EVT");
		break;
	case ESP_GATTS_CLOSE_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_CLOSE_EVT");
		break;
	case ESP_GATTS_LISTEN_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_LISTEN_EVT");
		break;
	case ESP_GATTS_CONGEST_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_CONGEST_EVT");
		break;
	default:
		break;
	}
}
static void gatts_profile_c_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	uint8_t profile = PROFILE_HR2_APP_ID;
	switch (event) {
	case ESP_GATTS_REG_EVT:

		ESP_LOGI(GATTS_TAG, "REGISTER_APP_EVT, status %d, app_id %d\n", param->reg.status, param->reg.app_id);
		gl_profile_tab[profile].service_id.is_primary = true;
		gl_profile_tab[profile].service_id.id.inst_id = 0x00;
		gl_profile_tab[profile].service_id.id.uuid.len = ESP_UUID_LEN_16;
		gl_profile_tab[profile].service_id.id.uuid.uuid.uuid16 = GATTS_SERVICE_UUID_HEART_RATE;

		esp_ble_gatts_create_service(gatts_if, &gl_profile_tab[profile].service_id, GATTS_NUM_HANDLE_HR);

		break;

	case ESP_GATTS_READ_EVT: {
		ESP_LOGI(GATTS_TAG, "GATT_READ_EVT, conn_id %d, trans_id %d, handle %d\n", param->read.conn_id, param->read.trans_id, param->read.handle);
		/*esp_gatt_rsp_t rsp;
        memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
        rsp.attr_value.len = 4;
        rsp.attr_value.value[0] = 0x11;
        rsp.attr_value.value[1] = 0x11;
        rsp.attr_value.value[2] = 0xbe;
        rsp.attr_value.value[3] = 0xef;
        esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
                                    ESP_GATT_OK, &rsp);*/
		gatts_check_callback(event, gatts_if, param);
		break;
	}
	case ESP_GATTS_WRITE_EVT: {
		ESP_LOGI(GATTS_TAG, "GATT_WRITE_EVT, conn_id %d, trans_id %d, handle %d", param->write.conn_id, param->write.trans_id, param->write.handle);
		ESP_LOGI(GATTS_TAG, "GATT_WRITE_EVT, value len %d, value %08x\n", param->write.len, *(uint32_t *)param->write.value);
		if (!param->write.is_prep){
			ESP_LOGI(GATTS_TAG, "GATT_WRITE_EVT, value len %d, value :", param->write.len);
			esp_log_buffer_hex(GATTS_TAG, param->write.value, param->write.len);
			if (gl_profile_tab[profile].descr_handle == param->write.handle && param->write.len == 2){
				uint16_t descr_value = param->write.value[1]<<8 | param->write.value[0];
				if (descr_value == 0x0001){
					if (ESP_GATT_CHAR_PROP_BIT_NOTIFY){
						ESP_LOGI(GATTS_TAG, "notify enable");
						gl_char[4].is_notify = true;
#ifdef EN_NOTIFY
						if (!notify_task_running){
							xTaskCreate(notify_task_optimized, "notify_task_optimized", 1024 * 4, (void*) 0, 10, &notify_TaskHandle);
						}
#endif
					}
				}else if (descr_value == 0x0002){
					if (ESP_GATT_CHAR_PROP_BIT_INDICATE){
						ESP_LOGI(GATTS_TAG, "indicate enable");
						uint8_t indicate_data[15];
						for (int i = 0; i < sizeof(indicate_data); ++i)
						{
							indicate_data[i] = i%0xff;
						}
						//the size of indicate_data[] need less than MTU size
						esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, gl_profile_tab[profile].char_handle[0],
								sizeof(indicate_data), indicate_data, true);
					}
				}
				else if (descr_value == 0x0000){
					ESP_LOGI(GATTS_TAG, "notify/indicate disable ");
					gl_char[4].is_notify = false;
				}else{
					ESP_LOGE(GATTS_TAG, "unknown descr value");
					esp_log_buffer_hex(GATTS_TAG, param->write.value, param->write.len);
				}

			}
		}
		//example_write_event_env(gatts_if, &c_prepare_write_env, param);
		gatts_check_callback(event, gatts_if, param);
		break;
	}
	case ESP_GATTS_EXEC_WRITE_EVT:
		ESP_LOGI(GATTS_TAG,"ESP_GATTS_EXEC_WRITE_EVT");
		esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
		example_exec_write_event_env(&c_prepare_write_env, param);
		break;
	case ESP_GATTS_MTU_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_MTU_EVT, MTU %d", param->mtu.mtu);
		break;
	case ESP_GATTS_UNREG_EVT:
		break;
	case ESP_GATTS_CREATE_EVT:
		ESP_LOGI(GATTS_TAG, "CREATE_SERVICE_EVT, status %d,  service_handle %d\n", param->create.status, param->create.service_handle);
		gl_profile_tab[profile].service_handle = param->create.service_handle;
		gl_profile_tab[profile].char_uuid.len = ESP_UUID_LEN_16;
		gl_profile_tab[profile].char_uuid.uuid.uuid16 = gl_char[5].char_uuid.uuid.uuid16;

		esp_ble_gatts_start_service(gl_profile_tab[profile].service_handle);
		gatts_add_char(profile);

		break;
	case ESP_GATTS_ADD_INCL_SRVC_EVT:
		break;
	case ESP_GATTS_ADD_CHAR_EVT: {
		ESP_LOGI(GATTS_TAG, "ADD_CHAR_EVT, status 0x%X,  attr_handle %d, service_handle %d\n",
				param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);


		if (param->add_char.status==ESP_GATT_OK) {
			gatts_check_add_char(param->add_char.char_uuid,param->add_char.attr_handle,profile);
			gl_profile_tab[profile].char_handle[0] = param->add_char.attr_handle;
		}
		if (gl_profile_tab[profile].char_num < gl_profile_tab[profile].char_num_total) {
			gatts_add_char(profile);
		}else{//organize the char handlers
			for (int i = gl_profile_tab[profile].char_num_total-1; i >=0; i--) {
				uint8_t base_handler = gl_profile_tab[profile].char_handle[0];
				gl_profile_tab[profile].char_handle[i]= base_handler-((gl_profile_tab[profile].char_num_total-i-1)*3);//ex: 67,64,61...
				//printf("profile: %d\ti=%d\tchar_handler[i]=%d\n",profile,i,gl_profile_tab[profile].char_handle[i]);
			}
		}
		break;
	}
	case ESP_GATTS_ADD_CHAR_DESCR_EVT:
		gl_profile_tab[profile].descr_handle = param->add_char_descr.attr_handle;
		ESP_LOGI(GATTS_TAG, "ADD_DESCR_EVT, status %d, attr_handle %d, service_handle %d\n",
				param->add_char_descr.status, param->add_char_descr.attr_handle, param->add_char_descr.service_handle);
		break;
	case ESP_GATTS_DELETE_EVT:
		break;
	case ESP_GATTS_START_EVT:
		ESP_LOGI(GATTS_TAG, "SERVICE_START_EVT, status %d, service_handle %d\n",
				param->start.status, param->start.service_handle);
		break;
	case ESP_GATTS_STOP_EVT:
		break;
	case ESP_GATTS_CONNECT_EVT: {
		break;
	}
	case ESP_GATTS_DISCONNECT_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_DISCONNECT_EVT");
		break;
	case ESP_GATTS_CONF_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_CONF_EVT, status %d",  param->conf.status);
		if (param->conf.status != ESP_GATT_OK){
			esp_log_buffer_hex(GATTS_TAG, param->conf.value, param->conf.len);
		}
		break;
	case ESP_GATTS_OPEN_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_OPEN_EVT");
		break;
	case ESP_GATTS_CANCEL_OPEN_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_CANCEL_OPEN_EVT");
		break;
	case ESP_GATTS_CLOSE_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_CLOSE_EVT");
		break;
	case ESP_GATTS_LISTEN_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_LISTEN_EVT");
		break;
	case ESP_GATTS_CONGEST_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_CONGEST_EVT");
		break;
	default:
		break;
	}
}
static void gatts_profile_d_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	uint8_t profile = PROFILE_PLX2_APP_ID;
	switch (event) {
	case ESP_GATTS_REG_EVT:

		ESP_LOGI(GATTS_TAG, "REGISTER_APP_EVT, status %d, app_id %d\n", param->reg.status, param->reg.app_id);
		gl_profile_tab[profile].service_id.is_primary = true;
		gl_profile_tab[profile].service_id.id.inst_id = 0x00;
		gl_profile_tab[profile].service_id.id.uuid.len = ESP_UUID_LEN_16;
		gl_profile_tab[profile].service_id.id.uuid.uuid.uuid16 = GATTS_SERVICE_UUID_PULSE_OXIMETER;

		esp_ble_gatts_create_service(gatts_if, &gl_profile_tab[profile].service_id, GATTS_NUM_HANDLE_PLX);
		break;

	case ESP_GATTS_READ_EVT: {
		ESP_LOGI(GATTS_TAG, "GATT_READ_EVT, conn_id %d, trans_id %d, handle %d\n", param->read.conn_id, param->read.trans_id, param->read.handle);
		/*esp_gatt_rsp_t rsp;
        memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
        rsp.attr_value.len = 4;
        rsp.attr_value.value[0] = 0x11;
        rsp.attr_value.value[1] = 0x11;
        rsp.attr_value.value[2] = 0xbe;
        rsp.attr_value.value[3] = 0xef;
        esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
                                    ESP_GATT_OK, &rsp);*/
		gatts_check_callback(event, gatts_if, param);
		break;
	}
	case ESP_GATTS_WRITE_EVT: {
		ESP_LOGI(GATTS_TAG, "GATT_WRITE_EVT, conn_id %d, trans_id %d, handle %d", param->write.conn_id, param->write.trans_id, param->write.handle);
		ESP_LOGI(GATTS_TAG, "GATT_WRITE_EVT, value len %d, value %08x\n", param->write.len, *(uint32_t *)param->write.value);
		if (!param->write.is_prep){
			ESP_LOGI(GATTS_TAG, "GATT_WRITE_EVT, value len %d, value :", param->write.len);
			esp_log_buffer_hex(GATTS_TAG, param->write.value, param->write.len);
			if (gl_profile_tab[profile].descr_handle == param->write.handle && param->write.len == 2){
				uint16_t descr_value = param->write.value[1]<<8 | param->write.value[0];
				if (descr_value == 0x0001){
					if (ESP_GATT_CHAR_PROP_BIT_NOTIFY){
						ESP_LOGI(GATTS_TAG, "notify enable");
						gl_char[5].is_notify = true;
#ifdef EN_NOTIFY
						if (!notify_task_running){
							xTaskCreate(notify_task_optimized, "notify_task_optimized", 1024 * 4, (void*) 0, 10, &notify_TaskHandle);
						}
#endif
					}
				}else if (descr_value == 0x0002){
					if (ESP_GATT_CHAR_PROP_BIT_INDICATE){
						ESP_LOGI(GATTS_TAG, "indicate enable");
						uint8_t indicate_data[15];
						for (int i = 0; i < sizeof(indicate_data); ++i)
						{
							indicate_data[i] = i%0xff;
						}

						//the size of indicate_data[] need less than MTU size
						esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, gl_profile_tab[profile].char_handle[0],
								sizeof(indicate_data), indicate_data, true);
					}
				}
				else if (descr_value == 0x0000){
					ESP_LOGI(GATTS_TAG, "notify/indicate disable ");
					gl_char[5].is_notify = false;
				}else{
					ESP_LOGE(GATTS_TAG, "unknown descr value");
					esp_log_buffer_hex(GATTS_TAG, param->write.value, param->write.len);
				}

			}
		}
		//example_write_event_env(gatts_if, &d_prepare_write_env, param);
		gatts_check_callback(event, gatts_if, param);
		break;
	}
	case ESP_GATTS_EXEC_WRITE_EVT:
		ESP_LOGI(GATTS_TAG,"ESP_GATTS_EXEC_WRITE_EVT");
		esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
		example_exec_write_event_env(&d_prepare_write_env, param);
		break;
	case ESP_GATTS_MTU_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_MTU_EVT, MTU %d", param->mtu.mtu);
		break;
	case ESP_GATTS_UNREG_EVT:
		break;
	case ESP_GATTS_CREATE_EVT:
		ESP_LOGI(GATTS_TAG, "CREATE_SERVICE_EVT, status %d,  service_handle %d\n", param->create.status, param->create.service_handle);
		gl_profile_tab[profile].service_handle = param->create.service_handle;
		gl_profile_tab[profile].char_uuid.len = ESP_UUID_LEN_16;
		gl_profile_tab[profile].char_uuid.uuid.uuid16 = gl_char[5].char_uuid.uuid.uuid16;

		esp_ble_gatts_start_service(gl_profile_tab[profile].service_handle);
		gatts_add_char(profile);

		break;
	case ESP_GATTS_ADD_INCL_SRVC_EVT:
		break;
	case ESP_GATTS_ADD_CHAR_EVT: {

		ESP_LOGI(GATTS_TAG, "ADD_CHAR_EVT, status 0x%X,  attr_handle %d, service_handle %d\n",
				param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);

		if (param->add_char.status==ESP_GATT_OK) {
			gl_profile_tab[profile].char_handle[0] = param->add_char.attr_handle;
			gatts_check_add_char(param->add_char.char_uuid,param->add_char.attr_handle,profile);
		}
		if (gl_profile_tab[profile].char_num < gl_profile_tab[profile].char_num_total) {
			gatts_add_char(profile);
		}
		break;
	}
	case ESP_GATTS_ADD_CHAR_DESCR_EVT:
		gl_profile_tab[profile].descr_handle = param->add_char_descr.attr_handle;
		ESP_LOGI(GATTS_TAG, "ADD_DESCR_EVT, status %d, attr_handle %d, service_handle %d\n",
				param->add_char_descr.status, param->add_char_descr.attr_handle, param->add_char_descr.service_handle);
		break;
	case ESP_GATTS_DELETE_EVT:
		break;
	case ESP_GATTS_START_EVT:
		ESP_LOGI(GATTS_TAG, "SERVICE_START_EVT, status %d, service_handle %d\n",
				param->start.status, param->start.service_handle);
		break;
	case ESP_GATTS_STOP_EVT:
		break;
	case ESP_GATTS_CONNECT_EVT: {
		break;
	}
	case ESP_GATTS_DISCONNECT_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_DISCONNECT_EVT");
		break;
	case ESP_GATTS_CONF_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_CONF_EVT, status %d", param->conf.status);
		if (param->conf.status != ESP_GATT_OK){
			esp_log_buffer_hex(GATTS_TAG, param->conf.value, param->conf.len);
		}
		break;
	case ESP_GATTS_OPEN_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_OPEN_EVT");
		break;
	case ESP_GATTS_CANCEL_OPEN_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_CANCEL_OPEN_EVT");
		break;
	case ESP_GATTS_CLOSE_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_CLOSE_EVT");
		break;
	case ESP_GATTS_LISTEN_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_LISTEN_EVT");
		break;
	case ESP_GATTS_CONGEST_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_CONGEST_EVT");
		break;
	default:
		break;
	}
}
static void gatts_profile_e_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	uint8_t profile = PROFILE_RAW_APP_ID;
	switch (event) {
	case ESP_GATTS_REG_EVT:

		ESP_LOGI(GATTS_TAG, "REGISTER_APP_EVT, status %d, app_id %d\n", param->reg.status, param->reg.app_id);
		gl_profile_tab[profile].service_id.is_primary = true;
		gl_profile_tab[profile].service_id.id.inst_id = 0x00;
		gl_profile_tab[profile].service_id.id.uuid.len = ESP_UUID_LEN_16;
		gl_profile_tab[profile].service_id.id.uuid.uuid.uuid16 = GATTS_SERVICE_UUID_RAW_DATA_SERVICE;

		esp_ble_gatts_create_service(gatts_if, &gl_profile_tab[profile].service_id, GATTS_NUM_HANDLE_RAW);
		break;

	case ESP_GATTS_READ_EVT: {
		ESP_LOGI(GATTS_TAG, "GATT_READ_EVT, conn_id %d, trans_id %d, handle %d\n", param->read.conn_id, param->read.trans_id, param->read.handle);
		/*esp_gatt_rsp_t rsp;
        memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
        rsp.attr_value.len = 4;
        rsp.attr_value.value[0] = 0x11;
        rsp.attr_value.value[1] = 0x11;
        rsp.attr_value.value[2] = 0xbe;
        rsp.attr_value.value[3] = 0xef;
        esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
                                    ESP_GATT_OK, &rsp);*/
		gatts_check_callback(event, gatts_if, param);
		break;
	}
	case ESP_GATTS_WRITE_EVT: {
		ESP_LOGI(GATTS_TAG, "GATT_WRITE_EVT, conn_id %d, trans_id %d, handle %d", param->write.conn_id, param->write.trans_id, param->write.handle);
		ESP_LOGI(GATTS_TAG, "GATT_WRITE_EVT, value len %d, value %08x\n", param->write.len, *(uint32_t *)param->write.value);
		if (!param->write.is_prep){
			ESP_LOGI(GATTS_TAG, "GATT_WRITE_EVT, value len %d, value :", param->write.len);
			esp_log_buffer_hex(GATTS_TAG, param->write.value, param->write.len);
			if ((gl_profile_tab[profile].descr_handle == param->write.handle
					|| gl_profile_tab[profile].descr_handle == param->write.handle+3)
					&& param->write.len == 2){
				uint16_t descr_value = param->write.value[1]<<8 | param->write.value[0];
				uint8_t raw1 = !(gl_profile_tab[profile].descr_handle == param->write.handle+3);
				if (descr_value == 0x0001){
					if (ESP_GATT_CHAR_PROP_BIT_NOTIFY){
						if (raw1){
							gl_char[7].is_notify = true;
						}else{
							gl_char[6].is_notify = true;
						}
#ifdef EN_NOTIFY
if (!notify_task_running){
	xTaskCreate(notify_task_optimized, "notify_task_optimized", 1024 * 4, (void*) 0, 10, &notify_TaskHandle);
}
#endif
					}
				}else if (descr_value == 0x0002){
					if (ESP_GATT_CHAR_PROP_BIT_INDICATE){
						ESP_LOGI(GATTS_TAG, "indicate enable");
						uint8_t indicate_data[15];
						for (int i = 0; i < sizeof(indicate_data); ++i)
						{
							indicate_data[i] = i%0xff;
						}

						//the size of indicate_data[] need less than MTU size
						esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, gl_profile_tab[profile].char_handle[0],
								sizeof(indicate_data), indicate_data, true);
					}
				}
				else if (descr_value == 0x0000){
					ESP_LOGI(GATTS_TAG, "notify/indicate disable ");
					if (raw1){
						gl_char[7].is_notify = false;
					}else {
						gl_char[6].is_notify = false;
					}
				}else{
					ESP_LOGE(GATTS_TAG, "unknown descr value");
					esp_log_buffer_hex(GATTS_TAG, param->write.value, param->write.len);
				}

			}
		}
		//example_write_event_env(gatts_if, &d_prepare_write_env, param);
		gatts_check_callback(event, gatts_if, param);
		break;
	}
	case ESP_GATTS_EXEC_WRITE_EVT:
		ESP_LOGI(GATTS_TAG,"ESP_GATTS_EXEC_WRITE_EVT");
		esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
		example_exec_write_event_env(&d_prepare_write_env, param);
		break;
	case ESP_GATTS_MTU_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_MTU_EVT, MTU %d", param->mtu.mtu);
		break;
	case ESP_GATTS_UNREG_EVT:
		break;
	case ESP_GATTS_CREATE_EVT:
		ESP_LOGI(GATTS_TAG, "CREATE_SERVICE_EVT, status %d,  service_handle %d\n", param->create.status, param->create.service_handle);
		gl_profile_tab[profile].service_handle = param->create.service_handle;
		gl_profile_tab[profile].char_uuid.len = ESP_UUID_LEN_16;
		gl_profile_tab[profile].char_uuid.uuid.uuid16 = gl_char[6].char_uuid.uuid.uuid16;

		esp_ble_gatts_start_service(gl_profile_tab[profile].service_handle);
		gatts_add_char(profile);

		break;
	case ESP_GATTS_ADD_INCL_SRVC_EVT:
		break;
	case ESP_GATTS_ADD_CHAR_EVT: {

		ESP_LOGI(GATTS_TAG, "ADD_CHAR_EVT, status 0x%X,  attr_handle %d, service_handle %d\n",
				param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);

		if (param->add_char.status==ESP_GATT_OK) {
			gl_profile_tab[profile].char_handle[0] = param->add_char.attr_handle;
			gatts_check_add_char(param->add_char.char_uuid,param->add_char.attr_handle,profile);
		}
		if (gl_profile_tab[profile].char_num < gl_profile_tab[profile].char_num_total) { //there are more chars to add
			gatts_add_char(profile);
		}else{//organize the char handlers
			for (int i = gl_profile_tab[profile].char_num_total-1; i >=0; i--) {

				uint8_t base_handler = gl_profile_tab[profile].char_handle[0];
				gl_profile_tab[profile].char_handle[i]= base_handler-((gl_profile_tab[profile].char_num_total-i-1)*3);
				//printf("profile: %d\ti=%d\tchar_handler[i]=%d\n",profile,i,gl_profile_tab[profile].char_handle[i]);
			}
		}
		break;
	}
	case ESP_GATTS_ADD_CHAR_DESCR_EVT:
		gl_profile_tab[profile].descr_handle = param->add_char_descr.attr_handle;
		ESP_LOGI(GATTS_TAG, "ADD_DESCR_EVT, status %d, attr_handle %d, service_handle %d\n",
				param->add_char_descr.status, param->add_char_descr.attr_handle, param->add_char_descr.service_handle);
		break;
	case ESP_GATTS_DELETE_EVT:
		break;
	case ESP_GATTS_START_EVT:
		ESP_LOGI(GATTS_TAG, "SERVICE_START_EVT, status %d, service_handle %d\n",
				param->start.status, param->start.service_handle);
		break;
	case ESP_GATTS_STOP_EVT:
		break;
	case ESP_GATTS_CONNECT_EVT: {
		break;
	}
	case ESP_GATTS_DISCONNECT_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_DISCONNECT_EVT");
		break;
	case ESP_GATTS_CONF_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_CONF_EVT, status %d", param->conf.status);
		if (param->conf.status != ESP_GATT_OK){
			esp_log_buffer_hex(GATTS_TAG, param->conf.value, param->conf.len);
		}
		break;
	case ESP_GATTS_OPEN_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_OPEN_EVT");
		break;
	case ESP_GATTS_CANCEL_OPEN_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_CANCEL_OPEN_EVT");
		break;
	case ESP_GATTS_CLOSE_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_CLOSE_EVT");
		break;
	case ESP_GATTS_LISTEN_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_LISTEN_EVT");
		break;
	case ESP_GATTS_CONGEST_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_CONGEST_EVT");
		break;
	default:
		break;
	}
}
static void gatts_profile_f_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	uint8_t profile = PROFILE_BATT_APP_ID;
	switch (event) {
	case ESP_GATTS_REG_EVT:

		ESP_LOGI(GATTS_TAG, "REGISTER_APP_EVT, status %d, app_id %d\n", param->reg.status, param->reg.app_id);
		gl_profile_tab[profile].service_id.is_primary = true;
		gl_profile_tab[profile].service_id.id.inst_id = 0x00;
		gl_profile_tab[profile].service_id.id.uuid.len = ESP_UUID_LEN_16;
		gl_profile_tab[profile].service_id.id.uuid.uuid.uuid16 = GATTS_SERVICE_UUID_BATTERY_SERVICE;

		esp_ble_gatts_create_service(gatts_if, &gl_profile_tab[profile].service_id, GATTS_NUM_HANDLE_BAT);
		break;

	case ESP_GATTS_READ_EVT: {
		ESP_LOGI(GATTS_TAG, "GATT_READ_EVT, conn_id %d, trans_id %d, handle %d\n", param->read.conn_id, param->read.trans_id, param->read.handle);
		/*esp_gatt_rsp_t rsp;
        memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
        rsp.attr_value.len = 4;
        rsp.attr_value.value[0] = 0x11;
        rsp.attr_value.value[1] = 0x11;
        rsp.attr_value.value[2] = 0xbe;
        rsp.attr_value.value[3] = 0xef;
        esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
                                    ESP_GATT_OK, &rsp);*/
		gatts_check_callback(event, gatts_if, param);
		break;
	}
	case ESP_GATTS_WRITE_EVT: {
		ESP_LOGI(GATTS_TAG, "GATT_WRITE_EVT, conn_id %d, trans_id %d, handle %d", param->write.conn_id, param->write.trans_id, param->write.handle);
		ESP_LOGI(GATTS_TAG, "GATT_WRITE_EVT, value len %d, value %08x\n", param->write.len, *(uint32_t *)param->write.value);
		if (!param->write.is_prep){
			ESP_LOGI(GATTS_TAG, "GATT_WRITE_EVT, value len %d, value :", param->write.len);
			esp_log_buffer_hex(GATTS_TAG, param->write.value, param->write.len);
			if ((gl_profile_tab[profile].descr_handle == param->write.handle
								|| gl_profile_tab[profile].descr_handle == param->write.handle+3)
								&& param->write.len == 2){
				uint16_t descr_value = param->write.value[1]<<8 | param->write.value[0];
				bool is_bat_level = (gl_profile_tab[profile].descr_handle == param->write.handle+3);
				if (descr_value == 0x0001){
					if (ESP_GATT_CHAR_PROP_BIT_NOTIFY){
						ESP_LOGI(GATTS_TAG, "notify enable");
						if (is_bat_level){
							gl_char[8].is_notify = true; 	//notify bat lvl
							ESP_LOGW("Notify","BAT_LVL");
#ifdef EN_BATTERY_MEASURMENT_TASK
							if(!battery_task_running){
								xTaskCreate(batt_level_task, "batt_level_task", 1024 * 2, (void*) 0, 10, &battery_TaskHandle);
							}
#endif
						}else{
							gl_char[9].is_notify = true;	//notify bat state
							ESP_LOGW("Notify","BAT_STATE");
							xTaskCreate(batt_state_task, "i2c_test_task_0", 1024 * 2, (void* ) 0, 10, NULL);
						}
#ifdef EN_NOTIFY
						if (!notify_task_running){
							xTaskCreate(notify_task_optimized, "notify_task_optimized", 1024 * 4, (void*) 0, 10, &notify_TaskHandle);
						}
#else
						ESP_LOGI(GATTS_TAG, "notify not defined ");
#endif
					}
				}else if (descr_value == 0x0002){
					if (ESP_GATT_CHAR_PROP_BIT_INDICATE){
						ESP_LOGI(GATTS_TAG, "indicate enable");
						uint8_t indicate_data[15];
						for (int i = 0; i < sizeof(indicate_data); ++i)
						{
							indicate_data[i] = i%0xff;
						}

						//the size of indicate_data[] need less than MTU size
						esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, gl_profile_tab[profile].char_handle[0],
								sizeof(indicate_data), indicate_data, true);
					}
				}
				else if (descr_value == 0x0000){
					ESP_LOGI(GATTS_TAG, "notify/indicate disable ");
					if (is_bat_level){
						gl_char[8].is_notify = false;
						if(battery_task_running){
							battery_task_running = false;
							vTaskDelete(battery_TaskHandle);
							battery_TaskHandle = NULL;
						}
					}else {
						gl_char[9].is_notify = false;
					}
				}else{
					ESP_LOGE(GATTS_TAG, "unknown descr value");
					esp_log_buffer_hex(GATTS_TAG, param->write.value, param->write.len);
				}

			}
		}
		//example_write_event_env(gatts_if, &d_prepare_write_env, param);
		gatts_check_callback(event, gatts_if, param);
		break;
	}
	case ESP_GATTS_EXEC_WRITE_EVT:
		ESP_LOGI(GATTS_TAG,"ESP_GATTS_EXEC_WRITE_EVT");
		esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
		example_exec_write_event_env(&d_prepare_write_env, param);
		break;
	case ESP_GATTS_MTU_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_MTU_EVT, MTU %d", param->mtu.mtu);
		break;
	case ESP_GATTS_UNREG_EVT:
		break;
	case ESP_GATTS_CREATE_EVT:
		ESP_LOGI(GATTS_TAG, "CREATE_SERVICE_EVT, status %d,  service_handle %d\n", param->create.status, param->create.service_handle);
		gl_profile_tab[profile].service_handle = param->create.service_handle;
		gl_profile_tab[profile].char_uuid.len = ESP_UUID_LEN_16;
		gl_profile_tab[profile].char_uuid.uuid.uuid16 = gl_char[8].char_uuid.uuid.uuid16;

		esp_ble_gatts_start_service(gl_profile_tab[profile].service_handle);
		gatts_add_char(profile);

		break;
	case ESP_GATTS_ADD_INCL_SRVC_EVT:
		break;
	case ESP_GATTS_ADD_CHAR_EVT: {

		ESP_LOGI(GATTS_TAG, "ADD_CHAR_EVT, status 0x%X,  attr_handle %d, service_handle %d\n",
				param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);

		if (param->add_char.status==ESP_GATT_OK) {
			gl_profile_tab[profile].char_handle[0] = param->add_char.attr_handle;
			gatts_check_add_char(param->add_char.char_uuid,param->add_char.attr_handle,profile);
		}
		if (gl_profile_tab[profile].char_num < gl_profile_tab[profile].char_num_total) {
			gatts_add_char(profile);
		}else{//organize the char handlers
			for (int i = gl_profile_tab[profile].char_num_total-1; i >=0; i--) {

				uint8_t base_handler = gl_profile_tab[profile].char_handle[0];
				gl_profile_tab[profile].char_handle[i]= base_handler-((gl_profile_tab[profile].char_num_total-i-1)*3);
				//printf("profile: %d\ti=%d\tchar_handler[i]=%d\n",profile,i,gl_profile_tab[profile].char_handle[i]);
			}
		}
		break;
	}
	case ESP_GATTS_ADD_CHAR_DESCR_EVT:
		gl_profile_tab[profile].descr_handle = param->add_char_descr.attr_handle;
		ESP_LOGI(GATTS_TAG, "ADD_DESCR_EVT, status %d, attr_handle %d, service_handle %d\n",
				param->add_char_descr.status, param->add_char_descr.attr_handle, param->add_char_descr.service_handle);
		break;
	case ESP_GATTS_DELETE_EVT:
		break;
	case ESP_GATTS_START_EVT:
		ESP_LOGI(GATTS_TAG, "SERVICE_START_EVT, status %d, service_handle %d\n",
				param->start.status, param->start.service_handle);
		break;
	case ESP_GATTS_STOP_EVT:
		break;
	case ESP_GATTS_CONNECT_EVT: {
		break;
	}
	case ESP_GATTS_DISCONNECT_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_DISCONNECT_EVT");
		break;
	case ESP_GATTS_CONF_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_CONF_EVT, status %d", param->conf.status);
		if (param->conf.status != ESP_GATT_OK){
			esp_log_buffer_hex(GATTS_TAG, param->conf.value, param->conf.len);
		}
		break;
	case ESP_GATTS_OPEN_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_OPEN_EVT");
		break;
	case ESP_GATTS_CANCEL_OPEN_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_CANCEL_OPEN_EVT");
		break;
	case ESP_GATTS_CLOSE_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_CLOSE_EVT");
		break;
	case ESP_GATTS_LISTEN_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_LISTEN_EVT");
		break;
	case ESP_GATTS_CONGEST_EVT:
		ESP_LOGI(GATTS_TAG, "ESP_GATTS_CONGEST_EVT");
		break;
	default:
		break;
	}
}

static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
	/* If event is register event, store the gatts_if for each profile */
	if (event == ESP_GATTS_REG_EVT) {
		if (param->reg.status == ESP_GATT_OK) {
			gl_profile_tab[param->reg.app_id].gatts_if = gatts_if;
		} else {
			ESP_LOGI(GATTS_TAG, "Reg app failed, app_id %04x, status %d\n",
					param->reg.app_id,
					param->reg.status);
			return;
		}
	}

	/* If the gatts_if equal to profile A, call profile A cb handler,
	 * so here call each profile's callback */
	do {
		for (int idx = 0; idx < PROFILE_TOTAL_NUM; idx++) {
			if (gatts_if == ESP_GATT_IF_NONE || /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
					gatts_if == gl_profile_tab[idx].gatts_if) {
				if (gl_profile_tab[idx].gatts_cb) {
					gl_profile_tab[idx].gatts_cb(event, gatts_if, param);
				}
			}
		}
	} while (0);
}

void bt_main(){
	esp_err_t ret;

	// Initialize NVS.
	ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK( ret );
	//ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

	esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
	ret = esp_bt_controller_init(&bt_cfg);
	if (ret) {
		ESP_LOGE(GATTS_TAG, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(ret));
		return;
	}
	ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
	if (ret) {
		ESP_LOGE(GATTS_TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
		return;
	}

	ret = esp_bluedroid_init();
	if (ret) {
		ESP_LOGE(GATTS_TAG, "%s init bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
		return;
	}
	ret = esp_bluedroid_enable();
	if (ret) {
		ESP_LOGE(GATTS_TAG, "%s enable bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
		return;
	}

	ret = esp_ble_gatts_register_callback(gatts_event_handler);
	if (ret){
		ESP_LOGE(GATTS_TAG, "gatts register error, error code = %x", ret);
		return;
	}

	ret = esp_ble_gap_register_callback(gap_event_handler);
	if (ret){
		ESP_LOGE(GATTS_TAG, "gap register error, error code = %x", ret);
		return;
	}

	ret = esp_ble_gap_set_device_name(TEST_DEVICE_NAME);
	if (ret){
		ESP_LOGE(GATTS_TAG, "set device name failed, error code = %x", ret);
	}

	//config adv data
	ret = esp_ble_gap_config_adv_data(&adv_data);
	if (ret){
		ESP_LOGE(GATTS_TAG, "config adv data failed, error code = %x", ret);
	}
	adv_config_done |= adv_config_flag;
	//config scan response data
	ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
	if (ret){
		ESP_LOGE(GATTS_TAG, "config scan response data failed, error code = %x", ret);
	}
	adv_config_done |= scan_rsp_config_flag;

	for (int profile = 0; profile < PROFILE_TOTAL_NUM; profile++) {	//register all profiles
		ret = esp_ble_gatts_app_register(profile);
		if (ret){
			ESP_LOGE(GATTS_TAG, "gatts app register error, error code = %x", ret);
			return;
		}
	}

	esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(FIFO_A_FULL *PACKS_TO_SEND + 3);//Define MTU size 120 bytes + 1 byte op_code + 2 byte handle
	if (local_mtu_ret){
		ESP_LOGE(GATTS_TAG, "set local  MTU failed, error code = %x", local_mtu_ret);
	}

	//esp_bt_sleep_enable();
	esp_ble_tx_power_set(9, 0);	//https://dl.espressif.com/doc/esp-idf/latest/api-reference/bluetooth/controller_vhci.html#_CPPv220esp_ble_tx_power_set20esp_ble_power_type_t17esp_power_level_t
	return;
}

uint8_t get_n_notify(){
	n_notify = 0;
	for (int i = 0; i < GATTS_CHAR_NUM_TOTAL; ++i) {
		if(gl_char[i].is_notify){
			n_notify++;
		}
	}
	return n_notify;
}

void print_array(uint8_t *array,uint16_t size){
	for (int  i= 0;  i< size; i++) {
		printf("array[%d]=0x%x\n",i,array[i]);
	}
}

void max30102_blink(int n_times, int period, i2c_port_t port){
/* n_times - number of times to blink
 * 	period  - period of blinking in MS
 * 	port	- i2c port to use
 * */
	for (int i = 0; i < n_times; i++) {

		max30102_write_reg(REG_PILOT_PA,port,50);
		vTaskDelay(period / portTICK_RATE_MS);
		max30102_write_reg(REG_PILOT_PA,port,1);
		vTaskDelay(period / portTICK_RATE_MS);
	}
}

//void vApplicationIdleHook(void* arg){
//	//core0_idle_time++;
//	//core1_idle_time++;
//}

void idle_task_print(void* arg){
	do{
		//ESP_LOGW("IDLE","CPU0: %.0f,\t%.0f",core0_idle_time,core0_idle_time-core0_idle_time_last);
		//ESP_LOGW("IDLE","CPU1: %.0f,\t%.0f",core1_idle_time,core1_idle_time-core1_idle_time_last);
		core0_idle_time_last = core0_idle_time;
		core1_idle_time_last = core1_idle_time;
		vTaskDelay(10000 / portTICK_RATE_MS); // delay 10s
	}while(1);
}

void idle_task_0(void* arg){
	//for(;;){
		core0_idle_time++;
	//}
}

void idle_task_1(void* arg){
	//for(;;){
		core1_idle_time++;
	//}
}

















