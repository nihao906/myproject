#include "FSV9563.h"
#include "stm32f0xx_hal.h"
//#include "core_cmInstr.h"
#include "nfc.h"
#include <stdio.h>
#include <string.h>

static u8  fac_us=48;
static u16 fac_ms=48000;
void TestMode_FSV9563(void)
{
/**************************************************************************************/	      
//			FSV9563_WriteReg(0x66,0xC0);
	
//		  FSV9563_WriteReg(0x6C,0x00);
//      FSV9563_WriteReg(0x6A,0x00);
//      FSV9563_WriteReg(0x6D,0x15); //VLDO   AUX2
		
//			FSV9563_WriteReg(0x6C,0x11);
//			FSV9563_WriteReg(0x6A,0x00);
//			FSV9563_WriteReg(0x6D,0x00); //VREF   AUX1
		
//			FSV9563_WriteReg(0x6C,0x0A);
//			FSV9563_WriteReg(0x6A,0x00);
//			FSV9563_WriteReg(0x6D,0x00); //VBG    AUX1
//		
//			FSV9563_WriteReg(0x6C,0xC0);
//			FSV9563_WriteReg(0x6A,0x00);
//			FSV9563_WriteReg(0x6D,0xCA); //ITST    AUX2
		
//			FSV9563_WriteReg(0x6C,0xC0);
//			FSV9563_WriteReg(0x6A,0x00);
//			FSV9563_WriteReg(0x6D,0xD4); //IB13    AUX2

//			FSV9563_WriteReg(0x6C,0xC0);
//			FSV9563_WriteReg(0x6A,0x00);
//			FSV9563_WriteReg(0x6D,0xD3); //VREF_OK  AUX2

//			FSV9563_WriteReg(0x6C,0xC0);
//			FSV9563_WriteReg(0x6A,0x00);
//			FSV9563_WriteReg(0x6D,0xD0); //VPOR   AUX1

//			FSV9563_WriteReg(0x6C,0x00);
//			FSV9563_WriteReg(0x6A,0x00);
//			FSV9563_WriteReg(0x6D,0x12); //VTSTI   AUX2
	/*********************************************************************************/
			FSV9563_WriteReg(0x66,0xc0);
			FSV9563_WriteReg(0x6A,0x00); 

//			FSV9563_WriteReg(0x6C,0x01);    //Q路第二级IA P端输出 AUX2
//			FSV9563_WriteReg(0x6D,0x03);    //I路第二级IA P端输出 AUX1
			
	//		FSV9563_WriteReg(0x6C,0x05);		//Q路第一级IA P端输出 AUX2
	//		FSV9563_WriteReg(0x6D,0x07);    //I路第一级IA P端输出 AUX1
			
	//		FSV9563_WriteReg(0x6C,0x0B);		//Q路解调P端输出 AUX2
	//		FSV9563_WriteReg(0x6D,0x0D);    //I路解调P端输出 AUX1
			
			FSV9563_WriteReg(0x6C,0x02);    //Q路第二级IA N端输出 AUX2
			FSV9563_WriteReg(0x6D,0x04);    //I路第二级IA N端输出 AUX1
			
	//		FSV9563_WriteReg(0x6C,0x06);		//Q路第一级IA N端输出 AUX2
	//		FSV9563_WriteReg(0x6D,0x08);    //I路第一级IA N端输出 AUX1
			
	//		FSV9563_WriteReg(0x6C,0x0C);		//Q路解调N端输出 AUX2
	//		FSV9563_WriteReg(0x6D,0x0E);    //I路解调N端输出 AUX1
	
//			FSV9563_WriteReg(0x6C,0xC9);
//			FSV9563_WriteReg(0x6A,0x00);
//			FSV9563_WriteReg(0x6D,0xC9);    //DAC输出，根据数字测试要求

	
}
//////////////////////////////////////////////////////////////////////////////delay
void delay_ns(u32 ns)
{
  u32 i;
  for(i=0;i<ns;i++)
  {
    __NOP();
    __NOP();
    __NOP();
  }
}

void delay_init(u8 SYSCLK)		//unit:MHz
{
	SysTick->CTRL &= 0xfffffffb;//select internal clk: HCLK/8
	fac_us = SYSCLK/8;      
	fac_ms = (u16)fac_us*1000;
}            

void delay_us(u32 Nus)
{ 
	SysTick->LOAD=Nus*fac_us;       //load time      
	SysTick->CTRL|=0x01;            //start count   
	while(!(SysTick->CTRL&(1<<16)));//wait time out 
	SysTick->CTRL=0X00000000;       //close counter
	SysTick->VAL=0X00000000;        //clear counter    
}

void delay_ms(u16 nms)	//nms <= 0xffffff*8/SYSCLK; for 72M, Nms<=1864 
{    
	SysTick->LOAD=(u32)nms*fac_ms; 
	SysTick->CTRL|=0x01;               
	while(!(SysTick->CTRL&(1<<16)));  
	SysTick->CTRL&=0XFFFFFFFE;         
	SysTick->VAL=0X00000000;           
}
/*???????????
??????????????
?????????????????*/
void printStatus(const char *message,s8 status,u8 *pbuf,u8 len)
{
#ifdef UART_PRINT
	u8 i;
	printf("%s: %d_",message,status);
	if(status==MI_OK )// || status==MI_FRAMINGERR
	{
		for(i=0;i<len;i++)
			printf(" %02X",*(pbuf+i));
	}
	printf("\n");
#endif
}
/////////////////////////////////////////////////////////////////////
//void FSV9563_Init(void)
//{
//	EXTI_InitTypeDef   EXTI_InitStructure;
//	NVIC_InitTypeDef NVIC_InitStructure;
//	u8 temp;
//	
//	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource11);	 //IRQ

//	EXTI_InitStructure.EXTI_Line = EXTI_Line11;
//	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
//	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  	//falling edge of IRQ result interrupt
//	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//	EXTI_Init(&EXTI_InitStructure);

//	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);	
//	
//	PDOWN_0;	//	->RESET
//	//delay_ms(30);
//	temp = FSV9563_ReadReg(rRegVersion);
//#ifdef UART_PRINT
//	//printf("version: %X\n",temp);
//#endif
//}
/*
u8 ReadChipSel(void)
{
	u16 temp;
	ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 1, ADC_SampleTime_239Cycles5);	 	//PA7 
	temp = ReadADC1();
	if(temp<0x400)							//V
		return 3;
	else if(temp<0x700)					//F
		return 2;
	else if(temp<0xA00)					//B
		return 1;
	else 											
		return 0;									//A
}

u16 Get_Adc(void) 
{
//设置指定 ADC 的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 1, ADC_SampleTime_239Cycles5);
  ADC_SoftwareStartConvCmd(ADC1, ENABLE); //使能指定的 ADC1 的软件转换启动功能
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束
	return ADC_GetConversionValue(ADC1); //返回最近一次 ADC1 规则组的转换结果
}

u16 ReadADC1(void)
{
	u32 temp_val=0; u8 t;
	for(t=0;t<4;t++)
{
	temp_val+=Get_Adc();
	delay_ms(5);
}
return temp_val/4;
}*/

//////////////////////////////////////////////
u8 FSV9563_SPIWriteByte(u8 Byte)    //10Mbit/s
{
	while((SPI1->SR&0X02)==0);		 
	SPI1->DR=Byte;	 	            
	while((SPI1->SR&0X01)==0);  	 //SPI_SR_RXNE  //接收缓冲区
	return SPI1->DR;    
}

void FSV9563_WriteReg(u8   Address, u8   value)  //3.2微秒
{  
	FSV9563_NSS_0;
	//delay_us(5);
	FSV9563_SPIWriteByte(Address<<1);  
	FSV9563_SPIWriteByte(value);
	FSV9563_NSS_1;
	//delay_us(5);
} 

u8 FSV9563_ReadReg(u8   Address)
{
	u8  ucResult=0;
	FSV9563_NSS_0;
	//delay_us(5);
	FSV9563_SPIWriteByte((Address<<1)|0x01);
	ucResult = FSV9563_SPIWriteByte(0);
	FSV9563_NSS_1;
	//delay_us(5);
	return ucResult;
}

/*???????????????
???????????????
???????????????????*/
void FSV9563_SetBitMask(u8   reg,u8   mask)  
{
    u8  tmp = FSV9563_ReadReg(reg);
    FSV9563_WriteReg(reg,tmp | mask);  
}
/*??????????
???????????
??????????????*/
void FSV9563_ClearBitMask(u8   reg,u8   mask)  
{
    u8  tmp = FSV9563_ReadReg(reg);
    FSV9563_WriteReg(reg, tmp & ~mask);  
}

void FSV9563_SetRawRC(u8   reg,u8 mask,u8 set)
{
	u8 temp = FSV9563_ReadReg(reg);
	temp = (temp&mask)|set;
	FSV9563_WriteReg(reg,temp);
}
///////////////////////////////////////////////////////////
void FSV9563_FlushFifo()
{
	FSV9563_SetBitMask(rRegFIFOControl,0x10);
}
/*??????????
??????????
????????????*/
void FSV9563_FieldOn()
{
	FSV9563_SetBitMask(rRegDrvMod,0x08);
}
/*??????????
???????????
??????????????*/
void FSV9563_FieldOff()
{
	FSV9563_ClearBitMask(rRegDrvMod,0x08);
}

void FSV9563_FieldReset()
{
	FSV9563_FieldOff();
	delay_ms(20);
	FSV9563_FieldOn();
	delay_ms(20);
}
/*?????????????????
?????????????????
?????????????????*/
u8 Status_INT;
u8 mode;
s8 FSV9563_Command_Int(struct TranSciveBuffer *pi)
{
	u16 i;
	u8 j,n;
	u8 Status_INT;
	FSV9563_WriteReg(rRegCommand,FSV9563_Idle);
	FSV9563_SetBitMask(rRegFIFOControl,0x10);		//FlushFifo
	FSV9563_WriteReg(rRegIRQ0,0x7F);	
	FSV9563_WriteReg(rRegIRQ1,0x7F);
	
	for(n=0;n<pi->Length;n++)
		FSV9563_WriteReg(rRegFIFOData, pi->Data[n]);
	if(pi->Command&0x80)
	{
		FSV9563_WriteReg(rRegIRQ0En,0x10);		
		if(mode)
			FSV9563_WriteReg(rRegIRQ1En,0xE0);		
		else
			FSV9563_WriteReg(rRegIRQ1En,0xE8);		//T3定时器中断请求
		
		Status_INT=0;
		FSV9563_WriteReg(rRegCommand, pi->Command);
		while(Status_INT==0);				//wait for IRQ ,using LPCD
		Status_INT=0;
		
		FSV9563_WriteReg(rRegIRQ0En,0x10);		
		if(mode)
			FSV9563_WriteReg(rRegIRQ1En,0x20);		
		else
			FSV9563_WriteReg(rRegIRQ1En,0x28);		
	}
	else
		FSV9563_WriteReg(rRegCommand, pi->Command);
	
	for(i=2000;i>0;i--)
	{
		n = FSV9563_ReadReg(rRegIRQ0);	
		if(n&0x10) break;		//IDLEIRQ  等待命令自行终止
	}
	if(i==0)
		return MI_ERR;
	n = FSV9563_ReadReg(rRegFIFOLength);
	for(j=0;j<n;j++)
		pi->Data[j]= FSV9563_ReadReg(rRegFIFOData);
	return MI_OK;
}
/*????????????????
??????????????
??????????????*/
s8 FSV9563_CMD_LoadProtocol(u8 rx,u8 tx)
{
	struct TranSciveBuffer ComData;
	
	ComData.Command = FSV9563_LoadProtocol;
	ComData.Length = 2;
	ComData.Data[0] = rx;
	ComData.Data[1] = tx;

	s8 result = FSV9563_Command_Int(&ComData);
	return result;
}

s8 FSV9563_CMD_LoadKey(u8* pkey)
{
	struct TranSciveBuffer ComData;
	
	ComData.Command = FSV9563_LoadKey;
	ComData.Length = 6;
	memcpy(ComData.Data,pkey,6);

	return FSV9563_Command_Int(&ComData);
}

