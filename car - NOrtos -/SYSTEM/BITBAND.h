#ifndef __BITBAND__H_
#define __BITBAND__H_

#define BITBAND(addr,bitnum)     (0x42000000+(addr&0xfffff)*32 + bitnum*4)
#define MER_ADDR(addr)           *((volatile unsigned int*)addr)
#define BIT_ADDR(addr,bitnum)    MER_ADDR(BITBAND(addr,bitnum))  

#define GPIOH_IDR_ADDR           (GPIOH_BASE + 0X10)
#define PHin(n)                  BIT_ADDR(GPIOH_IDR_ADDR,n) 
#define GPIOH_ODR_ADDR           (GPIOH_BASE + 0X14)
#define PHout(n)                 BIT_ADDR(GPIOH_ODR_ADDR,n) 

#define GPIOG_IDR_ADDR           (GPIOG_BASE + 0X10)
#define PGin(n)                  BIT_ADDR(GPIOG_IDR_ADDR,n) 
#define GPIOG_ODR_ADDR           (GPIOG_BASE + 0X14)
#define PGout(n)                 BIT_ADDR(GPIOG_ODR_ADDR,n) 

#define GPIOF_IDR_ADDR           (GPIOF_BASE + 0X10)
#define PFin(n)                  BIT_ADDR(GPIOF_IDR_ADDR,n) 
#define GPIOF_ODR_ADDR           (GPIOF_BASE + 0X14)
#define PFout(n)                 BIT_ADDR(GPIOF_ODR_ADDR,n)   

#define GPIOE_IDR_ADDR           (GPIOE_BASE + 0X10)
#define PEin(n)                  BIT_ADDR(GPIOE_IDR_ADDR,n)  
#define GPIOE_ODR_ADDR           (GPIOE_BASE + 0X14)
#define PEout(n)                 BIT_ADDR(GPIOE_ODR_ADDR,n) 

#define GPIOD_IDR_ADDR           (GPIOD_BASE + 0X10)
#define PDin(n)                  BIT_ADDR(GPIOD_IDR_ADDR,n)  
#define GPIOD_ODR_ADDR           (GPIOD_BASE + 0X14)
#define PDout(n)                 BIT_ADDR(GPIOD_ODR_ADDR,n)

#define GPIOC_IDR_ADDR           (GPIOC_BASE + 0X10)
#define PCin(n)                  BIT_ADDR(GPIOC_IDR_ADDR,n)  
#define GPIOC_ODR_ADDR           (GPIOC_BASE + 0X14)
#define PCout(n)                 BIT_ADDR(GPIOC_ODR_ADDR,n)  

#define GPIOB_IDR_ADDR           (GPIOB_BASE + 0X10)
#define PBin(n)                  BIT_ADDR(GPIOB_IDR_ADDR,n)  
#define GPIOB_ODR_ADDR           (GPIOB_BASE + 0X14)
#define PBout(n)                 BIT_ADDR(GPIOB_ODR_ADDR,n)  

#define GPIOA_IDR_ADDR           (GPIOA_BASE + 0X10)
#define PAin(n)                  BIT_ADDR(GPIOA_IDR_ADDR,n) 
#define GPIOA_ODR_ADDR           (GPIOA_BASE + 0X14)
#define PAout(n)                 BIT_ADDR(GPIOA_ODR_ADDR,n)  

#endif
