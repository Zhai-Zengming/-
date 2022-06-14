/*
 * @brief : 矩阵键盘，按下按键在数码管显示相应数字
 *
 * @date  : 2022/3/11
 */

#include <STC15F2K60S2.H>

u8 keyval = 0;

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


void delay(u16 t)
{
	while(t--);
}

#if 0
void smgdelay(void)
{
	u8 tt = 100;
	while(tt--);
}
#endif

void showsmg_bit(u8 pos, u8 dat)
{
	hc138_select(6);
	P0 = 1 << pos;
	hc138_select(7);
	P0 = seg7_tab[dat];
//	smgdelay();
//	
//	P0 = 0xff;    //消影
}

void key_scan(void)
{
	P30 = 0; P31 = 1; P32 = 1; P33 = 1;
	P42 = 1; P44 = 1; P34 = 1; P35 = 1;
	if(P44 == 0)
	{
		Delay10ms();
		if(P44 == 0)
		{
			while(P44 == 0);   //松手检测
			keyval = 0;
			showsmg_bit(0, keyval);
		}
	}
	if(P42 == 0)
	{
		Delay10ms();
		if(P42 == 0)
		{
			while(P42 == 0);   //松手检测
			keyval = 1;
			showsmg_bit(0, keyval);
		}
	}
	if(P35 == 0)
	{
		Delay10ms();
		if(P35 == 0)
		{
			while(P35 == 0);   //松手检测
			keyval = 2;
			showsmg_bit(0, keyval);
		}
	}
	if(P34 == 0)
	{
		Delay10ms();
		if(P34 == 0)
		{
			while(P34 == 0);   //松手检测
			keyval = 3;
			showsmg_bit(0, keyval);
		}
	}
	
	
	P30 = 1; P31 = 0; P32 = 1; P33 = 1;
	P42 = 1; P44 = 1; P34 = 1; P35 = 1;
	if(P44 == 0)
	{
		Delay10ms();
		if(P44 == 0)
		{
			while(P44 == 0);   //松手检测
			keyval = 4;
			showsmg_bit(0, keyval);
		}
	}
	if(P42 == 0)
	{
		Delay10ms();
		if(P42 == 0)
		{
			while(P42 == 0);   //松手检测
			keyval = 5;
			showsmg_bit(0, keyval);
		}
	}
	if(P35 == 0)
	{
		Delay10ms();
		if(P35 == 0)
		{
			while(P35 == 0);   //松手检测
			keyval = 6;
			showsmg_bit(0, keyval);
		}
	}
	if(P34 == 0)
	{
		Delay10ms();
		if(P34 == 0)
		{
			while(P34 == 0);   //松手检测
			keyval = 7;
			showsmg_bit(0, keyval);
		}
	}
	
	P30 = 1; P31 = 1; P32 = 0; P33 = 1;
	P42 = 1; P44 = 1; P34 = 1; P35 = 1;
	if(P44 == 0)
	{
		Delay10ms();
		if(P44 == 0)
		{
			while(P44 == 0);   //松手检测
			keyval = 8;
			showsmg_bit(0, keyval);
		}
	}
	if(P42 == 0)
	{
		Delay10ms();
		if(P42 == 0)
		{
			while(P42 == 0);   //松手检测
			keyval = 9;
			showsmg_bit(0, keyval);
		}
	}
	if(P35 == 0)
	{
		Delay10ms();
		if(P35 == 0)
		{
			while(P35 == 0);   //松手检测
			keyval = 10;
			showsmg_bit(0, keyval);
		}
	}
	if(P34 == 0)
	{
		Delay10ms();
		if(P34 == 0)
		{
			while(P34 == 0);   //松手检测
			keyval = 11;
			showsmg_bit(0, keyval);
		}
	}
	
	P30 = 1; P31 = 1; P32 = 1; P33 = 0;
	P42 = 1; P44 = 1; P34 = 1; P35 = 1;
	if(P44 == 0)
	{
		Delay10ms();
		if(P44 == 0)
		{
			while(P44 == 0);   //松手检测
			keyval = 12;
			showsmg_bit(0, keyval);
		}
	}
	if(P42 == 0)
	{
		Delay10ms();
		if(P42 == 0)
		{
			while(P42 == 0);   //松手检测
			keyval = 13;
			showsmg_bit(0, keyval);
		}
	}
	if(P35 == 0)
	{
		Delay10ms();
		if(P35 == 0)
		{
			while(P35 == 0);   //松手检测
			keyval = 14;
			showsmg_bit(0, keyval);
		}
	}
	if(P34 == 0)
	{
		Delay10ms();
		if(P34 == 0)
		{
			while(P34 == 0);   //松手检测
			keyval = 15;
			showsmg_bit(0, keyval);
		}
	}
}

void main()
{
	while(1)
	{
		key_scan();
		delay(10);
	}
}