s8 FSV9563_CMD_MfcAuthenticate(u8 auth_mode,u8 block,u8 *pSnr)
{
	s8 status;
	u8 reg;
	struct TranSciveBuffer ComData;
	
	ComData.Command = FSV9563_MFAuthent;
	ComData.Length = 6;
	ComData.Data[0] = auth_mode;
	ComData.Data[1] = block;
	memcpy(&ComData.Data[2],pSnr,4);

	status= FSV9563_Command_Int(&ComData);
	if(status==MI_OK)
	{
		reg = FSV9563_ReadReg(rRegStatus);
		if(!(reg&0x20))
			status=MI_AUTHERR;
	}
	return status;
}
s8 FSV9563_PcdAuthState(u8 auth_mode,u8 block,u8 *pKey,u8 *pSnr)
{
  s8 status;
	u8 reg;
 	struct TranSciveBuffer ComData;

	ComData.Command  = FSV9563_MFAuthent;
  ComData.Length  = 12;
  ComData.Data[0] = auth_mode;
  ComData.Data[1] = block;
  memcpy(&ComData.Data[2], pKey, 6); 
  memcpy(&ComData.Data[8], pSnr, 4); 
  status = FSV9563_Command_Int(&ComData);
  if (status == MI_OK) 
	{  
		reg = FSV9563_ReadReg(rRegStatus);
		if(!(reg&0x08))
			status = MI_AUTHERR;
  }
  return status;
}
//s8 FSV9563_CMD_ReadE2(u16 addr,u8 len,u8 *pdat)
//{
//	struct TranSciveBuffer ComData;
//	s8 status;
//	
//	ComData.Command = FSV9563_ReadE2;
//	ComData.Length = 3;
//	ComData.Data[0] = addr>>8;
//	ComData.Data[1] = addr&0xff;
//	ComData.Data[2] = len;

//	status = FSV9563_Command_Int(&ComData);
//	if(status== MI_OK)
//		memcpy(pdat,ComData.Data,len);
//	return status;
//}

//s8 FSV9563_CMD_WriteE2(u16 addr,u8 dat)
//{
//	struct TranSciveBuffer ComData;
//	s8 status;
//	
//	ComData.Command = FSV9563_ReadE2;
//	ComData.Length = 3;
//	ComData.Data[0] = addr>>8;
//	ComData.Data[1] = addr&0xff;
//	ComData.Data[2] = dat;

//	status = FSV9563_Command_Int(&ComData);
//	return status;
//}
/*??????????????
??????????????????
???????????*/
s8 FSV9563_PcdConfigISOType(u8 type)
{
	volatile u8 value1;
	FSV9563_WriteReg(rRegT0Control,0x98); //Starts at the end of Tx. Stops after Rx of first data. Auto-reloaded. 13.56 MHz input clock.;
	value1 = FSV9563_ReadReg(rRegT0Control);
	
	if(value1 != 0x98)
		return value1;
	
	FSV9563_WriteReg(rRegT1Control,0x92); //Starts at the end of Tx. Stops after Rx of first data. Input clock - cascaded with Timer-0.
	FSV9563_WriteReg(rRegT2Control,0x20); //Timer used for LFO trimming
	
	volatile u8 value2;
	FSV9563_WriteReg(rRegT2ReloadHi,0x03);	//
	value2 = FSV9563_ReadReg(rRegT2ReloadHi);
	
	if(value2 != 0x03)
		return value2;
	
	FSV9563_WriteReg(rRegT2ReloadLo,0xFF);	//
	FSV9563_WriteReg(rRegT3Control,0x00);	//Not started automatically. Not reloaded. Input clock 13.56 MHz	
	if(type=='V')  //99.2
	{
		FSV9563_WriteReg(rRegWaterLevel,0x10); 	//Set WaterLevel =(FIFO length -1)
										 
		FSV9563_WriteReg(rRegRxBitCtrl,0x80);	//Received bit after collision are replaced with 1.
		FSV9563_WriteReg(rRegDrvMod,0x89);	//Tx2Inv=1 0x80   //0x89
		FSV9563_WriteReg(rRegTxAmp,0x04);	//0	//0x04  //0x10
		FSV9563_WriteReg(rRegDrvCon,0x09);	//0x09
		FSV9563_WriteReg(rRegTxl,0x0A);	//0x05
		FSV9563_WriteReg(rRegRxSofD,0x00);	//
		
		FSV9563_CMD_LoadProtocol(0x0a,0x0A);	//A/B/C,A/C

		FSV9563_WriteReg(rRegIRQ0En,0);
		FSV9563_WriteReg(rRegIRQ1En,0);
		
		
		volatile u8 value;
		FSV9563_WriteReg(rRegFIFOControl,0xB0);	
		value = FSV9563_ReadReg(rRegFIFOControl);  
		
		if(value != 0xB0)
			return value;
		
		FSV9563_WriteReg(rRegTxModWidth,0x00); // Length of the pulse modulation in carrier clks+1  
		FSV9563_WriteReg(rRegTxSym10BurstLen,0); // Symbol 1 and 0 burst lengths = 8 bits.

		FSV9563_WriteReg(rRegFrameCon,0x0F);

		FSV9563_WriteReg(rRegRxCtrl,0x02); // Set Rx Baudrate 26 kBaud 
		FSV9563_WriteReg(rRegRxThreshold,0x74); // Set min-levels for Rx and phase shift  0x4e  0x5C
		FSV9563_WriteReg(rRegRcv,0x12);
		FSV9563_WriteReg(rRegRxAna,0x07); //0xa 0x04
		
		FSV9563_WriteReg(rRegRxWait,0x9C);	//0x8C
		
		FSV9563_WriteReg(rRegDrvMod,0x81);   //0x81
		FSV9563_WriteReg(rRegStatus,0);
		FSV9563_WriteReg(rRegDrvMod,0x89);   //0x89
	}
	FSV9563_WriteReg(rRegTestMode,0xC0);
//	FSV9563_WriteReg(rRegTestInSel,0x66);	//rx
	FSV9563_WriteReg(rRegAUX1Sel,0x03);		//I(0x03),Q(0x01);
	FSV9563_WriteReg(rRegAUX2Sel,0x01);
//	FSV9563_WriteReg(rReg_6E,0x23);	//VIR:SCL; 3,4,6:SDA //0x23
//	FSV9563_WriteReg(rRegAUX1Sel,0x0A); //vref aux1
	FSV9563_WriteReg(rRegSigOut,0x09);	//RX-bit signal
	
	return MI_OK;
}
/*?????????????????????
?????????????????????????
???????????????????????*/
s8 FSV9563_PcdComTransceive(struct TranSciveBuffer *pi)
{
	s8 status= MI_ERR;
	u16 i;
	u8 reg1,temp,lastBits;	//reg0,
	u8 errReg;
	// Terminate any running command. 
	FSV9563_WriteReg(rRegCommand,FSV9563_Idle);
	FSV9563_SetBitMask(rRegFIFOControl,0x10);		//Flush_FiFo
	
	u8 value;
	value = FSV9563_ReadReg(rRegCommand);
	if(value != 0x10)
		return value;
	
	// Clear all IRQ 0,1 flags
	FSV9563_WriteReg(rRegIRQ0,0x7F);		
	FSV9563_WriteReg(rRegIRQ1,0x7F);

	for(i=0;i<pi->Length;i++)
		FSV9563_WriteReg(rRegFIFOData,pi->Data[i]);
	// Idle interrupt(Command terminated), FSV9563_BIT_IDLEIRQ=0x10
	FSV9563_WriteReg(rRegIRQ0En,0x18);	//IdleIRQEn,TxIRQEn
	FSV9563_WriteReg(rRegIRQ1En,0x42);	//Global IRQ,Timer1IRQEn
	//>  Start FSV9563 command "Transcieve"=0x07. Activate Rx after Tx finishes.
	//volatile u8 value;
	FSV9563_WriteReg(rRegCommand,pi->Command);	//FSV9563_Transceive
	//value = FSV9563_ReadReg(rRegCommand);
	
	do	
  {
      reg1 = FSV9563_ReadReg(rRegIRQ1);	  //07h //wait for TxIRQ
  }while((reg1&0x40)==0); //GlobalIRQ   //

	if(pi->Command == FSV9563_Transceive)
	{
		FSV9563_WriteReg(rRegIRQ0En,0x54);	//HiAlertIRQEN,IdleIRQEn,RxIRQEn
		FSV9563_WriteReg(rRegIRQ1En,0x42);	//Global IRQ,Timer1IRQEn
		for(i=8000;i>0;i--)            		
		{
			reg1 = FSV9563_ReadReg(rRegIRQ1);	  //07h  //wait for RxIRQ
			if(reg1&0x40) break;	//GlobalIRQ
		}
	}
	FSV9563_WriteReg(rRegIRQ0En,0);		
	FSV9563_WriteReg(rRegIRQ1En,0);

	errReg = FSV9563_ReadReg(rRegError);
	if(i==0)
	{
		status = MI_QUIT;
	}
	else if(reg1&0x02)		//Timer1IRQ
		status = MI_NOTAGERR;
	else if( errReg)	  //0Bh
	{
		if(errReg&0x04)
			status = MI_COLLERR;
		//读rxcoll 寄存器 看是在第几位发生碰撞
		else if(errReg&0x01)			//是错误的奇偶校验或错误 CRC，仍能接受数据
		{
			status = MI_FRAMINGERR;
			temp = FSV9563_ReadReg(rRegFIFOLength);		 //04h
			for (i=0; i<temp; i++)
					pi->Data[i] = FSV9563_ReadReg(rRegFIFOData);  //05h
			pi->Length = temp*8;
		}
		else
			status = MI_ERR;
	}
	else
	{
		status = MI_OK;
		if (pi->Command == FSV9563_Transceive)
		{
				temp = FSV9563_ReadReg(rRegFIFOLength);		 //04h
				lastBits = FSV9563_ReadReg(rRegRxBitCtrl) & 0x07;	 //0ch  
				if (lastBits)
					pi->Length = (temp-1)*8 + lastBits;
				else
					pi->Length = temp*8;
				if (temp == 0)	temp = 1;
				if (temp > 250) temp = 250; //maxlen ...
				for (i=0; i<temp; i++)
					pi->Data[i] = FSV9563_ReadReg(rRegFIFOData);  //05h
		}
	}
	return status;
}
///////////////////////////////////////////////////////////////////////////////////////
s8 FSV9563_PcdRequestA(u8 *pTagType)
{
	s8 status; 
	struct TranSciveBuffer ComData,*pi= &ComData;

	FSV9563_WriteReg(rRegTxCrcPreset,0x18);
	FSV9563_WriteReg(rRegRxCrcPreset,0x18);
	FSV9563_WriteReg(rRegFrameCon,0xCF);
	FSV9563_WriteReg(rRegStatus,0);
	
	FSV9563_WriteReg(rRegTXWaitCtrl,0xC0);	//  TxWaitStart at the end of Rx data
	FSV9563_WriteReg(rRegTxWaitLo,0x0B);	// Set min.time between Rx and Tx or between two Tx   

	FSV9563_WriteReg(rRegT0ReloadHi,0x08); 	//2196/fc
	FSV9563_WriteReg(rRegT0ReloadLo,0x94); 
	FSV9563_WriteReg(rRegT1ReloadHi,0); 
	FSV9563_WriteReg(rRegT1ReloadLo,0x40);	//timerout ~= 10ms
	
	FSV9563_WriteReg(rRegIRQ0,0x08);
	FSV9563_WriteReg(rRegRxWait,0x90);
	FSV9563_WriteReg(rRegTxDataNum,0x0F);  //7bit
	
	//> Send the ReqA command
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 1;
	ComData.Data[0] = PICC_REQIDL;
//	FLAG_1;
	status = FSV9563_PcdComTransceive(pi);
//	FLAG_0;
	if (status == MI_OK)
	{
		if(ComData.Length == 0x10)
		{
			*pTagType     = ComData.Data[0];
			*(pTagType+1) = ComData.Data[1];
		}
		else
			status = MI_VALERR;
	}
	//FSV9563_WriteReg(rRegTxDataNum,0x08);
	return status;
}

