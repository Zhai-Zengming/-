/*
 * @brief : 定时器实现灯的闪烁
 *
 * @date  : 2022/3/11
 */
 
#include <reg52.h>

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

void timer0_init(void)
{
	TMOD = ((TMOD & 0xf0) | 0x01);
	TH0 = ((65535 - 50000) / 256);
	TL0 = ((65535 - 50000) % 256);
	ET0 = 1;
	TR0 = 1;
	EA = 1;
}

void main()
{
	hc138_select(4);
	timer0_init();
	while(1)
	{
		
	}
}


u8 cnt = 0;
void tim0() interrupt 1
{
	TH0 = ((65535 - 50000) / 256);
	TL0 = ((65535 - 50000) % 256);
	
	cnt++;
	if((cnt % 10) == 0)
	{
		P00 = !P00;
	}
	if(cnt == 20)
	{
		cnt = 0;
		P01 = !P01;
	}
}
