/*
 * @brief : 外部中断，按下按键S5，点亮L5
 * 
 * @date  : 2022/3/11
 */
 
#include <reg52.h>

u8 int0_flag = 0;

void delay(u16 t)
{
	u16 i = 60000;
	while(t--);
	while(i--);
}

void hc138_select(u8 n)
{
	switch(n)
	{
		case 4:
			P2 = ((P2 & 0x1f) | 0x80);  break;
		case 5:
			P2 = ((P2 & 0x1f) | 0xa0);  break;
		case 6:
			P2 = ((P2 & 0x1f) | 0xc0);  break;
		case 7:
			P2 = ((P2 & 0x1f) | 0xe0);  break;
	}
}

void led0_work(void)
{
	hc138_select(4);
	P00 = 0;
	delay(60000);
	P00 = 1;
	delay(60000);
}

void int_init(void)
{
	IT0 = 1;
	EX0 = 1;
	EA = 1;
}

void led5_work(void)
{
	if(int0_flag == 1)
	{
		int0_flag = 0;
		P04 = 0;
		delay(60000);
		delay(60000);
		delay(60000);
		delay(60000);
		P04 = 1;
	}
}

void main()
{
	int_init();
	
	while(1)
	{
		led0_work();
		led5_work();
	}
}

void int0_service() interrupt 0
{
	int0_flag = 1;
}
 