s8 FSV9563_PcdWakeUpA(u8 *pTagType) //41.6微秒+
{
	s8 status; 
	struct TranSciveBuffer ComData,*pi= &ComData;

	FSV9563_WriteReg(rRegTxCrcPreset,0x18);
	FSV9563_WriteReg(rRegRxCrcPreset,0x18);
	FSV9563_WriteReg(rRegFrameCon,0xCF);
	FSV9563_WriteReg(rRegStatus,0);
	
	FSV9563_WriteReg(rRegTXWaitCtrl,0xC0);	//  TxWaitStart at the end of Rx data
	FSV9563_WriteReg(rRegTxWaitLo,0x0B);	// Set min.time between Rx and Tx or between two Tx   

	FSV9563_WriteReg(rRegT0ReloadHi,0x08); 	//2196/fc
	FSV9563_WriteReg(rRegT0ReloadLo,0x94); 
	FSV9563_WriteReg(rRegT1ReloadHi,0); 
	FSV9563_WriteReg(rRegT1ReloadLo,0x40);	//timerout ~= 10ms
	
	FSV9563_WriteReg(rRegIRQ0,0x08);
	FSV9563_WriteReg(rRegRxWait,0x90);
	FSV9563_WriteReg(rRegTxDataNum,0x0F);  //7bit
	
	//> Send the ReqA command
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 1;
	ComData.Data[0] = PICC_REQALL;        //0x52
	
		FLAG_1;
	status = FSV9563_PcdComTransceive(pi);
		FLAG_0;
		
	if (status == MI_OK)
	{
		if(ComData.Length == 0x10)
		{
			*pTagType     = ComData.Data[0];
			*(pTagType+1) = ComData.Data[1];
		}
		else
			status = MI_VALERR;
	}
	//FSV9563_WriteReg(rRegTxDataNum,0x08);
	return status;
}

s8 FSV9563_PcdAnticollA(u8 level,u8 *pSnr)
{
	s8 status ;
	u8 i;
	u8 ucBits,ucBytes;
	u8 snr_check = 0;
	u8 ucCollPosition = 0;
	u8 ucTemp;
	u8 ucSNR[5] = {0, 0, 0, 0 ,0};
	struct TranSciveBuffer ComData,*pi = &ComData;
	
	FSV9563_ClearBitMask(rRegTxCrcPreset,0x01);	//TxCRCEn,off 
	FSV9563_ClearBitMask(rRegRxCrcPreset,0x01);	//TxCRCEn,off
	FSV9563_WriteReg(rRegTxDataNum,0x08);
	do
	{
		ucBits = (ucCollPosition) % 8;		
		if (ucBits != 0)
		{
			ucBytes = ucCollPosition / 8 + 1;
			FSV9563_SetRawRC(rRegRxBitCtrl, 0x8f,ucBits<<4);		//把冲撞位写入RxBitCtrl[6:4]位
			FSV9563_SetRawRC(rRegTxDataNum, 0xf8,ucBits);       //发送最后一个字节位的数目
		}
		else
			 ucBytes = ucCollPosition / 8;

		ComData.Command = FSV9563_Transceive;
		ComData.Data[0] = level;	//PICC_ANTICOLL1;//0x93
		ComData.Data[1] = 0x20 + ((ucCollPosition / 8) << 4) + (ucBits & 0x0F);
		for (i=0; i<ucBytes; i++)
			ComData.Data[i + 2] = ucSNR[i];
		ComData.Length = ucBytes + 2;
		status = FSV9563_PcdComTransceive(pi);

		ucTemp = ucSNR[(ucCollPosition / 8)];
		if (status == MI_COLLERR)
		{
			for (i=0; i < 5 - (ucCollPosition / 8); i++)
				 ucSNR[i + (ucCollPosition / 8)] = ComData.Data[i+1];
			ucSNR[(ucCollPosition / 8)] |= ucTemp;
			ucCollPosition = ComData.Data[0];
		}
		else if (status == MI_OK)
		{
			for (i=0; i < (ComData.Length / 8); i++)
				 ucSNR[4 - i] = ComData.Data[ComData.Length/8 - i - 1];
			ucSNR[(ucCollPosition / 8)] |= ucTemp;
		}
	} while (status == MI_COLLERR);
		
	if (status == MI_OK)
	{
		for (i=0; i<4; i++)
		{   
			*(pSnr+i)  = ucSNR[i];
			snr_check ^= ucSNR[i];
		}
		if (snr_check != ucSNR[i])
			status = MI_COM_ERR;
	}
	return status;
}

s8 FSV9563_PcdSelectA(u8 level,u8 *pSnr,u8 *pSize)
{
	s8   status;
	u8   i,snr_check = 0;
	struct TranSciveBuffer ComData,*pi = &ComData;

	FSV9563_SetRawRC(rRegTxCrcPreset,0xfe,0x01);	//TxCRCEn On
	FSV9563_SetRawRC(rRegRxCrcPreset,0xfe,0x01);	//RxCRCEn On
    
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 7;
	ComData.Data[0] = level;//PICC_ANTICOLL1;
	ComData.Data[1] = 0x70;
	for (i=0; i<4; i++)
	{
		snr_check ^= *(pSnr+i);
		ComData.Data[i+2] = *(pSnr+i);
	}
	ComData.Data[6] = snr_check;
	status = FSV9563_PcdComTransceive(pi);
	
	if (status == MI_OK)
	{
    if (ComData.Length != 0x8)
    	status = MI_BITCOUNTERR;
    else
    	*pSize = ComData.Data[0];
	}
	return status;
}
s8 FSV9563_PcdAuth(u8 *pSize)
{
	s8   status;
	struct TranSciveBuffer ComData,*pi = &ComData;

	FSV9563_SetRawRC(rRegTxCrcPreset,0xfe,0x01);	//TxCRCEn On
	FSV9563_SetRawRC(rRegRxCrcPreset,0xfe,0x01);	//RxCRCEn On
    
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 2;
	ComData.Data[0] = 0x0a;
	ComData.Data[1] = 0x00;

	status = FSV9563_PcdComTransceive(pi);
	
	if (status == MI_OK)
	{
    if (ComData.Length != 0x9)
    	status = MI_BITCOUNTERR;
    else
		{
//			for(i=0;i<ComData.Length;i++)
//			{
				*pSize = ComData.Data[0];
//			}
		}
	}
	return status;
}

s8 FSV9563_PcdMfcRead(u8   addr,u8 *pReaddata)
{
	s8 status;
	struct TranSciveBuffer ComData,*pi = &ComData;
	
	FSV9563_SetRawRC(rRegTxCrcPreset,0xfe,0x01);	//on
	FSV9563_SetRawRC(rRegRxCrcPreset,0xfe,0x00);	//off
	
  ComData.Command = FSV9563_Transceive;
	ComData.Length  = 2;
	ComData.Data[0] = PICC_READ;
	ComData.Data[1] = addr;
//	
//	FLAG_1;
	status = FSV9563_PcdComTransceive(pi);
//	FLAG_0;	
	
  if (status == MI_OK)
	{
		if (ComData.Length != 0x90)
			status = MI_BITCOUNTERR;
		else
			memcpy(pReaddata, &ComData.Data[0], 16);
	}
	return status;
}

s8 FSV9563_PcdMfcWrite(u8   addr,u8 *pWritedata)
{
	s8 status;
	struct TranSciveBuffer ComData,*pi = &ComData;

	FSV9563_SetRawRC(rRegTxCrcPreset,0xfe,0x01);	//on
	FSV9563_SetRawRC(rRegRxCrcPreset,0xfe,0x00);	//off
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 6;
	ComData.Data[0] = PICC_WRITE;
	ComData.Data[1] = addr;
	status = FSV9563_PcdComTransceive(pi);
	if (status != MI_NOTAGERR)
	{
		if(ComData.Length != 4)
			status=MI_BITCOUNTERR;
		else
		{
			ComData.Data[0] &= 0x0F;
			switch (ComData.Data[0])
			{
				case 0x00:
					status = MI_NOTAUTHERR;
					break;
				case 0x0A:
					status = MI_OK;
					break;
				default:
					status = MI_CODEERR;
					break;
			}
		}
	}    
	if (status == MI_OK)
	{
		ComData.Command = FSV9563_Transceive;
		ComData.Length  = 16;
		memcpy(&ComData.Data[0], pWritedata, 16);
		status = FSV9563_PcdComTransceive(pi);
		if (status != MI_NOTAGERR)
		{
			ComData.Data[0] &= 0x0F;
			switch(ComData.Data[0])
			{
				case 0x00:
					status = MI_WRITEERR;
					break;
				case 0x0A:
					status = MI_OK;
					break;
				default:
					status = MI_CODEERR;
					break;
			}
		}
	}
	return status;
}

s8 FSV9563_PcdMfulRead(u8   addr,u8 *pReaddata)
{
	s8 status;
	struct TranSciveBuffer ComData,*pi = &ComData;
	
	FSV9563_SetRawRC(rRegTxCrcPreset,0xfe,0x01);	//on
	FSV9563_SetRawRC(rRegRxCrcPreset,0xfe,0x01);	//on
	
  ComData.Command = FSV9563_Transceive;
	ComData.Length  = 2;
	ComData.Data[0] = PICC_READ;
	ComData.Data[1] = addr;
	status = FSV9563_PcdComTransceive(pi);
  if(status == MI_OK)
	{
		if (ComData.Length != 0x80)
			status = MI_BITCOUNTERR;
		else
			memcpy(pReaddata, &ComData.Data[0], 16);
	}
//	FSV9563_SetRawRC(rRegRxCrcPreset,0xfe,0x00); // clear crc
	return status;
}

s8 FSV9563_PcdHaltA(void)
{
	s8 status; 
	struct TranSciveBuffer ComData,*pi= &ComData;
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 2;
	ComData.Data[0] = PICC_HALT;
	ComData.Data[1] = 0;
	status = FSV9563_PcdComTransceive(pi);
	if(status == MI_NOTAGERR)		//halt command has no response
		status = MI_OK;
	else
		status = MI_ERR;
	return status;
}

////////////////////////////////////////T1T JEWEL
void UpdateCrc_B(u8 bCh, u16 *pLpwCrc)
{
    bCh = (bCh^(u8)((*pLpwCrc)&0x00FFU));
    bCh = (bCh ^ (bCh<<4U));
    *pLpwCrc = (*pLpwCrc >> 8U) ^ ((uint16_t)bCh << 8U) ^ ((u16)bCh << 3U) ^ ((u16)bCh>>4U);
}

void ComputeCrc_B( u8 *pData,u32 dwLength,u8 *pCrc)
{
    u8 bChBlock = 0;
    u16 wCrc = 0xFFFF;
    do
    {
        bChBlock = *pData++;
        UpdateCrc_B(bChBlock, &wCrc);
    } while (0u != (--dwLength));
    wCrc = ~wCrc;
    pCrc[0] = (u8) (wCrc & 0xFFU);
    pCrc[1] = (u8) ( (wCrc>>8U) & 0xFFU);
}
s8 FSV9563_PcdJewelCommand(u8* pdata,u8 len,u8* resp,u8* replen)
{
	s8 status;
	struct TranSciveBuffer ComData,*pi = &ComData;
	u8 i;
	u8 crc[2];
	
	FSV9563_WriteReg(rRegFrameCon,0x4F); //TxParityEn off,RxParityEn on//4F
	FSV9563_WriteReg(rRegTxDataNum,0x0F);  //7bit
//	FLAG_1;
	for(i=0;i<len-1;i++)
	{
		ComData.Command = FSV9563_Transmit;
		ComData.Length  = 1;
		ComData.Data[0] = *(pdata+i);	
		status = FSV9563_PcdComTransceive(pi);
		if(status != MI_OK)
			return status;
		if(i==0)
			FSV9563_WriteReg(rRegTxDataNum,0x08);
	}
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 1;
	ComData.Data[0] = *(pdata+len-1);	
	status = FSV9563_PcdComTransceive(pi);
//	FLAG_0;
	if(status == MI_OK)
	{
		*replen = ComData.Length/8;
		if (*replen != 0)
		{
			memcpy(resp, &ComData.Data[0], *replen);
			ComputeCrc_B(resp,*replen-2,crc);
			if(crc[0]!=resp[*replen-2] || crc[1]!=resp[*replen-1])
				status = MI_CRCERR;
		}
	}
	return status;
}

