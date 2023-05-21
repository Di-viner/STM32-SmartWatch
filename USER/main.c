#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "lcd.h"
#include "key.h"
#include "beep.h"
#include "timer.h"
#include "sram.h"
#include "touch.h"
#include "adc.h"
#include "adc3.h"
#include "lsens.h"
#include "rtc.h"

#include "mpu6050.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"

#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"

#include "gui_app.h"
#include "MeterStepAlgorithm.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>

// ��Ҫ���µ�����
// ʱ��ҳ��UI�Ż������������ʾ
// ����ҳ��UI�Ż�������������ѡ���һ����ɫ

// �������ȼ�
#define START_TASK_PRIO 1
// �����ջ��С
#define START_STK_SIZE 128
// ������
TaskHandle_t StartTask_Handler;
// ������
void start_task(void *pvParameters);

// �������ȼ�
#define LED0_TASK_PRIO 3
// �����ջ��С
#define LED0_STK_SIZE 50
// ������
TaskHandle_t LED0Task_Handler;
// ������
void led0_task(void *pvParameters);

// �������ȼ�
#define LED1_TASK_PRIO 4
// �����ջ��С
#define LED1_STK_SIZE 50
// ������
TaskHandle_t LED1Task_Handler;
// ������
void led1_task(void *pvParameters);

// �������ȼ�
#define LVGL_TASK_PRIO 2
// �����ջ��С
#define LVGL_STK_SIZE 1024
// ������
TaskHandle_t LVGLTask_Handler;
// ������
void lvgl_task(void *pvParameters);

// �������ȼ�
#define ADC_TASK_PRIO 5
// �����ջ��С
#define ADC_STK_SIZE 128
// ������
TaskHandle_t ADCTask_Handler;
// ������
void adc_task(void *pvParameters);

// �������ȼ�
#define MPU_TASK_PRIO 6
// �����ջ��С
#define MPU_STK_SIZE 256
// ������
TaskHandle_t MPUTask_Handler;
// ������
void mpu_task(void *pvParameters);

// �������ȼ�
#define KEY_TASK_PRIO 7
// �����ջ��С
#define KEY_STK_SIZE 50
// ������
TaskHandle_t KEYTask_Handler;
// ������
void key_task(void *pvParameters);

// �������ȼ�
#define ALARM_TASK_PRIO 8
// �����ջ��С
#define ALARM_STK_SIZE 50
// ������
TaskHandle_t ALARMTask_Handler;
// ������
void alarm_task(void *pvParameters);

// ȫ�ֱ���
// ʱ��
uint8_t hour = 14;
uint8_t min = 35;
uint8_t sec = 0;
uint16_t year = 2022;
uint8_t month = 11;
uint8_t day = 29;

// adc
u8 light_val = 0;
u16 temp_val = 0;

// ����
uint8_t num = 0; // ����ɨ������
char is_Alarm = 0;

// �˶����
personInfo_t My_info = {170 / 100, 110 / 2}; // �������ߡ�����
sportsInfo_t SportsInfo;					 // װ��������·�·�̵Ľṹ�����
ADD_SPEED as;								 // ��ż��ٶ�
RTC_TimeTypeDef Rtc_Time;					 // ��ȡRTCʱ��
static u8 tempSecond;						 // ����������̬��
static timeStamp_t timeStamp = {0};			 // ��ʱ�����20�����ڵļ���

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); // ����ϵͳ�ж����ȼ�����4
	delay_init(168);								// ��ʼ����ʱ����
	uart_init(115200);								// ��ʼ�����ڲ�����Ϊ115200

	LED_Init();								  // ��ʼ��LED
	My_RTC_Init();							  // ��ʼ��RTC
	RTC_Set_Time(hour, min, sec, RTC_H12_PM); // ����ʱ��
	RTC_Set_Date(22, 11, 29, 2);			  // ��������
	MPU_Init();
	LCD_Init();	 // LCD��ʼ��
	KEY_Init();	 // ������ʼ��
	BEEP_Init(); // ��������ʼ��

	TIM3_Int_Init(9999, 8399); // ��ʼ��1s��ʱ��TIM3��1s 1�ж�
	TIM5_Int_Init(9999, 8399); // 10s ��ʱ��

	FSMC_SRAM_Init(); // ��ʼ���ⲿsram
	tp_dev.init();	  // ��������ʼ��
	Adc_Init();		  // ADC��ʼ��
	Lsens_Init();	  // ��ʼ������������

	while (mpu_dmp_init()) // ��ʼ��dmp
	{
		LCD_ShowString(30, 130, 200, 16, 16, "MPU6050 Error");
		delay_ms(200);
		LCD_Fill(30, 130, 239, 130 + 16, WHITE);
		delay_ms(200);
	}

	lv_init();			  // lvglϵͳ��ʼ��
	lv_port_disp_init();  // lvgl��ʾ�ӿڳ�ʼ��,����lv_init()�ĺ���
	lv_port_indev_init(); // lvgl����ӿڳ�ʼ��,����lv_init()�ĺ���

	// BEEP = 1; //�����ɹ�
	// delay_ms(50);
	// BEEP = 0;

	gui_app_start(); // ��������

	// ������ʼ����
	xTaskCreate((TaskFunction_t)start_task,			 // ������
				(const char *)"start_task",			 // ��������
				(uint16_t)START_STK_SIZE,			 // �����ջ��С
				(void *)NULL,						 // ���ݸ��������Ĳ���
				(UBaseType_t)START_TASK_PRIO,		 // �������ȼ�
				(TaskHandle_t *)&StartTask_Handler); // ������
	vTaskStartScheduler();							 // �����������
}

