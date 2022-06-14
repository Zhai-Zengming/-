/*
 * @brief : 共阳极数码管静态显示，每个数码管依次显示0~9
 *			然后所有数码管一起显示0~F
 * @date  : 2022/3/10
 */


#include <reg52.h>

u8 code seg7_tab[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,
                    0x82,0xf8,0x80,0x90,0x88,0x80,
                    0xc6,0xc0,0x86,0x8e,0xbf,0x7f};            //共阳极最后两位是 - .

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

void system_init(void)     //关闭蜂鸣器等无关器件
{
	hc138_select(5);
	P0 = 0;
}

void showsmg_bit(u8 pos, u8 dat)
{
	hc138_select(6);
	P0 = (1 << pos);        //位选
	hc138_select(7);
	P0 = seg7_tab[dat];     //显示
}

void showsmg(void)
{
	u8 i, j;
	for(i = 0; i < 8; i++)     //八个数码管依次显示0~9
	{
		for(j = 0; j < 10; j++)
		{
			showsmg_bit(i, j);
			delay(60000);
		}
	}
	
	/* 八个数码管全部显示数组内全部内容 */
	hc138_select(6);
	P0 = 0xff;              //八个数码管全部选中
	hc138_select(7);
	for(i = 0; i < 18; i++)
	{
		P0 = seg7_tab[i];     //显示
		delay(60000);
	}
}

void main()
{
	system_init();

	while(1)
	{
		showsmg();
	}
}