s8 FSV9563_PcdRidA(u8 *pUid)		
{
	s8   status;
	u8 cmd_rid[9]={0x78,0,0,0,0,0,0};	//{0x78,0,0,0,0,0,0,0xd0,0x43};	
	u8 resp[8];
	u8 len;

	ComputeCrc_B(cmd_rid,7,&cmd_rid[7]);
	status = FSV9563_PcdJewelCommand(cmd_rid,sizeof(cmd_rid),resp,&len);
	printStatus("RID",status,resp,len);
	if(status==MI_OK)
	{
		memcpy(pUid, &resp[2], 4);
	}
	return status;
} 

s8 FSV9563_PcdReadA(u8 *pUid)		
{
	s8   status;
	u8 cmdbuf[9]={1,8,0};
	u8 resp[8];
	u8 len;

	memcpy(&cmdbuf[3],pUid,4);
	ComputeCrc_B(cmdbuf,7,&cmdbuf[7]);
	status = FSV9563_PcdJewelCommand(cmdbuf,sizeof(cmdbuf),resp,&len);
	printStatus("Read",status,resp,len);

	return status;
} 

s8 FSV9563_PcdJewel(void)
{
	s8   status;
	u8 uid[4];
	status = FSV9563_PcdRidA(uid);
	if(status!=MI_OK) return status;
	status = FSV9563_PcdReadA(uid);
	return status;
}
s8 FSV9563_PcdActivateA(u8 *patqa,u8 *puid,u8 *plen,u8 *psak)
{
	s8 status;

	FSV9563_PcdConfigISOType('A');       //96微秒
	delay_ms(10);
	
	status = FSV9563_PcdWakeUpA(patqa);   //0.216
	
	
	printStatus("ATQA",status,patqa,2);
	if(status!=MI_OK) return status;

	if(*(patqa+1)==0x0C && *patqa==0 )	//T1T JEWEL TOPAZ512，没写此卡程序不用管
	{
		*plen = 6;
		status = FSV9563_PcdJewel();
		if(status!=MI_OK) return status;
	}
	else	//other
	{
		*plen = 4;
		status = FSV9563_PcdAnticollA(PICC_ANTICOLL1,puid);  //3.6
		printStatus("UID1",status,puid,4);
		if(status!=MI_OK) return status;

		status = FSV9563_PcdSelectA(PICC_ANTICOLL1,puid,psak);//1.13
		printStatus("SEL1",status,psak,1);
		if(status!=MI_OK) return status;
		
		if(*patqa&0xC0)	//level 2   判断bit7  bit8
		{
			*plen = 8;
			status = FSV9563_PcdAnticollA(PICC_ANTICOLL2,puid+4);  
			printStatus("UID2",status,puid+4,4);
			if(status!=MI_OK) return status;

			status = FSV9563_PcdSelectA(PICC_ANTICOLL2,puid+4,psak); 
			printStatus("SEL2",status,psak,1);
			if(status!=MI_OK) return status;
		}
		
		if(*patqa&0x80)	//level 3    判断bit7  bit8
		{
			*plen = 12;
			status = FSV9563_PcdAnticollA(PICC_ANTICOLL3,puid+8);
			printStatus("UID3",status,puid+8,4);
			if(status!=MI_OK) return status;

			status = FSV9563_PcdSelectA(PICC_ANTICOLL3,puid+8,psak);
			printStatus("SEL3",status,psak,1);
			if(status!=MI_OK) return status;
		}
	}
	return status;
}

u8 FSV9563_RatsA(u8 *pbuf,u8 *plen)
{
	s8   status;
	struct TranSciveBuffer ComData,*pi = &ComData;

	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 2;
	ComData.Data[0] = 0xE0;	
	ComData.Data[1] = 0x51;	//default=0x51	Fsdi,CID

	status = FSV9563_PcdComTransceive(pi);
	if (status == MI_OK)
	{
		*plen = ComData.Length/8;
    memcpy(pbuf, &ComData.Data[0], *plen);	//D81:06 75 77 81 02 80
	}
	return status;
}

u8 FSV9563_PpsA(u8 param,u8 *pPpss)
{
	s8   status;
	struct TranSciveBuffer ComData,*pi = &ComData;

	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 3;
	ComData.Data[0] = 0xD1;	//0xD0 | CID	
	ComData.Data[1] = 0x11;	//default=0x51	bFsdi,bCid
	ComData.Data[2] = param;	//0x0A(424,424);	0x0F(848,848)

	status = FSV9563_PcdComTransceive(pi);
	if (status == MI_OK)
	{
    if (ComData.Length != 8)
    	status = MI_BITCOUNTERR;
    else
    	*pPpss = ComData.Data[0];	//
	}
	return status;
}

void FSV9563_SetBaudrate(u8 txrate,u8 rrate)
{
	switch(txrate)
	{
		case 0:	//106k
			FSV9563_WriteReg(rRegTxModWidth,0x20); //2fh,
			break;
		case 1:	//212k
			FSV9563_WriteReg(rRegTxModWidth,0x10); //2fh,
			break;
		case 2:	//424k
			FSV9563_WriteReg(rRegTxModWidth,0x07); //2fh,
			break;
		case 3:	//848k
			FSV9563_WriteReg(rRegTxModWidth,0x02); //2fh,
			break;
		default://106k
			FSV9563_WriteReg(rRegTxModWidth,0x20); //2fh,
			break;
	}
	switch(rrate)
	{
		case 0:	//106k
			FSV9563_WriteReg(rRegRxCtrl,0x04); //35h,
			break;
		case 1:	//212k
			FSV9563_WriteReg(rRegRxCtrl,0x05); //35h,
			break;
		case 2:	//424k
			FSV9563_WriteReg(rRegRxCtrl,0x06); //35h,
			break;
		case 3:	//848k
			FSV9563_WriteReg(rRegRxCtrl,0x07); //35h,
			break;
		default://106k
			FSV9563_WriteReg(rRegRxCtrl,0x04); //35h,
			break;
	}
}

u8 FSV9563_CPU_I_Block(u8 *psbuf,u8 slen,u8 *prbuf,u8 *prlen)
{
	s8   status;
	struct TranSciveBuffer ComData,*pi = &ComData;

	FSV9563_WriteReg(rRegT0ReloadHi,0x41); 	//2196/fc 0x08
	FSV9563_WriteReg(rRegT0ReloadLo,0x0B); 	//0x94
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = slen+2;
	ComData.Data[0] = 0x0A;	//PCB	
	ComData.Data[1] = 0x01;	//Cid
	memcpy(&ComData.Data[2], psbuf, slen);
	status = FSV9563_PcdComTransceive(pi);
	if (status == MI_OK)
	{
		*prlen= ComData.Length/8;
		memcpy(prbuf, &ComData.Data[0], *prlen);
	}
	return status;
}

u8 FSV9563_NT3H2111Read(u8 page,u8 *prbuf,u8 *prlen)
{
	s8   status;
	struct TranSciveBuffer ComData,*pi = &ComData;
	FSV9563_WriteReg(rRegT0ReloadHi,0x41); 	//2196/fc 0x08;0x41
	FSV9563_WriteReg(rRegT0ReloadLo,0x94); 	//0x94;0x0B
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 2;
	ComData.Data[0] = 0x30;	//PCB	:0x0A;
	ComData.Data[1] = page;	//Cid
//	ComData.Data[2] = 0x09;	//Cid
//	memcpy(&ComData.Data[2], psbuf,slen);
//	FLAG_1;
	status = FSV9563_PcdComTransceive(pi);
//	FLAG_0;
	if (status == MI_OK)
	{
		*prlen= ComData.Length/8;
		memcpy(prbuf, &ComData.Data[0], *prlen);
	}
	return status;
}

s8 FSV9563_NT3H2111Write(u8   addr,u8 *pWritedata)
{
	s8 status;
	struct TranSciveBuffer ComData,*pi = &ComData;
	
	FSV9563_SetRawRC(rRegTxCrcPreset,0xfe,0x01);	//on
	FSV9563_SetRawRC(rRegRxCrcPreset,0xfe,0x00);	//off
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 6;  //2
	ComData.Data[0] = 0xA2;  //PICC_WRITE
	ComData.Data[1] = addr;
	memcpy(&ComData.Data[2], pWritedata, 4);
	status = FSV9563_PcdComTransceive(pi);
	if (status != MI_NOTAGERR)
	{
		if(ComData.Length != 4)
			status=MI_BITCOUNTERR;
		else
		{
			ComData.Data[0] &= 0x0F; //0X0F
			switch (ComData.Data[0])
			{
				case 0x00:
					status = MI_NOTAUTHERR;
					break;
				case 0x0A:
					status = MI_OK;
					break;
				default:
					status = MI_CODEERR;
					break;
			}
		}
	}
	FSV9563_SetRawRC(rRegRxCrcPreset,0xfe,0x01);	//on
	return status;
}
u8 FSV9563_NT3H2111Test(u8 *prbuf,u8 *prlen)
{
	s8   status;
	struct TranSciveBuffer ComData,*pi = &ComData;
	FSV9563_WriteReg(rRegT0ReloadHi,0x41); 	//2196/fc 0x08;0x41
	FSV9563_WriteReg(rRegT0ReloadLo,0x94); 	//0x94;0x0B
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 5;
	ComData.Data[0] = 0x1B;	
	memcpy(&ComData.Data[1], prbuf,4);
	status = FSV9563_PcdComTransceive(pi);
	if (status == MI_OK)
	{
		*prlen= ComData.Length/8;
		memcpy(prbuf, &ComData.Data[0], *prlen);
	}
	return status;

}
void FSV9563_NT3H2111(void)
{
			s8 status;
			u8 atqa[2];
			u8 uid[12],ulen;
			u8 sak;
		  u8 E8[16],E9[16],E5[16],E3[16],E4[16],E7[16];
			u8 CNT[4],CNT3[4];
	
			status = FSV9563_PcdActivateA(atqa,uid,&ulen,&sak);
			if(status!=MI_OK) return;
/**************************初始化TAG******************************/
			CNT[0] = 0x01;
			CNT[1] = 0x00;
			CNT[2] = 0x01;
			CNT[3] = 0xff;
			status = FSV9563_NT3H2111Write(0xE8,CNT); //配置寄存器
			printStatus("Write_e8",status,CNT,0);
			if(status!=MI_OK) return;		    
			
			CNT[0] = 0xff;
			CNT[1] = 0x01;
			CNT[2] = 0x00;
			CNT[3] = 0x00;
			status = FSV9563_NT3H2111Write(0xE9,CNT);//配置寄存器
			printStatus("Write_e9",status,CNT,0);
			if(status!=MI_OK) return;		    

			CNT[0] = 0x00;
			CNT[1] = 0x00;
			CNT[2] = 0x00;
			CNT[3] = 0x00;
			status = FSV9563_NT3H2111Write(0xE5,CNT);  //设置密钥
			printStatus("Write_e5",status,CNT,0);
			if(status!=MI_OK) return;	
			
			CNT3[0] = 0x00;
			CNT3[1] = 0x00;
			CNT3[2] = 0x00;
			CNT3[3] = 0x00;
			status = FSV9563_NT3H2111Test(CNT3,&ulen); //认证 commond 1B
			printStatus("Pass",status,CNT3,ulen);
			if(status!=MI_OK) return;

			CNT[0] = 0x00;
			CNT[1] = 0x00;
			CNT[2] = 0x00;
			CNT[3] = 0x01;
			status = FSV9563_NT3H2111Write(0xE3,CNT);
			printStatus("Write_e3",status,CNT,0);
			if(status!=MI_OK) return;				
			
			CNT[0] = 0x00;
			CNT[1] = 0x00;
			CNT[2] = 0x00;
			CNT[3] = 0x00;
			status = FSV9563_NT3H2111Write(0xE4,CNT);
			printStatus("Write_e4",status,CNT,0);
			if(status!=MI_OK) return;	
			
			CNT[0] = 0x04;
			CNT[1] = 0x00;
			CNT[2] = 0x00;
			CNT[3] = 0x00;
			status = FSV9563_NT3H2111Write(0xE7,CNT);
			printStatus("Write_e7",status,CNT,0);
			if(status!=MI_OK) return;	
	
	
			status = FSV9563_NT3H2111Read(0xE8,E8,&ulen);
			printStatus("Read_e8",status,E8,ulen);
			if(status!=MI_OK) return;

			status = FSV9563_NT3H2111Read(0xE9,E9,&ulen);
			printStatus("Read_e9",status,E9,ulen);
			if(status!=MI_OK) return;
			
			status = FSV9563_NT3H2111Read(0xE5,E5,&ulen);
			printStatus("Read_e5",status,E5,ulen);
			if(status!=MI_OK) return;			
			
			status = FSV9563_NT3H2111Read(0xE3,E3,&ulen);
			printStatus("Read_e3",status,E3,ulen);
			if(status!=MI_OK) return;		

			status = FSV9563_NT3H2111Read(0xE4,E4,&ulen);
			printStatus("Read_e4",status,E4,ulen);
			if(status!=MI_OK) return;

			status = FSV9563_NT3H2111Read(0xE7,E7,&ulen);
			printStatus("Read_e7",status,E7,ulen);
			if(status!=MI_OK) return;
			delay_ms(100);
				 
			if( E8[0] == 0x01 && E8[1] == 0x00 && E8[2] == 0x01 && E8[3] == 0xff 
					&& E9[0] == 0xff && E9[1] == 0x01 && E9[2] == 0x00 && E9[3] == 0x00
					&&E5[0] == 0x00 && E5[1] == 0x00&& E5[2] == 0x00 && E5[3] == 0x00 
					&& E3[0] == 0x00 && E3[1] == 0x00&& E3[2] == 0x00&& E3[3] == 0x01
				 && E4[0] == 0x00 && E4[1] == 0x00&& E4[2] == 0x00&& E4[3] == 0x00
				&& E7[0] == 0x04 && E7[1] == 0x00&& E7[2] == 0x00&& E7[3] == 0x00	
				)
				{
							LED_0;
					    delay_ms(500);
				    	LED_1;
				}
				else
				{

							LED_0;
					    delay_ms(100);

				}
/***********************************************************************************************/
}