// ��ʼ����������
void start_task(void *pvParameters)
{
	taskENTER_CRITICAL(); // �����ٽ���
	// ����LED0����
	xTaskCreate((TaskFunction_t)led0_task,
				(const char *)"led0_task",
				(uint16_t)LED0_STK_SIZE,
				(void *)NULL,
				(UBaseType_t)LED0_TASK_PRIO,
				(TaskHandle_t *)&LED0Task_Handler);

	// ����LED1����
	xTaskCreate((TaskFunction_t)led1_task,
				(const char *)"led1_task",
				(uint16_t)LED1_STK_SIZE,
				(void *)NULL,
				(UBaseType_t)LED1_TASK_PRIO,
				(TaskHandle_t *)&LED1Task_Handler);

	// lvgl
	xTaskCreate((TaskFunction_t)lvgl_task,
				(const char *)"lvgl_task",
				(uint16_t)LVGL_STK_SIZE,
				(void *)NULL,
				(UBaseType_t)LVGL_TASK_PRIO,
				(TaskHandle_t *)&LVGLTask_Handler);

	// adc
	xTaskCreate((TaskFunction_t)adc_task,
				(const char *)"adc_task",
				(uint16_t)ADC_STK_SIZE,
				(void *)NULL,
				(UBaseType_t)ADC_TASK_PRIO,
				(TaskHandle_t *)&ADCTask_Handler);

	// mpu
	xTaskCreate((TaskFunction_t)mpu_task,
				(const char *)"mpu_task",
				(uint16_t)MPU_STK_SIZE,
				(void *)NULL,
				(UBaseType_t)MPU_TASK_PRIO,
				(TaskHandle_t *)&MPUTask_Handler);

	// key
	xTaskCreate((TaskFunction_t)key_task,
				(const char *)"key_task",
				(uint16_t)KEY_STK_SIZE,
				(void *)NULL,
				(UBaseType_t)KEY_TASK_PRIO,
				(TaskHandle_t *)&KEYTask_Handler);

	// alarm
	xTaskCreate((TaskFunction_t)alarm_task,
				(const char *)"alarm_task",
				(uint16_t)ALARM_STK_SIZE,
				(void *)NULL,
				(UBaseType_t)ALARM_TASK_PRIO,
				(TaskHandle_t *)&ALARMTask_Handler);

	vTaskDelete(StartTask_Handler); // ɾ����ʼ����
	taskEXIT_CRITICAL();			// �˳��ٽ���
}

// LED0������ ���������ʱ��
void led0_task(void *pvParameters)
{
	while (1)
	{
		LED0 = ~LED0;

		vTaskDelay(500);
	}
}

// LED1������
void led1_task(void *pvParameters)
{
	while (1)
	{
		LED1 = 0;
		vTaskDelay(200);
		LED1 = 1;
		vTaskDelay(800);
	}
}

// lvgl����
void lvgl_task(void *pvParameters)
{
	while (1)
	{
		tp_dev.scan(0);
		lv_task_handler();
		vTaskDelay(10);
	}
}

// adc����
void adc_task(void *pvParameters)
{
	while (1)
	{
		temp_val = Get_Temprate();	 // Ĭ��Ϊ����
		light_val = Lsens_Get_Val(); // Ĭ��Ϊ����
		// printf("Temp:%d.%d;Adcx:%d\r\n",(temp_val/100)%100,temp_val%100,light_val);
		vTaskDelay(250);
	}
}

// mpu ����
void mpu_task(void *pvParameters)
{
	while (1)
	{
		as = MPU6050_ReadSpeed();
		RTC_GetTime(RTC_Format_BIN, &Rtc_Time);
		if (tempSecond != timeStamp.second)
		{
			tempSecond = timeStamp.second;
			timeStamp.twentyMsCount = 0; // 20ms������������
		}
		else // �벻���£�1�����50*20ms
		{
			timeStamp.twentyMsCount++; // 20ms��������++
		}
		timeStamp.hour = Rtc_Time.RTC_Hours;
		timeStamp.minute = Rtc_Time.RTC_Minutes;
		timeStamp.second = Rtc_Time.RTC_Seconds;
		SportsInfo = *SensorChanged(&as, &timeStamp, &My_info);
		// printf("Steps: %d \r\n",SportsInfo.stepCount);
		// printf("%d %d %d\r\n",timeStamp.hour,timeStamp.minute,timeStamp.second);
		vTaskDelay(20);
	}
}

// ��������
void key_task(void *pvParameters)
{
	char key; // ������
	while (1)
	{
		key = KEY_Scan(0);
		if (key)
		{
			switch (key)
			{
			case WKUP_PRES:
				is_Alarm = 0; // �ص�����
				BEEP = 0;
				break;
			case KEY0_PRES:
				break;
			case KEY1_PRES:
				break;
			case KEY2_PRES:
				break;
			}
		}
		else
			vTaskDelay(10);
	}
}

// ��������
void alarm_task(void *pvParameters)
{
	while (1)
	{
		if (is_Alarm)
		{
			BEEP = !BEEP;
		}
		vTaskDelay(100);
	}
}
