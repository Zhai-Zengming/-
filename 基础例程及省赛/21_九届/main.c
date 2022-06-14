/*
 * @brief : 第九届省赛
 *
 * @date  : 2022/3/17
 */

#include <STC15F2K60S2.H>
#include "iic.h"
#include <absacc.h>

u8 mode;
u8 t1_800ms = 0, t1_800flag = 0;
u8 adc_dat = 0, duty = 0;
u8 mode_delay[4] = {8, 8, 8, 8};      //50ms * 8 = 400ms;
u8 mode_cnt[4] = {0}, mode_flag[4] = {0};
u8 t0_cnt = 0, t0_flag = 0;
u8 s7_flag = 0, s6_flag = 0, s5_flag = 0, s4_flag = 0;

u8 code smg_display[]={                       //标准字库
//   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71,
//black  -     H    J    K    L    N    o   P    U     t    G    Q    r   M    y
    0x00,0x40,0x76,0x1E,0x70,0x38,0x37,0x5C,0x73,0x3E,0x78,0x3d,0x67,0x50,0x37,0x6e,
    0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};    //0. 1. 2. 3. 4. 5. 6. 7. 8. 9. -1

void smgshow(void);
void mode4(void);
void mode3(void);
void mode2(void);
void mode1(void);
void key_scan(void);


void sys_init(void)
{
	XBYTE[0x8000] = 0xff;
	XBYTE[0xa000] = 0x00;
	XBYTE[0xc000] = 0xff;
	XBYTE[0xe000] = 0xff;
}

void smg_delay(void)
{
	u8 i = 70;
	while(i--);
}

void smgshow_bit(u8 pos, u8 dat)
{
	XBYTE[0xc000] = 1 << pos;
	XBYTE[0xe000] = dat;
	smg_delay();
	XBYTE[0xe000] = 0xff;
}

void pcf8591_adc(void)
{
	IIC_Start();
	IIC_SendByte(0x90);
	IIC_WaitAck();
	IIC_SendByte(0x43);        //AIN3
	IIC_WaitAck();
	IIC_Stop();
	
	IIC_Start();
	IIC_SendByte(0x91);
	IIC_WaitAck();
	adc_dat = IIC_RecByte();
	IIC_SendAck(0);
	IIC_Stop();
	
	if(adc_dat <= (255 / 4))
	{
		duty = 25;
	}
	else if(adc_dat <= (255 / 2))
	{
		duty = 50;
	}
	else if(adc_dat <= (255 * 3 / 4))
	{
		duty = 75;
	}
	else if(adc_dat < 255)
	{
		duty = 100;
	}
}

void write_eeprom(u8 pos, u8 dat)
{
	IIC_Start();
	IIC_SendByte(0x90);
	IIC_WaitAck();
	IIC_SendByte(pos);        //AIN3
	IIC_WaitAck();
	
	IIC_SendByte(dat);
	IIC_WaitAck();
	IIC_Stop();
}

/* tim0产生pwm, tim1定时50ms */
void tim_init(void)
{
	TMOD = 0x11;
	TH0 = (65535 - 100) / 256;
	TL0 = (65535 - 100) % 256;
	TH1 = (65535 - 50000) / 256;
	TL1 = (65535 - 50000) % 256;
	ET0 = 1;
	TR0 = 1;
	ET1 = 1;
	TR1 = 1;
	EA = 1;
}

void mode1(void)
{
	u8 i = 0;
	mode = 1;
	for(i = 0; i < 8; i++)
	{
		if(s7_flag)
		{
			XBYTE[0x8000] = ~(1 << i);
			while(mode_flag[0] == 0)
			{
				if(t0_flag == 1)
					XBYTE[0x8000] = ~(1 << i);
				smgshow();
			}
			mode_flag[0] = 0;
		}
	}
}

void mode2(void)
{
	u8 i = 0;
	mode = 2;
	for(i = 0; i < 8; i++)
	{
		if(s7_flag)
		{
			XBYTE[0x8000] = ~(0x80 >> i);
			while(mode_flag[1] == 0)
			{
				if(t0_flag == 1)
					XBYTE[0x8000] = ~(0x80 >> i);
				smgshow();
			}
			mode_flag[1] = 0;
		}
	}
}

void mode3(void)
{
	u8 i = 0;
	mode = 3;
	for(i = 0; i < 4; i++)
	{
		if(s7_flag)
		{
			XBYTE[0x8000] = (~(0x80 >> i)) & (~(1 << i));
			while(mode_flag[2] == 0)
			{
				if(t0_flag == 1)
					XBYTE[0x8000] = (~(0x80 >> i)) & (~(1 << i));
				smgshow();
			}
			mode_flag[2] = 0;
		}
	}
}

