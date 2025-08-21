#include "stm32f4xx.h"                  // Device header
#include "DELAY.h"
//FLASH 读函数 写函数 擦函数
//注意：写入之前一定要擦
//写：解锁--擦除--编程（地址，数据）--上锁
//读：读取编程地址
//擦：解锁--擦除--上锁

void FLASH_write(uint32_t data)
{
 //1、解锁 清除标志
 FLASH_Unlock();
	
 FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 
	
 //2、擦除
	FLASH_EraseSector(FLASH_Sector_4,VoltageRange_3);//2.7V to 3.6V FLASH尽量要从最后一页写 
	
 //3、编程
	FLASH_ProgramWord(0X08010000,data);//地址而不扇区
	
 //4、上锁
  FLASH_Lock();
	DELAY_ms(100);
}

void FLASH_eraser(void)
{
 //1、解锁
 FLASH_Unlock();
	
 //2、擦除
	FLASH_EraseSector(FLASH_Sector_4,VoltageRange_3);//2.7V to 3.6V FLASH尽量要从最后一页写 
	
 //3、上锁
  FLASH_Lock();
}

uint32_t FLASH_read(uint32_t ADDRESSS)
{
   return *(__IO uint32_t*)ADDRESSS;
}
