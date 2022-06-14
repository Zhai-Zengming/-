/*
 * @brief : 综合项目，定时器定时50ms,每50ms数码管最后两位加一，加到20即1s, 
 *			数码管秒位加一，分位以此类推
 *			按下S7暂停，S6清零。
 * @date  : 2022/3/11
 *
 */
 
#include <reg52.h>

u8 min, sec, ms_50 = 0;

u8 code seg7_tab[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,
                    0x82,0xf8,0x80,0x90,0x88,0x80,
                    0xc6,0xc0,0x86,0x8e,0xbf,0x7f};            //共阳极最后两位是 - .

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

void tim0_init(void)
{
	TMOD = ((TMOD & 0xf0) | 0x01);
	TH0 = ((65535 - 50000) / 256);
	TL0 = ((65535 - 50000) % 256);
	
	TR0 = 0;
	ET0 = 1;
	EA = 1;
}

void tim0() interrupt 1
{
	TH0 = ((65535 - 50000) / 256);
	TL0 = ((65535 - 50000) % 256);
	
	ms_50++;
	if(ms_50 == 20)
	{
		ms_50 = 0;
		sec++;
		if(sec == 60)
		{
			sec = 0;
			min++;
		}
	}
}

void smg_delay(void)
{
	u8 t = 100;
	while(t--);
}

void showsmg_bit(u8 pos, u8 dat)
{
	hc138_select(6);       //位选
	P0 = 1 << pos;
	hc138_select(7);       //段选
	P0 = seg7_tab[dat];
	
	smg_delay();
	P0 = 0xff;    //消影
//	smg_delay();
}

void showsmg(void)
{
	showsmg_bit(7, ms_50%10);
	showsmg_bit(6, ms_50/10);
	
	showsmg_bit(5, 16);     //显示分隔符‘-’
	
	showsmg_bit(4, sec%10);
	showsmg_bit(3, sec/10);
	
	showsmg_bit(2, 16);     //显示分隔符‘-’
	
	showsmg_bit(1, min%10);
	showsmg_bit(0, min/10);
}

void Delay10ms()		//@11.0592MHz
{
	unsigned char i, j;

	i = 108;
	j = 145;
	do
	{
		while (--j);
	} while (--i);
}


void key_scan(void)
{
	if(P30 == 0)          //S7
	{
		Delay10ms();
		if(P30 == 0)
		{
			while(P30 == 0)
				showsmg();
			TR0 = ~TR0;
		}
	}
	if(P31 == 0)          //S6
	{
		Delay10ms();
		if(P31 == 0)
		{
			while(P31 == 0)
				showsmg();
			min = 0;
			sec = 0;
			ms_50 = 0;
		}
	}
}

void main()
{
	tim0_init();
	while(1)
	{
		showsmg();
		key_scan();
	}
}