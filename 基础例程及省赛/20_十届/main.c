/*
 * @brief : 第十届省赛
 *
 * @date  : 2022/3/16
 */

#include <STC15F2K60S2.H>
#include "iic.h"

float rb2_udat, adc_dat;
u16 pulse;
u16 t0_cnt = 0;
u16 t1_cnt = 0;

u8 code smg_display[]={                       //标准字库
//   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71,
//black  -     H    J    K    L    N    o   P    U     t    G    Q    r   M    y
    0x00,0x40,0x76,0x1E,0x70,0x38,0x37,0x5C,0x73,0x3E,0x78,0x3d,0x67,0x50,0x37,0x6e};

u8 code smgdot_display[] = {0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};    //0. 1. 2. 3. 4. 5. 6. 7. 8. 9. -1
    
void smgshow(void);


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

void sys_init(void)
{
	hc138_select(4);
	P0 = 0xff;
	hc138_select(5);
	P0 = 0x00;
}

void smg_delay()
{
	u8 i = 100;
	while(i--);
}

void smgshow_bit(u8 pos, u8 dat)
{
	hc138_select(6);
	P0 = 1 << pos;
	hc138_select(7);
	P0 = dat;
	smg_delay();
	P0 = 0xff;
}

void pcf8591_adc(u8 addr)
{
	IIC_Start();
	IIC_SendByte(0x90);
	IIC_WaitAck();
	IIC_SendByte(addr);
	IIC_WaitAck();
	IIC_Stop();
	
	IIC_Start();
	IIC_SendByte(0x91);
	IIC_WaitAck();
	adc_dat = IIC_RecByte();
	IIC_SendAck(0);
	IIC_Stop();
	
	rb2_udat = (adc_dat * 500) / 255;   //扩大100倍
}

void smgshow_rb2(void)
{
	smgshow_bit(0, ~smg_display[25]);
	smgshow_bit(5, ~smgdot_display[(int)rb2_udat / 100]);
	smgshow_bit(6, ~smg_display[(int)rb2_udat / 10 % 10]);
	smgshow_bit(7, ~smg_display[(int)rb2_udat % 10]);
}

void pcf8591_dac(u8 dat)
{
	IIC_Start();
	IIC_SendByte(0x90);
	IIC_WaitAck();
	IIC_SendByte(0x40);
	IIC_WaitAck();
	IIC_SendByte(dat);
	IIC_WaitAck();
	IIC_Stop();
}

void tim_init(void)
{
	TMOD = 0x16;
	TH1 = (65535 - 50000) / 255;
	TL1 = (65535 - 50000) % 255;
	ET1 = 1;
	TR1 = 1;
	
	TH0 = 0xff;
	TL0 = 0xff;
	ET0 = 1;
	TR0 = 1;
	
	EA = 1;
}

void smgshow_pulse(void)
{
	smgshow_bit(0, ~smg_display[15]);
	if(pulse > 9999)
	{
		smgshow_bit(3, ~smg_display[pulse / 10000 % 10]);
	}
	if(pulse > 999)
	{
		smgshow_bit(4, ~smg_display[pulse / 1000 % 10]);
	}
	if(pulse > 99)
	{
		smgshow_bit(5, ~smg_display[pulse / 100 % 10]);
	}
	if(pulse > 9)
	{
		smgshow_bit(6, ~smg_display[pulse / 10 % 10]);
	}
	smgshow_bit(7, ~smg_display[pulse % 10]);
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

u8 s4_flag = 0, s5_flag = 0, s6_flag = 0, s7_flag = 0;
void key_scan(void)
{
	if(P33 == 0)              //切换smg界面
	{
		Delay10ms();
		if(P33 == 0)
		{
			while(P33 == 0)
				smgshow();
			s4_flag++;
			if(s4_flag == 2)
				s4_flag = 0;
		}
	}
	
	if(P32 == 0)         //切换dac输出
	{
		Delay10ms();
		if(P32 == 0)
		{
			while(P32 == 0)
				smgshow();
			s5_flag++;
			if(s5_flag == 2)
				s5_flag = 0;
		}
	}
	
	if(P31 == 0)              //控制LED开关
	{
		Delay10ms();
		if(P31 == 0)
		{
			while(P31 == 0)
				smgshow();
			s6_flag = !s6_flag;
			if(s6_flag)
			{
				hc138_select(4);
				P0 = 0xff;
			}
		}
	}
	
	if(P30 == 0)         //控制SMG开关
	{
		Delay10ms();
		if(P30 == 0)
		{
			while(P30 == 0)
				smgshow();
			s7_flag = !s7_flag;
			if(s7_flag)
			{
				hc138_select(6);
				P0 = 0xff;
				hc138_select(7);
				P0 = 0xff;
			}
		}
	}
}

void led_work(void)
{
	if(s6_flag == 0)
	{
		if(s4_flag == 1)   //测脉冲时，L1灭，L2亮
		{
			hc138_select(4);
			P00 = 1;
			P01 = 0;
			if((pulse < 1000) || ((5000 <= pulse) && (pulse < 10000)))
				P03 = 1;
			else
				P03 = 0;
		}
		
		if(s4_flag == 0)   //测电压时，L1亮，L2灭
		{
			hc138_select(4);
			P00 = 0;
			P01 = 1;
			hc138_select(4);
			if(((rb2_udat / 100) < 1.5) || ((2.5 <= (rb2_udat / 100)) && ((rb2_udat / 100)< 3.5)))
				P02 = 1;
			else
				P02 = 0;
		}
		
		hc138_select(4);
		if(s5_flag == 0)   //DAC输出2.0V时，L5 熄灭，DAC 输出跟随RB2变化时，L5 点亮。
			P04 = 1;
		else if(s5_flag == 1)
			P04 = 0;
	}
}


void pcf8591_out(void)
{
	if(s5_flag == 0)
		pcf8591_dac(102);
	else if(s5_flag == 1)
		pcf8591_dac((u8)adc_dat);
}

void smgshow(void)
{
	if(s7_flag == 0)
	{
		if(s4_flag == 1)
			smgshow_pulse();
		else
		{
			pcf8591_adc(0x43);
			smgshow_rb2();
		}
	}
}




void main()
{
	sys_init();
	tim_init();
	while(1)
	{
		key_scan();
		pcf8591_out();
		smgshow();
		led_work();
	}
}


void tim0() interrupt 1
{
	t0_cnt++;
}

void tim1() interrupt 3
{
	TH1 = (65535 - 50000) / 255;
	TL1 = (65535 - 50000) % 255;
	t1_cnt++;
	if(t1_cnt == 20)
	{
		t1_cnt = 0;
		pulse = t0_cnt;
		t0_cnt = 0;
	}
}



