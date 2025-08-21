#include "stm32f4xx.h"                  // Device header
uint8_t RTC_flag = 0;
void RTC_setdatetime(void)
{
	if(RTC_ReadBackupRegister(RTC_BKP_DR0) != 0X32F2)
	{
	 RTC_DateTypeDef RTC_DateStructure;
   RTC_DateStructure.RTC_Year = 0x25;
   RTC_DateStructure.RTC_Month = RTC_Month_July;
   RTC_DateStructure.RTC_Date = 0x25;
   RTC_DateStructure.RTC_WeekDay = RTC_Weekday_Friday;
   
	 RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);
   
   RTC_TimeTypeDef  RTC_TimeStructure;
   RTC_TimeStructure.RTC_H12     = RTC_H12_PM;
   RTC_TimeStructure.RTC_Hours   = 0x05;
   RTC_TimeStructure.RTC_Minutes = 0x37;
   RTC_TimeStructure.RTC_Seconds = 0x00; 
   
	 RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure);  
	
	 RTC_WriteBackupRegister(RTC_BKP_DR0, 0x32F2);
	}
}

void RTC_init(void)
{
  //1、使能PWM时钟
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
 
	//2、允许RTC写入
	PWR_BackupAccessCmd(ENABLE);
	
	//3、使能LSE时钟
	RCC_LSEConfig(RCC_LSE_ON);
	
	//4、等待LSE准备完成
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
	{
	
	}
	
	//5、选择LSE作为RTC时钟
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
	//新加开启RTC时钟
	RCC_RTCCLKCmd(ENABLE);
	//6、等待APB寄存器同步
	RTC_WaitForSynchro();
	
	//7、RTC配置
	RTC_InitTypeDef  RTC_InitStructure;
	RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
	RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
	RTC_InitStructure.RTC_SynchPrediv = 0xFF;
 
	//8、RTC初始化
	RTC_Init(&RTC_InitStructure);
	
	//9、设置日期和时间
	RTC_setdatetime();
	
	//10、关闭RTC的WEAKUP定时
	RTC_WakeUpCmd(DISABLE);
	
	//11、设置RTC的时钟源
	RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);
	
	//12、设置RTC唤醒计数值
	RTC_SetWakeUpCounter(0);
	
	//13、清除RTC唤醒状态位，标志状态位
	RTC_ClearFlag(RTC_FLAG_WUTF);
	RTC_ClearITPendingBit(RTC_IT_WUT);
	EXTI_ClearITPendingBit(EXTI_Line22);
	
	//14、开启RTC唤醒中断
	RTC_ITConfig(RTC_IT_WUT,ENABLE);
	
	//15、使能RTC中断定时
	RTC_WakeUpCmd(ENABLE);
	
	//16、将EXTI与RTC连接起来
	EXTI_InitTypeDef  EXTI_INSTRUCT;
	EXTI_INSTRUCT.EXTI_Line = EXTI_Line22;
	EXTI_INSTRUCT.EXTI_LineCmd = ENABLE;
	EXTI_INSTRUCT.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_INSTRUCT.EXTI_Trigger = EXTI_Trigger_Rising;
	
	//17、EXTI初始化
	EXTI_Init(&EXTI_INSTRUCT);
	
	//18、NVIC配置
	NVIC_InitTypeDef NVIC_INSTRUCT;
	NVIC_INSTRUCT.NVIC_IRQChannel = RTC_WKUP_IRQn;
	NVIC_INSTRUCT.NVIC_IRQChannelCmd = ENABLE;
	NVIC_INSTRUCT.NVIC_IRQChannelPreemptionPriority = 0X02;//抢占优先级 0-3
	NVIC_INSTRUCT.NVIC_IRQChannelSubPriority =  0X03;//响应优先级 0-3
	
	//19、NVIC初始化
	NVIC_Init(&NVIC_INSTRUCT);
}

//20、唤醒的中断服务函数
void RTC_WKUP_IRQHandler(void)
{
 if(RTC_GetITStatus(RTC_IT_WUT) == SET)
 {
	 RTC_flag = 1;
   EXTI_ClearITPendingBit(EXTI_Line22);
	 RTC_ClearITPendingBit(RTC_IT_WUT);
 }
}
