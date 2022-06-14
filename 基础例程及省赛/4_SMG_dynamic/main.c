/*
 * @brief : 数码管动态显示年份，月份
 * @date  : 2022/3/10
 */
 
 
#include <reg52.h>

u8 month = 1;

u8 code seg7_tab[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,
                    0x82,0xf8,0x80,0x90,0x88,0x80,
                    0xc6,0xc0,0x86,0x8e,0xbf,0x7f};            //共阳极最后两位是 - .

void showsmg(void);

					
void delay(u16 t)
{
	while(t--)
	{
		showsmg();
	}
}

void smg_delay(void)
{
	u16 tt = 100;
	while(tt--);
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

void system_init(void)
{
	hc138_select(5);
	P0 = 0;
}

void showsmg_bit(u8 pos, u8 dat)
{
	hc138_select(6);     //位选
	P0 = (1 << pos);
	hc138_select(7);     //段选
	P0 = seg7_tab[dat];
	smg_delay();
	hc138_select(7);     //消影
	P0 = 0xff;
}

void showsmg(void)
{
	showsmg_bit(0, 2);
	
	showsmg_bit(1, 0);

	showsmg_bit(2, 2);
	
	showsmg_bit(3, 2);
	
	showsmg_bit(4, 16);      //显示'-'
	
	showsmg_bit(5, 16);
	
	showsmg_bit(6, month/10);
	
	showsmg_bit(7, month%10);
	
}

void main()
{
	system_init();
	
	while(1)
	{
		showsmg();
		delay(500);
		month++;
		if(month > 12)
			month = 1;
	}
}