void FSV9563_NT3H2111_Transfer(void)
{
		s8 status;
		u8 atqa[2];
		u8 uid[12],ulen;
		u8 sak,l;
	  u8 CNT1[16];
		status = FSV9563_PcdActivateA(atqa,uid,&ulen,&sak);
		if(status!=MI_OK) return;
	  delay_ms(500);
	
	 //判断nfc 来操作(发送数据)		
		 for(l = 0;l<10;l++)
		 {
			 status = FSV9563_NT3H2111Read(0xEC,CNT1,&ulen);
			 printStatus("Read_EC",status,CNT1,ulen);
			 if(status!=MI_OK) return;	
			 
			 if(CNT1[6] == 0x21)
			 {				 
				 break;					 
			 }		 
			delay_ms(50); 
		 }	 
		 if(l==10) return;
	

}

void FSV9563_TypeA(void)
{
	s8 status;
	u8 atqa[2];
	u8 uid[12],ulen;
	u8 sak;
	u8 RD_Data[16];	
	static u8 KEY[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	u8 ver1 = 0x60;
	u8 random[]={0x00,0x84,0x00,0x00,0x08};	//cpu_random	
	PDOWN_1;
	delay_ms(1);
	PDOWN_0;
	delay_ms(5);	
	status = FSV9563_PcdActivateA(atqa,uid,&ulen,&sak);   //4.95
	if(status!=MI_OK) return;
	if(ulen!=6)		//not topaz512
	{
		if(sak&0x04)
		{
			printf("uid not complete");
			return;
		}
		if(sak&0x20)	//compliant with ISO-14443-4	
		{
			status = FSV9563_RatsA(RD_Data,&ulen);
			printStatus("Rats",status,RD_Data,ulen);
			if(status!=MI_OK) return;
			
			if((RD_Data[1]&0x10)&&(RD_Data[2]&0x44)==0x44)	//TA(1) 848k supported	//mifare desfire
			{//5.9ms
				//848k(sure????)
				status = FSV9563_PpsA(0x00,&sak);//????0x0F
				printStatus("Pps",status,&sak,1);
				if(status!=MI_OK) return;
				FSV9563_CMD_LoadProtocol(0,0);
				FSV9563_SetBaudrate(0,0);	
			
				status = FSV9563_CPU_I_Block(&ver1,1,uid,&ulen);
				printStatus("ver1",status,uid,ulen);
				if(status!=MI_OK) return;
				
				//106k
				FSV9563_CMD_LoadProtocol(0,0);
				FSV9563_SetBaudrate(0,0);	
			}
			else
			{//6.9ms
				//mifare desfire (ISO/IEC 14443-4, default 106kHz)
				status = FSV9563_CPU_I_Block(random,5,uid,&ulen);  //基础实用
				printStatus("random",status,uid,ulen);
				if(status!=MI_OK) return;
			}
		}
		//MIFARE Classic
		else if(ulen==4)	//not compliant with ISO-14443-4	
		{//6.7ms
			if(sak==0x08 && atqa[0]==0x04)
			{
				printf("mifare S50 has detected!\n");
			}
			else if(sak==0x18 && atqa[0]==0x02)
			{
				printf("mifare S70 has detected!\n");
			}

			status = FSV9563_CMD_LoadKey(KEY);
			printStatus("LoadKey",status,NULL,0);
			if(status!=MI_OK) return;
			
			status = FSV9563_CMD_MfcAuthenticate(0x60,0,uid);
			printStatus("Auth",status,NULL,0);
			if(status!=MI_OK) return;
		
			status=FSV9563_PcdMfcRead(1,RD_Data);
			printStatus("MfcRead",status,RD_Data,16);
			if(status!=MI_OK) return;

		}
		else
		{
			//MIFARE Ultralight
			status = FSV9563_PcdMfulRead(3,RD_Data);
			printStatus("MfulRead",status,RD_Data,16);   //6.15
			if(status!=MI_OK) return;

		}
	}
	do
	{
		//射频场
		FSV9563_FieldOff();  //RESET
		delay_ms(10);
		FSV9563_FieldOn();   //Power the card
		LED_1;
		delay_ms(10);
		status = FSV9563_PcdRequestA(atqa);   //0.45
//	status = FSV9563_PcdWakeUpA(atqa);
		if(status==MI_OK)
		{
			LED_0;
		}
		else 
			break;
	}while(status==MI_OK);
}
//////////////////////////////////////////////////////ID2
s8 FSV9563_PcdRequestB(u8 *pTagType,u8 *pLen)
{
	s8 status; 
	struct TranSciveBuffer ComData,*pi= &ComData;

	FSV9563_WriteReg(rRegTxCrcPreset,0x7B); 
	FSV9563_WriteReg(rRegRxCrcPreset,0x7B); 
	FSV9563_WriteReg(rRegTxDataNum,0x08); 
	
	FSV9563_WriteReg(rRegTXWaitCtrl,0xC1);	
	FSV9563_WriteReg(rRegTxWaitLo,0x0B);	

	FSV9563_WriteReg(rRegT0ReloadHi,0x08); 	//2196/fc
	FSV9563_WriteReg(rRegT0ReloadLo,0x94); 
	FSV9563_WriteReg(rRegT1ReloadHi,0); 
	FSV9563_WriteReg(rRegT1ReloadLo,0x30);	//timerout ~= 10ms
	
	FSV9563_WriteReg(rRegIRQ0,0x08);
	FSV9563_WriteReg(rRegRxWait,0x90);
	//> Send the ReqB command
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 3;
	ComData.Data[0] = PICC_ANTI;
	ComData.Data[1] = 0;
	ComData.Data[2] = 0;
	FLAG_1;
	status = FSV9563_PcdComTransceive(pi);
	FLAG_0;
	if (status == MI_OK || status == MI_FRAMINGERR)
	{
//		if((ComData.Length == 0x60) || (ComData.Length == 0x10))
//		{
			*pLen = ComData.Length/8;
			memcpy(pTagType, &ComData.Data[0], *pLen);
//		}
//		else
//			status = MI_VALERR;
	}
	return status;
}

s8 FSV9563_PcdAttribB(u8 *pPupi,u8 *pCid)
{
	s8   status;
	struct TranSciveBuffer ComData,*pi = &ComData;

	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 9;
	ComData.Data[0] = PICC_ATTRIB;
//	memset(&ComData.Data[1], 0x00, 4);	//pupi
	memcpy(&ComData.Data[1], pPupi, 4);
	ComData.Data[5] = 0;	//
	ComData.Data[6] = 8;
	ComData.Data[7] = 1;
	ComData.Data[8] = 0;	//0 8
	status = FSV9563_PcdComTransceive(pi);
	if (status == MI_OK)
	{
    if (ComData.Length != 0x8)
    	status = MI_BITCOUNTERR;
    else
    	*pCid = ComData.Data[0];
	}
	return status;
}

s8 FSV9563_PcdGetUidB(u8 *pUID)
{
	s8   status;
	struct TranSciveBuffer ComData,*pi = &ComData;

	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 5;
	ComData.Data[0] = 0;
	ComData.Data[1] = 0x36;
	ComData.Data[2] = 0;
	ComData.Data[3] = 0;
	ComData.Data[4] = 8;
	
//	FLAG_1;
	status = FSV9563_PcdComTransceive(pi);
//	FLAG_0;
	if(status == MI_OK)
	{
		if(ComData.Length == 0x50)
			memcpy(pUID, &ComData.Data[0], 10);
		else
			status = MI_BITCOUNTERR;
	}
	return status;
}
//////////////////////////////////////////////////////
s8 FSV9563_SRT512_Initate(u8 *p)
{
	s8   status;  
	struct TranSciveBuffer ComData,*pi = &ComData; 

	ComData.Command = FSV9563_Transceive;
	ComData.Length  =2;
	ComData.Data[0] =0x6; 
	ComData.Data[1] =0;
	status  = FSV9563_PcdComTransceive(pi);
	if(status == MI_OK)
	{		
		if(ComData.Length == 8)	 
			*p = ComData.Data[0];
		else
			status = MI_BITCOUNTERR;
	}
   
	return status;
}

s8 FSV9563_SRT512_Select(u8 *p)
{
	s8   status;  
	struct TranSciveBuffer ComData,*pi = &ComData; 

	ComData.Command = FSV9563_Transceive;
	ComData.Length  =2;
	ComData.Data[0] =0x0E; 
	ComData.Data[1] =*p;
	status  = FSV9563_PcdComTransceive(pi);
	if(status == MI_OK)
	{		
		if(ComData.Length == 8)	 
			*p = ComData.Data[0];
		else
			status = MI_BITCOUNTERR;
	}
	return status;
}

s8 FSV9563_SRT512_GetUID(u8 *p)
{
	s8   status;  
	struct TranSciveBuffer ComData,*pi = &ComData; 

	ComData.Command = FSV9563_Transceive;
	ComData.Length  =1;
	ComData.Data[0] =0x0B; 
	status  = FSV9563_PcdComTransceive(pi);
	if(status == MI_OK)
	{
		if(ComData.Length == 0x40) 	
			memcpy(p, &ComData.Data[0], 8);
		else
			status = MI_BITCOUNTERR;
	}
   
	return status;
}

void FSV9563_TypeB(void)
{//4.36ms
	s8 status;
	u8 i;//,temp;
	u8 RD_Data[12],Len;	
	PDOWN_1;
	delay_ms(1);
	PDOWN_0;
	delay_ms(5);
	FSV9563_PcdConfigISOType('B');
	delay_ms(10);
	///////////////////////////////////2nd ID

	status = FSV9563_PcdRequestB(RD_Data,&Len);   //1.13ms
	//FLAG_0;
#ifdef UART_PRINT
	printf("ATQB: %d_",status);
	if(status==MI_OK)
	{
		for(i=0;i<Len;i++)
			printf(" %02X",RD_Data[i]);
	}
	printf("\n");
#endif
	if(status!=MI_OK) return;
	
	if(Len==12)
	{
		status = FSV9563_PcdAttribB(&RD_Data[1],RD_Data);  //1.2ms
#ifdef UART_PRINT
		printf("SELECT:%d_",status);
		if(status==MI_OK)
			printf(" %02X",RD_Data[0]);
		printf("\n");
#endif
		if(status!=MI_OK) return;
	}
	
	status = FSV9563_PcdGetUidB(RD_Data);   //0.9ms
#ifdef UART_PRINT
	printf("UID: %d_",status);
	if(status==MI_OK)
	{
		for(i=0;i<10;i++)
			printf(" %02X",RD_Data[i]);
	}
	printf("\n");
#endif

	while(1)
	{
		FSV9563_FieldOff();
		delay_ms(10);
		FSV9563_FieldOn();
		LED_1;
		delay_ms(10);
		status = FSV9563_PcdRequestB(RD_Data,&Len);  //1.13
		if(status==MI_OK)
		{
			LED_0;
		}
		else 
			break;
	}
}

//////////////////////////////////////////////////////Felica
s8 FSV9563_PcdRequestF(u8 *pIDm,u8 *pPMm)
{
	s8 status; 
	
	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegTxCrcPreset,0x09); 
	FSV9563_WriteReg(rRegRxCrcPreset,0x09); 
	FSV9563_WriteReg(rRegTxDataNum,0x08); 
	
	FSV9563_WriteReg(rRegTXWaitCtrl,0xC0);	//  TxWaitStart at the end of Rx data
	FSV9563_WriteReg(rRegTxWaitLo,0x00);	// Set min.time between Rx and Tx or between two Tx   

	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	//2196/fc 0x08
	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	//0x94
	FSV9563_WriteReg(rRegT1ReloadHi,0x03); 
	FSV9563_WriteReg(rRegT1ReloadLo,0x22);	//timerout ~= 10ms
	
	//> Send the ReqF command
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 6;
	ComData.Data[0] = 0x06;
	ComData.Data[1] = 0;
	ComData.Data[2] = 0xFF;
	ComData.Data[3] = 0xFF;
	ComData.Data[4] = 0;
	ComData.Data[5] = 0;
	status = FSV9563_PcdComTransceive(pi);
	if (status == MI_OK)
	{
		if(ComData.Length == 0x90)
		{

			memcpy(pIDm, &ComData.Data[2], 8);
			memcpy(pPMm, &ComData.Data[10], 8);
		}
		else
			status = MI_VALERR;
	}
	return status;
}

