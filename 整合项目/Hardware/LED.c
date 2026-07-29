#include "stm32f10x.h"

void LED_Init(void)
{
	/*开启GPIOB时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);		//改为GPIOB
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;	//改为PB12和PB13
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);						//改为GPIOB
	
	/*默认高电平（LED灭）*/
	GPIO_SetBits(GPIOB, GPIO_Pin_12 | GPIO_Pin_13);				//改为GPIOB
}

/**
  * 函    数：LED1（绿灯）开启
  */
void LED1_ON(void)
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_12);		//PB12低电平亮（绿灯）
}

/**
  * 函    数：LED1（绿灯）关闭
  */
void LED1_OFF(void)
{
	GPIO_SetBits(GPIOB, GPIO_Pin_12);		//PB12高电平灭
}

/**
  * 函    数：LED1（绿灯）状态翻转
  */
void LED1_Turn(void)
{
	if (GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_12) == 0)
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_12);
	}
	else
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_12);
	}
}

/**
  * 函    数：LED2（红灯）开启
  */
void LED2_ON(void)
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_13);		//PB13低电平亮（红灯）
}

/**
  * 函    数：LED2（红灯）关闭
  */
void LED2_OFF(void)
{
	GPIO_SetBits(GPIOB, GPIO_Pin_13);		//PB13高电平灭
}

/**
  * 函    数：LED2（红灯）状态翻转
  */
void LED2_Turn(void)
{
	if (GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_13) == 0)
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_13);
	}
	else
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_13);
	}
}
