#include "lcd_init.h"
#include "lcd.h"

void member_display()
{
	LCD_Init();
	LCD_Fill(0,0,128,128,BLACK);
	LCD_ShowChinese(0,0,"通工",WHITE,BLACK,16,0);
	LCD_ShowString(30,00,"2208",WHITE,BLACK,16,0);
	LCD_ShowChinese(63,0,"班",WHITE,BLACK,16,0);
	LCD_ShowChinese(0,22,"第二组",WHITE,BLACK,16,0);
	LCD_ShowString(50,22,":",WHITE,BLACK,16,0);
	LCD_ShowChinese(0,42,"组长",WHITE,BLACK,16,0);
	LCD_ShowString(33,42,":",WHITE,BLACK,16,0);
	LCD_ShowChinese(43,42,"王圣翔",WHITE,BLACK,16,0);
	LCD_ShowChinese(0,62,"组员",WHITE,BLACK,16,0);
	LCD_ShowString(33,62,":",WHITE,BLACK,16,0);
	LCD_ShowChinese(43,62,"李良源",WHITE,BLACK,16,0);
	LCD_ShowChinese(43,82,"边奕隆",WHITE,BLACK,16,0);
	LCD_ShowChinese(43,102,"李文秀",WHITE,BLACK,16,0);
}
