#include "timer.h"
#include "led.h"
#include "usart.h"

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//��ʱ�� ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/4
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	 


//ͨ�ö�ʱ��3�жϳ�ʼ��
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz
//����ʹ�õ��Ƕ�ʱ��3!

extern uint8_t hour ;
extern uint8_t min ;
extern uint8_t sec ;
extern uint8_t num; //������1s�Ŵ��10s
extern char is_Alarm;
extern uint8_t hour_array[5];
extern uint8_t min_array[5];

void TIM3_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  ///ʹ��TIM3ʱ��
	
	TIM_TimeBaseInitStructure.TIM_Period = arr; 	//�Զ���װ��ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //��ʱ����Ƶ
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);//��ʼ��TIM3
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //����ʱ��3�����ж�
	TIM_Cmd(TIM3,ENABLE); //ʹ�ܶ�ʱ��3
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn; //��ʱ��3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x04; //��ռ���ȼ�4
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x00; //�����ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

//ͨ�ö�ʱ��5�жϳ�ʼ��
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz
//����ʹ�õ��Ƕ�ʱ��5!
void TIM5_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);	//ʹ��TIM5ʱ��
	
	TIM_TimeBaseInitStructure.TIM_Period = arr; 		//�Զ���װ��ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  		//��ʱ����Ƶ
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM5,&TIM_TimeBaseInitStructure);	//��ʼ��TIM5
	
	TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE); 			//����ʱ��5�����ж�
	TIM_Cmd(TIM5,ENABLE); 								//ʹ�ܶ�ʱ��5
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM5_IRQn; 		//��ʱ��5�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x05; 	//��ռ���ȼ�5
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x00; 		//�����ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

//��ʱ��3�жϷ�����
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //����ж�
	{
			sec = (sec + 1) % 60;
			if(sec == 0){
				min = (min + 1) % 60;
				if(min == 0){
					hour = (hour + 1) % 24;
				}
			}
			/*
			time_text[0] = hour / 10 + '0';
			time_text[1] = hour % 10 + '0';
			time_text[3] = min / 10 + '0';
			time_text[4] = min % 10 + '0';
			*/
			//printf("%c%c%c%c\r\n",time_text[0],time_text[1],time_text[3],time_text[4]);
	}
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //����жϱ�־λ
}


//��ʱ��5�жϷ�����
void TIM5_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM5,TIM_IT_Update)==SET) //����ж�
	{
		num =(num+1) % 60;
		if(num==0){ //����60s
			for(char i =0;i<5;i++){
				//printf("alarm%d: %d:%d %d\r\n",i,hour_array[i],min_array[i]/2,min_array[i]%2);
				if(hour==hour_array[i] && min==(min_array[i]/2) && min_array[i]%2==1){
					is_Alarm = 1;
					break;
				}
			}
		}
	}
	TIM_ClearITPendingBit(TIM5,TIM_IT_Update);  //����жϱ�־λ
}
