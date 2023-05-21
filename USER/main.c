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

// 需要更新的内容
// 时钟页面UI优化，加入电量显示
// 闹钟页面UI优化，给闹钟设置选项加一个底色

// 任务优先级
#define START_TASK_PRIO 1
// 任务堆栈大小
#define START_STK_SIZE 128
// 任务句柄
TaskHandle_t StartTask_Handler;
// 任务函数
void start_task(void *pvParameters);

// 任务优先级
#define LED0_TASK_PRIO 3
// 任务堆栈大小
#define LED0_STK_SIZE 50
// 任务句柄
TaskHandle_t LED0Task_Handler;
// 任务函数
void led0_task(void *pvParameters);

// 任务优先级
#define LED1_TASK_PRIO 4
// 任务堆栈大小
#define LED1_STK_SIZE 50
// 任务句柄
TaskHandle_t LED1Task_Handler;
// 任务函数
void led1_task(void *pvParameters);

// 任务优先级
#define LVGL_TASK_PRIO 2
// 任务堆栈大小
#define LVGL_STK_SIZE 1024
// 任务句柄
TaskHandle_t LVGLTask_Handler;
// 任务函数
void lvgl_task(void *pvParameters);

// 任务优先级
#define ADC_TASK_PRIO 5
// 任务堆栈大小
#define ADC_STK_SIZE 128
// 任务句柄
TaskHandle_t ADCTask_Handler;
// 任务函数
void adc_task(void *pvParameters);

// 任务优先级
#define MPU_TASK_PRIO 6
// 任务堆栈大小
#define MPU_STK_SIZE 256
// 任务句柄
TaskHandle_t MPUTask_Handler;
// 任务函数
void mpu_task(void *pvParameters);

// 任务优先级
#define KEY_TASK_PRIO 7
// 任务堆栈大小
#define KEY_STK_SIZE 50
// 任务句柄
TaskHandle_t KEYTask_Handler;
// 任务函数
void key_task(void *pvParameters);

// 任务优先级
#define ALARM_TASK_PRIO 8
// 任务堆栈大小
#define ALARM_STK_SIZE 50
// 任务句柄
TaskHandle_t ALARMTask_Handler;
// 任务函数
void alarm_task(void *pvParameters);

// 全局变量
// 时间
uint8_t hour = 14;
uint8_t min = 35;
uint8_t sec = 0;
uint16_t year = 2022;
uint8_t month = 11;
uint8_t day = 29;

// adc
u8 light_val = 0;
u16 temp_val = 0;

// 闹钟
uint8_t num = 0; // 闹钟扫描周期
char is_Alarm = 0;

// 运动相关
personInfo_t My_info = {170 / 100, 110 / 2}; // 身体的身高、体重
sportsInfo_t SportsInfo;					 // 装步数、卡路里、路程的结构体变量
ADD_SPEED as;								 // 存放加速度
RTC_TimeTypeDef Rtc_Time;					 // 获取RTC时间
static u8 tempSecond;						 // 保存秒钟暂态量
static timeStamp_t timeStamp = {0};			 // 存时分秒和20毫秒内的计数

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); // 设置系统中断优先级分组4
	delay_init(168);								// 初始化延时函数
	uart_init(115200);								// 初始化串口波特率为115200

	LED_Init();								  // 初始化LED
	My_RTC_Init();							  // 初始化RTC
	RTC_Set_Time(hour, min, sec, RTC_H12_PM); // 设置时间
	RTC_Set_Date(22, 11, 29, 2);			  // 设置日期
	MPU_Init();
	LCD_Init();	 // LCD初始化
	KEY_Init();	 // 按键初始化
	BEEP_Init(); // 蜂鸣器初始化

	TIM3_Int_Init(9999, 8399); // 初始化1s定时器TIM3，1s 1中断
	TIM5_Int_Init(9999, 8399); // 10s 定时器

	FSMC_SRAM_Init(); // 初始化外部sram
	tp_dev.init();	  // 触摸屏初始化
	Adc_Init();		  // ADC初始化
	Lsens_Init();	  // 初始化光敏传感器

	while (mpu_dmp_init()) // 初始化dmp
	{
		LCD_ShowString(30, 130, 200, 16, 16, "MPU6050 Error");
		delay_ms(200);
		LCD_Fill(30, 130, 239, 130 + 16, WHITE);
		delay_ms(200);
	}

	lv_init();			  // lvgl系统初始化
	lv_port_disp_init();  // lvgl显示接口初始化,放在lv_init()的后面
	lv_port_indev_init(); // lvgl输入接口初始化,放在lv_init()的后面

	// BEEP = 1; //开机成功
	// delay_ms(50);
	// BEEP = 0;

	gui_app_start(); // 运行例程

	// 创建开始任务
	xTaskCreate((TaskFunction_t)start_task,			 // 任务函数
				(const char *)"start_task",			 // 任务名称
				(uint16_t)START_STK_SIZE,			 // 任务堆栈大小
				(void *)NULL,						 // 传递给任务函数的参数
				(UBaseType_t)START_TASK_PRIO,		 // 任务优先级
				(TaskHandle_t *)&StartTask_Handler); // 任务句柄
	vTaskStartScheduler();							 // 开启任务调度
}

// 开始任务任务函数
void start_task(void *pvParameters)
{
	taskENTER_CRITICAL(); // 进入临界区
	// 创建LED0任务
	xTaskCreate((TaskFunction_t)led0_task,
				(const char *)"led0_task",
				(uint16_t)LED0_STK_SIZE,
				(void *)NULL,
				(UBaseType_t)LED0_TASK_PRIO,
				(TaskHandle_t *)&LED0Task_Handler);

	// 创建LED1任务
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

	vTaskDelete(StartTask_Handler); // 删除开始任务
	taskEXIT_CRITICAL();			// 退出临界区
}

// LED0任务函数 在这里计算时间
void led0_task(void *pvParameters)
{
	while (1)
	{
		LED0 = ~LED0;

		vTaskDelay(500);
	}
}

// LED1任务函数
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

// lvgl任务
void lvgl_task(void *pvParameters)
{
	while (1)
	{
		tp_dev.scan(0);
		lv_task_handler();
		vTaskDelay(10);
	}
}

// adc任务
void adc_task(void *pvParameters)
{
	while (1)
	{
		temp_val = Get_Temprate();	 // 默认为正数
		light_val = Lsens_Get_Val(); // 默认为正数
		// printf("Temp:%d.%d;Adcx:%d\r\n",(temp_val/100)%100,temp_val%100,light_val);
		vTaskDelay(250);
	}
}

// mpu 任务
void mpu_task(void *pvParameters)
{
	while (1)
	{
		as = MPU6050_ReadSpeed();
		RTC_GetTime(RTC_Format_BIN, &Rtc_Time);
		if (tempSecond != timeStamp.second)
		{
			tempSecond = timeStamp.second;
			timeStamp.twentyMsCount = 0; // 20ms计数变量清零
		}
		else // 秒不更新，1秒等于50*20ms
		{
			timeStamp.twentyMsCount++; // 20ms计数变量++
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

// 键盘任务
void key_task(void *pvParameters)
{
	char key; // 读键盘
	while (1)
	{
		key = KEY_Scan(0);
		if (key)
		{
			switch (key)
			{
			case WKUP_PRES:
				is_Alarm = 0; // 关掉闹钟
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

// 闹钟任务
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