void mode4(void)
{
	u8 i = 0;
	mode = 4;
	for(i = 0; i < 4; i++)
	{
		if(s7_flag)
		{
			XBYTE[0x8000] = (~(0x08 >> i)) & (~(0x10 << i));
			while(mode_flag[3] == 0)
			{
				if(t0_flag == 1)
					XBYTE[0x8000] = (~(0x08 >> i)) & (~(0x10 << i));
				smgshow();
			}
			mode_flag[3] = 0;
		}
	}
}

void led_work(void)
{
	if(s7_flag)
	{
		mode1();
		mode2();
		mode3();
		mode4();
	}
}

void smg_lightgrade(void)
{
	smgshow_bit(6, ~smg_display[17]);
	smgshow_bit(7, ~smg_display[duty / 25]);   // 25/25=1 75/25=2 50/25=2 100/25=4 
}

void smg_mode(void)
{
	smgshow_bit(0, ~smg_display[17]);
	smgshow_bit(1, ~smg_display[mode]);
	smgshow_bit(2, ~smg_display[17]);
	
	if((mode_delay[mode] * 50) > 999)
	{
		smgshow_bit(4, ~smg_display[mode_delay[mode] * 50 / 1000]);
	}
	smgshow_bit(5, ~smg_display[mode_delay[mode] * 50 / 100]);
	smgshow_bit(6, ~smg_display[0]);
	smgshow_bit(7, ~smg_display[0]);
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
	if(P30 == 0)
	{
		Delay10ms();
		if(P30 == 0)
		{
			while(P30 == 0)
			{
//				led_work();
			}
			s7_flag = !s7_flag;
		}
	}
	
	if(P31 == 0)
	{
		Delay10ms();
		if(P31 == 0)
		{
			while(P31 == 0)
			{
//				led_work();
			}
			s6_flag++;
			if(s6_flag == 3)
			{
				s6_flag = 0;
				XBYTE[0xc000] = 0xff;
				XBYTE[0xe000] = 0xff;
//				write_eeprom(0x01, mode_delay[]);
			}
		}
	}
}

void smgshow(void)
{
	if(s6_flag == 1)
	{
//		smg_mode();
		while(t1_800flag == 0)   //延时800ms
		{
			smg_mode();
//			led_work();
		}
		t1_800flag = 0;
		
		XBYTE[0xc000] = 0x07;
		XBYTE[0xe000] = 0xff;
		while(t1_800flag == 0)
		{
			smgshow_bit(0, ~smg_display[17]);
			smgshow_bit(1, ~smg_display[mode]);
			smgshow_bit(2, ~smg_display[17]);
		}
		t1_800flag = 0;
	}
	
	if(s6_flag == 2)
	{
//		smg_mode();
		while(t1_800flag == 0)   //延时800ms
		{
			smg_mode();
//			led_work();
		}
		t1_800flag = 0;
		
		XBYTE[0xc000] = 0xe0;
		XBYTE[0xe000] = 0xff;
		while(t1_800flag == 0)
		{
			if((mode_delay[mode] * 50) > 999)
			{
				smgshow_bit(4, ~smg_display[mode_delay[mode] * 50 / 1000]);
			}
			smgshow_bit(5, ~smg_display[mode_delay[mode] * 50 / 100]);
			smgshow_bit(6, ~smg_display[0]);
			smgshow_bit(7, ~smg_display[0]);
		}
		t1_800flag = 0;
	}
}

void main()
{
	sys_init();
	tim_init();
	while(1)
	{
		led_work();
		smgshow();
	}
}


void tim0() interrupt 1
{
	TH0 = (65535 - 100) / 256;
	TL0 = (65535 - 100) % 256;
	
	key_scan();
	
	t0_cnt++;
	if(t0_cnt == 100)
		t0_cnt = 0;
	if(t0_cnt < duty)
	{
		t0_flag = 1;
	}
	else
	{
		t0_flag = 0;
		XBYTE[0x8000] = 0xff;
	}
}

void tim1() interrupt 3
{
	TH1 = (65535 - 50000) / 256;
	TL1 = (65535 - 50000) % 256;
	mode_cnt[0]++;
	mode_cnt[1]++;
	mode_cnt[2]++;
	mode_cnt[3]++;
	t1_800ms++;
	if(mode_cnt[0] == mode_delay[0])
	{
		mode_cnt[0] = 0;
		mode_flag[0] = 1;
		pcf8591_adc();
	}
	if(mode_cnt[1] == mode_delay[1])
	{
		mode_cnt[1] = 0;
		mode_flag[1] = 1;
	}
	if(mode_cnt[2] == mode_delay[2])
	{
		mode_cnt[2] = 0;
		mode_flag[2] = 1;
	}
	if(mode_cnt[3] == mode_delay[3])
	{
		mode_cnt[3] = 0;
		mode_flag[3] = 1;
	}
	if(t1_800ms == 16)    //16*50=800ms
	{
		t1_800flag = 1;
	}
}