s8 FSV9563_PcdReadF(u8 *pIDm,u8 *pDat)
{
	s8 status; 
	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x41); 	//2196/fc 0x08
	FSV9563_WriteReg(rRegT0ReloadLo,0x0B); 	//0x94
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 16;
	ComData.Data[0] = 0x10;		//len
	ComData.Data[1] = 0x06;
	memcpy(&ComData.Data[2],pIDm, 8);
	ComData.Data[10] = 1;			//bNumServices
	ComData.Data[11] = 0x0B;	//pServiceList..
	ComData.Data[12] = 0;
	ComData.Data[13] = 1;			//bTxNumBlocks
	ComData.Data[14] = 0x80;	//pBlockList..
	ComData.Data[15] = 0x01;
	status = FSV9563_PcdComTransceive(pi);
	if (status == MI_OK)
	{
		if(ComData.Length == 0xE8)		//
		{
			//0x1d,0x07,IDm(8),0,0,1,data(16)
			//memcpy(pIDm, &ComData.Data[2], 8);
			memcpy(pDat, &ComData.Data[13], 16);
		}
		else
			status = MI_VALERR;
	}
	return status;
}
s8 FSV9563_PcdWriteF(u8 *pIDm,u8 *pDat)
{
	s8 status; 
	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	//2196/fc 0x08
	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	//0x94
	FSV9563_WriteReg(rRegT1ReloadHi,0x02); 	//2196/fc 0x08
	FSV9563_WriteReg(rRegT1ReloadLo,0xA1); 	//0x94
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 32;
	ComData.Data[0] = 0x20;		//len
	ComData.Data[1] = 0x08;
	memcpy(&ComData.Data[2],pIDm, 8);
	ComData.Data[10] = 1;			//bNumServices
	ComData.Data[11] = 0x09;	//pServiceList..
	ComData.Data[12] = 0;
	ComData.Data[13] = 1;			//bTxNumBlocks
	ComData.Data[14] = 0x80;	//pBlockList..
	ComData.Data[15] = 0x01;
	memcpy(&ComData.Data[16],pDat, 16);
	status = FSV9563_PcdComTransceive(pi);
	if(status == MI_OK)
	{
		if(ComData.Length != 0x60)		//12
		{
			//0x0C,0x09,IDm(8),0,0
			status = MI_VALERR;
		}
	}
	return status;
}
void FSV9563_Felica(void)
{ 	//1.95ms
	s8 status;
	u8 IDm[8],PMm[8],buf[16];	
	PDOWN_1;
	delay_ms(1);
	PDOWN_0;
	delay_ms(5);
	FSV9563_PcdConfigISOType('F');
	delay_ms(10);		
	///////////////////////////////////
	status = FSV9563_PcdRequestF(IDm,PMm);  //0.52ms
	printStatus("ATQF",status,IDm,8);
	if(status!=MI_OK) return;
	
	status = FSV9563_PcdReadF(IDm,buf);     //0.91ms
	printStatus("READ",status,buf,16);
	if(status!=MI_OK) return;
	
	/*delay_ms(100);
	buf[15]++;
	status = FSV9563_PcdWriteF(IDm,buf);
	printStatus("READ",status,NULL,0);
	if(status!=MI_OK) return;
	*/
	do
	{
		FSV9563_FieldOff();
		delay_ms(10);
		FSV9563_FieldOn();
		LED_1;
		delay_ms(10);
		status = FSV9563_PcdRequestF(IDm,PMm);   //0.52ms
		if(status==MI_OK)
		{
			LED_0;
		}
		else 
			break;
	}while(status==MI_OK);
}
//////////////////////////////////////////////////////ISO15693
/*????????????
????????????
?????????????*/
s8 FSV9563_PcdInventoryV(u8 *pDsfId,u8 *pUID)
{
	s8 status; 
	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegTxCrcPreset,0x7B); 
	FSV9563_WriteReg(rRegRxCrcPreset,0x7B); 
	FSV9563_WriteReg(rRegTxDataNum,0x08); 
	
	FSV9563_WriteReg(rRegTXWaitCtrl,0x88);	//  TxWaitStart at the end of Rx data 0xC0
	FSV9563_WriteReg(rRegTxWaitLo,0xA9);	// Set min.time between Rx and Tx or between two Tx   0
	// Set timeout for this command cmd. Init reload values for timers-0,1 
	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	//2196/fc 0x0080
	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	//0x94
	FSV9563_WriteReg(rRegT1ReloadHi,0x03); 	//0x0232
	FSV9563_WriteReg(rRegT1ReloadLo,0x22);	//timerout ~= 10ms

	//> Send the ReqF command
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 3;
	ComData.Data[0] = 0x26;	//flags:NBSLOTS,no AFI
	ComData.Data[1] = 0x01;	//invetory
	//ComData.Data[2] = 0;	//afi
	ComData.Data[2] = 0;	//masklen
//	FLAG_1;
	status = FSV9563_PcdComTransceive(pi);
//	FLAG_0;
	if (status == MI_OK)
	{
		if(ComData.Length == 0x50)
		{
			*pDsfId = ComData.Data[1];
			memcpy(pUID, &ComData.Data[2], 8);
		}
		else
			status = MI_VALERR;
	}
	return status;
}
/*??????????????
????????????????
??????????????*/
s8 FSV9563_SelectV(u8 *pUid)
{
	s8 status; 
	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x24); 	
	FSV9563_WriteReg(rRegT0ReloadLo,0xEB); 	
	FSV9563_WriteReg(rRegT1ReloadHi,0x00); 	
	FSV9563_WriteReg(rRegT1ReloadLo,0x00);	
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 10;
	ComData.Data[0] = 0x22;	//bflag
	ComData.Data[1] = 0x25;	//PHPAL_SLI15693_SW_CMD_SELECT
	memcpy(&ComData.Data[2],pUid, 8);
	status = FSV9563_PcdComTransceive(pi);	//~5.5ms
	if (status == MI_OK)
	{
		if(ComData.Length ==0 || ComData.Data[0]!=0)
			status = MI_VALERR;
	}
	return status;
}

s8 FSV9563_readV(u8 *pUid)
{
	s8 status; 
	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x24); 	
	FSV9563_WriteReg(rRegT0ReloadLo,0xEB); 	
	FSV9563_WriteReg(rRegT1ReloadHi,0x00); 	
	FSV9563_WriteReg(rRegT1ReloadLo,0x00);	
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 10;
	ComData.Data[0] = 0x22;	//bflag
	ComData.Data[1] = 0x26;	//PHPAL_SLI15693_SW_CMD_SELECT
	memcpy(&ComData.Data[2],pUid, 8);
	status = FSV9563_PcdComTransceive(pi);	//~5.5ms
	if (status == MI_OK)
	{
		if(ComData.Length ==0 || ComData.Data[0]!=0)
			status = MI_VALERR;
	}
	return status;
}

s8 FSV9563_StayQuiet(u8 *pUid)
{
	s8 status; 
	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x24); 	
	FSV9563_WriteReg(rRegT0ReloadLo,0xEB); 	
	FSV9563_WriteReg(rRegT1ReloadHi,0x00); 	
	FSV9563_WriteReg(rRegT1ReloadLo,0x00);	
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 10;
	ComData.Data[0] = 0x22;	//bflag
	ComData.Data[1] = 0x02;	//StayQuiet
	memcpy(&ComData.Data[2],pUid, 8);
//	FLAG_1;
	status = FSV9563_PcdComTransceive(pi);
//	FLAG_0;
	if (status == MI_OK)
	{	
		if(ComData.Length !=0)	//should no response
			status = MI_VALERR;
	}
	else if(status == MI_NOTAGERR)	//timeout
	{
		status = MI_OK;
	}
	return status;
}
/*????????????
????????????
??????????????*/
s8 FSV9563_ReadSingleBlockV(const u8 *pUid,u8 block,u8 *pDat,u8 *pLen)
{
	s8 status; 

	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x24); 	//2196/fc 0x0080
	FSV9563_WriteReg(rRegT0ReloadLo,0xEB); 	//0x94
	FSV9563_WriteReg(rRegT1ReloadHi,0x00); 	//0x0232
	FSV9563_WriteReg(rRegT1ReloadLo,0x00);	//timerout ~= 10ms
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 3;
	ComData.Data[0] = 0x12;	//bflag
	ComData.Data[1] = 0x20;	//PHAL_ICODE_CMD_READ_SINGLE_BLOCK
//	for(k =0;k < 8;k++)
//	{
//		ComData.Data[2+k] = pUid[k];
//	}
	ComData.Data[2] = block;
	
//  FLAG_1;
	status = FSV9563_PcdComTransceive(pi);
//	FLAG_0;
	
	if (status == MI_OK)
	{
		if(ComData.Length !=0)//== 0x50)
		{
			*pLen = ComData.Length/8-1;
			memcpy(pDat, &ComData.Data[1], *pLen);
		}
		else
			status = MI_VALERR;
	}
	return status;
}

s8 FSV9563_WriteSingleBlockV(u8 block,u8 *pDat,u8 Len)
{
	s8 status; 
   Len = 4;
	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	
	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	
	FSV9563_WriteReg(rRegT1ReloadHi,0x08); 	
	FSV9563_WriteReg(rRegT1ReloadLo,0x6E);	//timerout ~= 20ms
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = Len+3;  //Len+3
	ComData.Data[0] = 0x12;	//bflag
	ComData.Data[1] = 0x21;	//PHAL_ICODE_CMD_WRITE_SINGLE_BLOCK
	ComData.Data[2] = block;
	
	
	memcpy(&ComData.Data[3],pDat,Len);

	
	status = FSV9563_PcdComTransceive(pi);
	if (status == MI_OK)
	{
		if(ComData.Length ==0 || ComData.Data[0]!=0)
			status = MI_VALERR;
	}
	return status;
}

