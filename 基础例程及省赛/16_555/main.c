/*
 * @brief : 测量555产生的频率显示在数码管上
 *
 * @date  : 2022/3/14
 *
 */


#include <STC15F2K60S2.H>
#include <absacc.h>

u16 pulse = 0;
u16 pulse_cnt = 0;
u8 tim1_cnt = 0;

u8 code smg_display[]={         //标准字库
//   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71,
//black  -     H    J    K    L    N    o   P    U     t    G    Q    r   M    y
    0x00,0x40,0x76,0x1E,0x70,0x38,0x37,0x5C,0x73,0x3E,0x78,0x3d,0x67,0x50,0x37,0x6e,
    0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};    //0. 1. 2. 3. 4. 5. 6. 7. 8. 9. -1

void system_init(void)
{
	XBYTE[0x8000] = 0xff;
	XBYTE[0xa000] = 0x00;
}

void smg_delay(void)
{
	u8 i = 100;
	while(i--);
}

void smgshow_bit(u8 pos, u8 dat)
{
	XBYTE[0xc000] = 1 << pos;
	XBYTE[0xe000] = dat;
	smg_delay();
	XBYTE[0xe000] = 0xff;
}

void tim_init(void)
{
	TMOD = 0x16;                     //tim0八位自动重装，tim1十六位不可重装
	TH1 = (65535 - 50000) / 256;
	TL1 = (65535 - 50000) % 256;
	TH0 = 0xff;
	TL0 = 0xff;
	
	ET1 = 1;
	ET0 = 1;
	TR1 = 1;
	TR0 = 1;
	EA = 1;
}

void smgshow_pulse(void)
{
	smgshow_bit(0, ~smg_display[15]);      //F
	smgshow_bit(1, 0xff);
	smgshow_bit(2, 0xff);
	
	if(pulse > 9999)     //五位数
	{
		smgshow_bit(3, ~smg_display[pulse / 10000]);
//		smgshow_bit(4, ~smg_display[pulse / 1000 % 10]);
//		smgshow_bit(5, ~smg_display[pulse / 100 % 10]);
//		smgshow_bit(6, ~smg_display[pulse / 10 % 10]);
//		smgshow_bit(7, ~smg_display[pulse % 10]);
	}
	if(pulse > 999)     //四位数
	{
//		smgshow_bit(3, 0xff);
		smgshow_bit(4, ~smg_display[pulse / 1000 % 10]);
//		smgshow_bit(5, ~smg_display[pulse / 100 % 10]);
//		smgshow_bit(6, ~smg_display[pulse / 10 % 10]);
//		smgshow_bit(7, ~smg_display[pulse % 10]);
	}
	if(pulse > 99)     //三位数
	{
//		smgshow_bit(3, 0xff);
//		smgshow_bit(4, 0xff);
		smgshow_bit(5, ~smg_display[pulse / 100 % 10]);
//		smgshow_bit(6, ~smg_display[pulse / 10 % 10]);
//		smgshow_bit(7, ~smg_display[pulse % 10]);
	}
	if(pulse > 9)     //二位数
	{
//		smgshow_bit(3, 0xff);
//		smgshow_bit(4, 0xff);
//		smgshow_bit(5, 0xff);
		smgshow_bit(6, ~smg_display[pulse / 10 % 10]);
//		smgshow_bit(7, ~smg_display[pulse % 10]);
	}
	smgshow_bit(7, ~smg_display[pulse % 10]);
}

void main()
{
	system_init();
	tim_init();
	
	while(1)
	{
		smgshow_pulse();
	}
}



void tim0() interrupt 1
{
	pulse_cnt++;
}

void tim1() interrupt 3
{
	tim1_cnt++;
	TH1 = (65535 - 50000) / 256;
	TL1 = (65535 - 50000) % 256;
	if(tim1_cnt == 20)    //定时1S时间到
	{
		tim1_cnt = 0;
		pulse = pulse_cnt;
		pulse_cnt = 0;
	}
}





