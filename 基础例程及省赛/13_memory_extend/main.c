/*
 * @brief : 存储器扩展模式
 *
 * @date  : 2022/3/13
 *
 */


#include <STC15F2K60S2.H>
#include <absacc.h>

void delay(void)
{
	u16 t = 65500,tt = 65500;
	while(t--);
	while(tt--);
}

void led_running(void)
{
	XBYTE[0x8000] = 0xf0;
	delay();
	delay();
	XBYTE[0x8000] = 0x0f;
	delay();
	delay();
	XBYTE[0x8000] = 0xff;
	delay();
	delay();
}


void smg_running(void)
{
	u8 i;
	for(i = 0; i < 8; i++)
	{
		XBYTE[0xc000] = (1 << i);     //位选
		XBYTE[0xe000] = 0x00;
		delay();
		delay();
	}
	XBYTE[0xe000] = 0xff;
}

void main(void)
{
	while(1)
	{
		led_running();
		smg_running();
	}
}