s8 FSV9563_GetRandomNumber(const u8 *pUid,u8 *pRandom,u8 *pLen)
{
	s8 status; 
  u8 k;
	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	
	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	
	FSV9563_WriteReg(rRegT1ReloadHi,0x08); 	
	FSV9563_WriteReg(rRegT1ReloadLo,0x6E);	//timerout ~= 20ms
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 11;
	ComData.Data[0] = 0x22;	//bflag
	ComData.Data[1] = 0xb2;	//PHAL_ICODE_CMD_WRITE_SINGLE_BLOCK
	ComData.Data[2] = 0x04;
	for(k =0;k < 8;k++)
	{
		ComData.Data[3+k] = pUid[k];
	}
	
	status = FSV9563_PcdComTransceive(pi);
	
//	
//	if (status == MI_OK)
//	{
//		if(ComData.Length ==0 || ComData.Data[0]!=0)
//			status = MI_VALERR;
//	}
//	return status;
	if (status == MI_OK)
	{
		if(ComData.Length !=0)//== 0x50)
		{
			*pLen = ComData.Length/8 ;
			memcpy(pRandom, &ComData.Data[0], *pLen);
		}
		else
			status = MI_VALERR;
	}
  return status;
}

 s8 FSV9563_SetPassWard(const u8 *pUid,u8 *pPwd,u8 *pRandom,u8 *pLen)
{
	s8 status; 

	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	
	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	
	FSV9563_WriteReg(rRegT1ReloadHi,0x08); 	
	FSV9563_WriteReg(rRegT1ReloadLo,0x6E);	//timerout ~= 20ms
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  =8;
	ComData.Data[0] = 0x12;	//bflag
	ComData.Data[1] = 0xb3;	//PHAL_ICODE_CMD_WRITE_SINGLE_BLOCK
	ComData.Data[2] = 0x04;
//	for(k =0;k < 8;k++)
//	{
//		ComData.Data[3+k] = pUid[k];
//	}
	
	ComData.Data[3] = 0x01;
	ComData.Data[4] = pPwd[0] ^ pRandom[0];
	ComData.Data[5] = pPwd[1] ^ pRandom[1];
	ComData.Data[6] = pPwd[2] ^ pRandom[0];
	ComData.Data[7] = pPwd[3] ^ pRandom[1];
	
	status = FSV9563_PcdComTransceive(pi);
	if (status == MI_OK)
	{
		if(ComData.Length !=8 || ComData.Data[0]!=0)
		status = MI_VALERR;
	}

		if (status == MI_OK)
		{
			if(ComData.Length !=0)//== 0x50)
			{
				*pLen = ComData.Length/8;
				memcpy(pRandom, &ComData.Data[0], *pLen);
			}
			else
				status = MI_VALERR;
		}
	return status;
}

s8 FSV9563_WritePassWard(const u8 *pUid,u8 *Ppassword, u8 *pDat,u8 *pLen)
{
	s8 status; 

	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	
	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	
	FSV9563_WriteReg(rRegT1ReloadHi,0x08); 	
	FSV9563_WriteReg(rRegT1ReloadLo,0x6E);	//timerout ~= 20ms
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  =8;
	ComData.Data[0] = 0x12;	//bflag
	ComData.Data[1] = 0xb4;	//PHAL_ICODE_CMD_WRITE_SINGLE_BLOCK
	ComData.Data[2] = 0x04;
//		for(i = 0;i < 8; i++)
//	{
//		ComData.Data[3+i] = pUid[i];
//	}
	
	ComData.Data[3] = 0x01;
	
	ComData.Data[4] = Ppassword[0];
	ComData.Data[5] = Ppassword[1];
	ComData.Data[6] = Ppassword[2];
	ComData.Data[7] = Ppassword[3];
	
	
	status = FSV9563_PcdComTransceive(pi);
	
		if (status == MI_OK)
		{
			if(ComData.Length !=0)//== 0x50)
			{
				*pLen = ComData.Length/8;
				memcpy(pDat, &ComData.Data[0], *pLen);
			}
			else
				status = MI_VALERR;
		}
	return status;
//	if (status == MI_OK)
//	{
//		if(ComData.Length ==0 || ComData.Data[0]!=0)
//			status = MI_VALERR;
//	}
//	return status;
}


//s8 FSV9563_PassWardProtect(const u8 *pUid)
//{
//	s8 status,i; 

//	struct TranSciveBuffer ComData,*pi= &ComData;
//	
//	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	
//	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	
//	FSV9563_WriteReg(rRegT1ReloadHi,0x08); 	
//	FSV9563_WriteReg(rRegT1ReloadLo,0x6E);	//timerout ~= 20ms
//	
//	ComData.Command = FSV9563_Transceive;
//	ComData.Length  =11;
//	ComData.Data[0] = 0x12;	//bflag
//	ComData.Data[1] = 0xa6;	//PHAL_ICODE_CMD_WRITE_SINGLE_BLOCK
//	ComData.Data[2] = 0x04;
//		for(i = 0;i < 8; i++)
//	{
//		ComData.Data[3+i] = pUid[i];
//	}
//	
//	status = FSV9563_PcdComTransceive(pi);
//	if (status == MI_OK)
//	{
//		if(ComData.Length ==0 || ComData.Data[0]!=0)
//			status = MI_VALERR;
//	}
//	return status;
//}

s8 FSV9563_ProtectPage(const u8 *pUid,u8 Page,u8 ProtectionStatus,u8 *pDat,u8 *pLen)
{
	s8 status; 

	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	
	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	
	FSV9563_WriteReg(rRegT1ReloadHi,0x08); 	
	FSV9563_WriteReg(rRegT1ReloadLo,0x6E);	//timerout ~= 20ms
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  =5;
	ComData.Data[0] = 0x12;	//bflag
	ComData.Data[1] = 0xc6;	//PHAL_ICODE_CMD_WRITE_SINGLE_BLOCK
	ComData.Data[2] = 0x04;
//		for(k = 0;k < 8; k++)
//	{
//		ComData.Data[3+k] = pUid[k];
//	}
	ComData.Data[3] = Page;
	ComData.Data[4] = ProtectionStatus;
	
	status = FSV9563_PcdComTransceive(pi);
//	if (status == MI_OK)
//	{
//		if(ComData.Length ==0 || ComData.Data[0]!=0)
//			status = MI_VALERR;
//	}
	if (status == MI_OK)
		{
			if(ComData.Length !=0)//== 0x50)
			{
				*pLen = ComData.Length/8;
				memcpy(pDat, &ComData.Data[0], *pLen);
			}
			else
				status = MI_VALERR;
		}
	return status;
}

s8 FSV9563_GetProtectionStatus(const u8 *pUid,u8 *pDat,u8 *pLen)
{
	s8 status; 

	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	
	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	
	FSV9563_WriteReg(rRegT1ReloadHi,0x08); 	
	FSV9563_WriteReg(rRegT1ReloadLo,0x6E);	//timerout ~= 20ms
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  =3;
	ComData.Data[0] = 0x12;	//bflag
	ComData.Data[1] = 0xc8;	//PHAL_ICODE_CMD_WRITE_SINGLE_BLOCK
	ComData.Data[2] = 0x04;
//	for(k = 0;k < 8; k++)
//	{
//		ComData.Data[3+k] = pUid[k];
//	}
//	status = FSV9563_PcdComTransceive(pi);
//	if (status == MI_OK)
//	{
//		if(ComData.Length ==0 || ComData.Data[0]!=0)
//			status = MI_VALERR;
//	}
  status = FSV9563_PcdComTransceive(pi);
	
	if (status == MI_OK)
	{
		if(ComData.Length !=0)//== 0x50)
		{
			*pLen = ComData.Length/8;
			memcpy(pDat, &ComData.Data[0], *pLen);
		}
		else
			status = MI_VALERR;
	}
	return status;
}

s8 FSV9563_LockPageProtectionCondition(const u8 *pUid,u8 *pDat,u8 *pLen)
{
	s8 status; 

	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	
	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	
	FSV9563_WriteReg(rRegT1ReloadHi,0x08); 	
	FSV9563_WriteReg(rRegT1ReloadLo,0x6E);	//timerout ~= 20ms
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  =3;
	ComData.Data[0] = 0x12;	//bflag
	ComData.Data[1] = 0xc7;	//PHAL_ICODE_CMD_WRITE_SINGLE_BLOCK
	ComData.Data[2] = 0x04;
//		for(k = 0;k < 8; k++)
//	{
//		ComData.Data[3+k] = pUid[k];
//	}

	status = FSV9563_PcdComTransceive(pi);
//	if (status == MI_OK)
//	{
//		if(ComData.Length ==0 || ComData.Data[0]!=0)
//			status = MI_VALERR;
//	}
	if (status == MI_OK)
		{
			if(ComData.Length !=0)//== 0x50)
			{
				*pLen = ComData.Length/8;
				memcpy(pDat, &ComData.Data[0], *pLen);
			}
			else
				status = MI_VALERR;
		}
	return status;
}

s8 FSV9563_SetEAS(const u8 *pUid,u8 *pDat,u8 *pLen)
{
	s8 status; 

	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	
	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	
	FSV9563_WriteReg(rRegT1ReloadHi,0x08); 	
	FSV9563_WriteReg(rRegT1ReloadLo,0x6E);	//timerout ~= 20ms
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  =3;
	ComData.Data[0] = 0x12;	//bflag
	ComData.Data[1] = 0xa2;	//PHAL_ICODE_CMD_WRITE_SINGLE_BLOCK
	ComData.Data[2] = 0x04;
//		for(k = 0;k < 8; k++)
//	{
//		ComData.Data[3+k] = pUid[k];
//	}
	
	status = FSV9563_PcdComTransceive(pi);

	if (status == MI_OK)
		{
			if(ComData.Length !=0)//== 0x50)
			{
				*pLen = ComData.Length/8;
				memcpy(pDat, &ComData.Data[0], *pLen);
			}
			else
				status = MI_VALERR;
		}
	return status;
}

s8 FSV9563_ResetEAS(const u8 *pUid,u8 *pDat,u8 *pLen)
{
	s8 status; 

	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	
	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	
	FSV9563_WriteReg(rRegT1ReloadHi,0x08); 	
	FSV9563_WriteReg(rRegT1ReloadLo,0x6E);	//timerout ~= 20ms
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  =3;
	ComData.Data[0] = 0x12;	//bflag
	ComData.Data[1] = 0xa3;	//PHAL_ICODE_CMD_WRITE_SINGLE_BLOCK
	ComData.Data[2] = 0x04;
//		for(k = 0;k < 8; k++)
//	{
//		ComData.Data[3+k] = pUid[k];
//	}
	
	status = FSV9563_PcdComTransceive(pi);

	if (status == MI_OK)
		{
			if(ComData.Length !=0)//== 0x50)
			{
				*pLen = ComData.Length/8;
				memcpy(pDat, &ComData.Data[0], *pLen);
			}
			else
				status = MI_VALERR;
		}
	return status;
}

s8 FSV9563_EasAlarm(const u8 *pUid,u8 *pEasSequence,u8 *pLen)
{
	s8 status; 

	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	
	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	
	FSV9563_WriteReg(rRegT1ReloadHi,0x08); 	
	FSV9563_WriteReg(rRegT1ReloadLo,0x6E);	//timerout ~= 20ms
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  =3;
	ComData.Data[0] = 0x12;	//bflag
	ComData.Data[1] = 0xa5;	//PHAL_ICODE_CMD_WRITE_SINGLE_BLOCK
	ComData.Data[2] = 0x04;
//		for(k = 0;k < 8; k++)
//	{
//		ComData.Data[3+k] = pUid[k];
//	}
	
	status = FSV9563_PcdComTransceive(pi);

	if (status == MI_OK)
		{
			if(ComData.Length !=0)//== 0x50)
			{
				*pLen = ComData.Length/8;
				memcpy(pEasSequence, &ComData.Data[0], *pLen);
			}
			else
				status = MI_VALERR;
		}
	return status;
}

s8 FSV9563_FastInventoryRead(const u8 *pUid,u8 MaskLength,u8 MaskValue,u8 FirstBlockNumber,u8 NumberOfBlock,u8 *pDat,u8 *pLen)
{
	s8 status; 
	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegTxCrcPreset,0x7B); 
	FSV9563_WriteReg(rRegRxCrcPreset,0x7B); 
	FSV9563_WriteReg(rRegTxDataNum,0x08); 
	
	FSV9563_WriteReg(rRegTXWaitCtrl,0x88);	//  TxWaitStart at the end of Rx data 0xC0
	FSV9563_WriteReg(rRegTxWaitLo,0xA9);	// Set min.time between Rx and Tx or between two Tx   0
	// Set timeout for this command cmd. Init reload values for timers-0,1 
	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	//2196/fc 0x0080
	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	//0x94
	FSV9563_WriteReg(rRegT1ReloadHi,0x03); 	//0x0232
	FSV9563_WriteReg(rRegT1ReloadLo,0x22);

	
	
	FSV9563_WriteReg(rRegRxCtrl,0x03);
	FSV9563_CMD_LoadProtocol(0x0b,0x0A);
//	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	
//	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	
//	FSV9563_WriteReg(rRegT1ReloadHi,0x08); 	
//	FSV9563_WriteReg(rRegT1ReloadLo,0x6E);	//timerout ~= 20ms
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 7;
	ComData.Data[0] = 0x66;	//bflag
	ComData.Data[1] = 0xa1;	//PHAL_ICODE_CMD_WRITE_SINGLE_BLOCK
	ComData.Data[2] = 0x04;
	ComData.Data[3] = MaskLength;
	ComData.Data[4] = MaskValue;
	ComData.Data[5] = FirstBlockNumber;
	ComData.Data[6] = NumberOfBlock;
	
	
//		for(k = 0;k < 8; k++)
//	{
//		ComData.Data[3+k] = pUid[k];
//	}
	FLAG_1;
	status = FSV9563_PcdComTransceive(pi);
  FLAG_0;
	
	if (status == MI_OK)
		{
			if(ComData.Length !=0)//== 0x50)
			{
				*pLen = ComData.Length/8;
				memcpy(pDat, &ComData.Data[0], *pLen);
			}
			else
				status = MI_VALERR;
		}
		
		FSV9563_WriteReg(rRegRxCtrl,0x02);
		FSV9563_CMD_LoadProtocol(0x0a,0x0A);
	return status;
}

s8 FSV9563_InventoryRead(const u8 *pUid,u8 MaskLength,u8 MaskValue,u8 FirstBlockNumber,u8 NumberOfBlock,u8 *pDat,u8 *pLen)
{
	s8 status; 
	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegTxCrcPreset,0x7B); 
	FSV9563_WriteReg(rRegRxCrcPreset,0x7B); 
	FSV9563_WriteReg(rRegTxDataNum,0x08); 
	
	FSV9563_WriteReg(rRegTXWaitCtrl,0x88);	//  TxWaitStart at the end of Rx data 0xC0
	FSV9563_WriteReg(rRegTxWaitLo,0xA9);	// Set min.time between Rx and Tx or between two Tx   0
	// Set timeout for this command cmd. Init reload values for timers-0,1 
	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	//2196/fc 0x0080
	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	//0x94
	FSV9563_WriteReg(rRegT1ReloadHi,0x03); 	//0x0232
	FSV9563_WriteReg(rRegT1ReloadLo,0x22);

	
	
//	FSV9563_WriteReg(rRegRxCtrl,0x03);
//	FSV9563_CMD_LoadProtocol(0x0b,0x0A);
//	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	
//	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	
//	FSV9563_WriteReg(rRegT1ReloadHi,0x08); 	
//	FSV9563_WriteReg(rRegT1ReloadLo,0x6E);	//timerout ~= 20ms
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 7;
	ComData.Data[0] = 0x66;	//bflag  16bit
	ComData.Data[1] = 0xa0;	//Inventory Read  commond code  16bit
	ComData.Data[2] = 0x04; // IC Mfg code  16bit
	ComData.Data[3] = MaskLength; // 0x08   16bit
	ComData.Data[4] = MaskValue;  // 0x12   16bit
	ComData.Data[5] = FirstBlockNumber;  // 0x05  16bit
	ComData.Data[6] = NumberOfBlock;    //  0x02  16bit  
	
	
//		for(k = 0;k < 8; k++)
//	{
//		ComData.Data[3+k] = pUid[k];
//	}
	FLAG_1;
	status = FSV9563_PcdComTransceive(pi);
  FLAG_0;
	
	if (status == MI_OK)
		{
			if(ComData.Length !=0)//== 0x50)
			{
				*pLen = ComData.Length/8;
				memcpy(pDat, &ComData.Data[0], *pLen);
			}
			else
				status = MI_VALERR;
		}
		
//		FSV9563_WriteReg(rRegRxCtrl,0x02);
//		FSV9563_CMD_LoadProtocol(0x0a,0x0A);
	return status;
}
/*???????????????????
?????????????????
?????????????????????*/
s8 FSV9563_WriteTriggerMode(const u8 *pUid,u8 Mode ,u8 *pDat,u8 *pLen)
{
	s8 status; 

	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	
	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	
	FSV9563_WriteReg(rRegT1ReloadHi,0x08); 	
	FSV9563_WriteReg(rRegT1ReloadLo,0x6E);	//timerout ~= 20ms
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  =4;
	ComData.Data[0] = 0x12;	//bflag
	ComData.Data[1] = 0xc5;	//PHAL_ICODE_CMD_WRITE_SINGLE_BLOCK
	ComData.Data[2] = 0x04;
	ComData.Data[3] = Mode;
//		for(k = 0;k < 8; k++)
//	{
//		ComData.Data[3+k] = pUid[k];
//	}
	
	status = FSV9563_PcdComTransceive(pi);

	if (status == MI_OK)
		{
			if(ComData.Length !=0)//== 0x50)
			{
				*pLen = ComData.Length/8;
				memcpy(pDat, &ComData.Data[0], *pLen);
			}
			else
				status = MI_VALERR;
		}
	return status;
}

s8 FSV9563_Unlock(void)
{
	u8 dat[4],len ;
	s8 status; 
	
//	 status = FSV9563_ReadSingleBlockV(0x4a,dat,&len);
//   printStatus("ReadBlock_4B",status,dat,len);
//  	if(status!=MI_OK) return 2;
	
	 dat[0] = 0x35;		//open
	 dat[1] = 0xcb;
	 dat[2] = 0x00;
	 dat[3] = 0x00;
	
	 delay_ms(10);
	
	 status = FSV9563_WriteSingleBlockV(0x4A,dat,len);
	 printStatus("Write_4A",status,NULL,0);
	 if(status!=MI_OK) return 1;
	 
//	for(i=0;i<3;i++)
//	{
//	delay_ms(100);
//	 status = FSV9563_ReadSingleBlockV(0x52,dat,&len);
//	 printStatus("ReadBlock_52",status,dat,len);
//	 if(status!=MI_OK) return 2;
//	 if(dat[3]==0x00)
//		 return 0;
//	}

    return 0;
}
//s8 FSV9563_EnableLock(void)
//{
//	u8 dat[4],i,len;
//	s8 status;
//	
//	 dat[0] = 0x73;		//close 
//	 dat[1] = 0x8d;
//	 dat[2] = 0x00;
//	 dat[3] = 0x00;
//	 status = FSV9563_WriteSingleBlockV(0x4A,dat,len);
//	 printStatus("Write2",status,NULL,0);
//	 if(status!=MI_OK) return 1;
//		 
//	for(i=0;i<3;i++)
//	{
//	delay_ms(100);
//	 status = FSV9563_ReadSingleBlockV(0x4A,dat,&len);
//	 printStatus("ReadBlock2",status,dat,len);
//	 if(status!=MI_OK) return 2;
//	 if(dat[3]==0x00)
//		 return 0;
//	}
//	return 3;
//}

//s8 FSV9563_GetLockStatus(void)
//{
//	u8 dat[4],len;
//	s8 status;
//	
//	dat[0] = 0xa3;		//get status
//	dat[1] = 0x5d;
//	dat[2] = 0x00;
//	dat[3] = 0x00;
//	status = FSV9563_WriteSingleBlockV(0x4A,dat,len);
//	printStatus("Write_4A",status,NULL,0);
//	if(status!=MI_OK) return 1;
//	
//	status = FSV9563_WriteSingleBlockV(0x4F,dat,len);
//	printStatus("Write_4F",status,NULL,0);
//	if(status!=MI_OK) return 2;
//	
//	delay_ms(200);
//	
//	status = FSV9563_ReadSingleBlockV(uid,0x4B,dat,&len);
//	printStatus("ReadBlock_4B",status,dat,len);
//	if(status!=MI_OK) return 3;
//	
//	status = FSV9563_ReadSingleBlockV(uid,0x4F,dat,&len);
//	printStatus("ReadBlock_4F",status,dat,len);
//	if(status!=MI_OK) return 4;
	
	
//	for(i=0;i<3;i++)
//	{
//		delay_ms(150);
//		status = FSV9563_ReadSingleBlockV(0x4B,dat,&len);
//		printStatus("ReadBlock2",status,dat,len);
//		if(status!=MI_OK) return 2;
//		if(dat[0]==0x53&&dat[1]==0xad)		//open status
//		{
//			*pflag=0;
//			return 0;
//		}
//		if(dat[0]==0x63&&dat[1]==0x9d)		//lock status
//		{
//			*pflag=1;
//			return 0;
//		}
//	}
//    return 0;
//}
void combine_uid_and_timestamp(u8 *uid, u32 timestamp, u8 *output) {
    for (int i = 0; i < 4; i++) {
        output[i] = uid[4 + i];
    }
    output[4] = (timestamp >> 24) & 0xFF; 
    output[5] = (timestamp >> 16) & 0xFF;
    output[6] = (timestamp >> 8) & 0xFF;
    output[7] = timestamp & 0xFF;  
}
#include "stm32f0xx_hal_can.h"
#include "can.h"
#include "Write_DATA.h"
void Sent_Message(u8 *output)
{
	CAN_TxHeaderTypeDef TxHeader;
	MX_CAN_Init();
	uint32_t TxMailbox;

	TxHeader.StdId = config.canId;
	TxHeader.ExtId = 0x01;     		
	TxHeader.IDE   = CAN_ID_STD;  
	TxHeader.RTR   = CAN_RTR_DATA; 
	TxHeader.DLC   = 8;  

	if (HAL_CAN_AddTxMessage(&hcan, &TxHeader, output, &TxMailbox) != HAL_OK) {
		Error_Handler();
	}
}

void FSV9563_ISO15693(void)
 {   
	s8 status;
	u8 DsfId,uid[8]={0},output[8];
	u8 dat[16],len;
	u32 timestamp;
	//u8 flag;
//	FSV9563_FieldOff();
//	delay_ms(10);
//	FSV9563_FieldOn();
	PDOWN_1;  //PA1
	__NOP();	
	PDOWN_0;		
	__NOP();
	LED_0;
	FSV9563_PcdConfigISOType('V');

	uid[0] = 0x12;
	LED_1;
	//delay_ms(10);		
	///////////////////////////////////Type V/ISO15693
	status = FSV9563_PcdInventoryV(&DsfId,uid);              //4.15ms
	//printStatus("Inventory",status,uid,8);
	if(status == MI_OK)
	{
		LED_0;
		timestamp = HAL_GetTick();
		combine_uid_and_timestamp(uid, timestamp, output);
		Sent_Message(output);
	}
	if(status == MI_VALERR)
	{
		LED_1;
	}
	
	status = FSV9563_SelectV(uid);							 //5.5ms
	//printStatus("Select",status,NULL,0);
	if(status!=MI_OK) return;
	
	status = FSV9563_ReadSingleBlockV(uid,05,dat,&len);		  //5.8ms
	//printStatus("ReadBlock",status,dat,len);
	if(status!=MI_OK) return;
	
	do
	{
		FSV9563_FieldOff();
		//delay_ms(10);
		FSV9563_FieldOn();
		LED_1;
		//delay_ms(10);
		status = FSV9563_PcdInventoryV(&DsfId,uid);   //0.52ms
		if(status==MI_OK)
			LED_0;
		else 
			break;
	}while(status==MI_OK);
}


