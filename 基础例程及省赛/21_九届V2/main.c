/*
 * @brief : 第九届省赛
 *
 * @date  : 2022/3/17
 */

#include <reg52.h>
#include "iic.h"
#include <absacc.h>

u8 mode = 0;
u8 led_state = 0;
u8  ms800_flag = 0;
u16 ms800_cnt = 0;
u8 adc_dat = 0, duty = 0;
u16 mode_delay[4] = {0};
u16 mode_cnt = 0;
u8 mode_flag = 0;
u8 pwm_cnt = 0;
u8 s7_flag = 0, s6_flag = 0;
u8 show_mode = 0;
u8 mode0_finish = 0, mode1_finish = 0, mode2_finish = 0, mode3_finish = 1;

u8 code smg_display[]={                       //标准字库
//   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71,
//black  -     H    J    K    L    N    o   P    U     t    G    Q    r   M    y
    0x00,0x40,0x76,0x1E,0x70,0x38,0x37,0x5C,0x73,0x3E,0x78,0x3d,0x67,0x50,0x37,0x6e,
    0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};    //0. 1. 2. 3. 4. 5. 6. 7. 8. 9. -1

void Delay5ms(void);
u8 read_eeprom(u8 pos);

void sys_init(void)
{
	u8 i;
	
	XBYTE[0x8000] = 0xff;
	XBYTE[0xa000] = 0x00;
	XBYTE[0xc000] = 0xff;
	XBYTE[0xe000] = 0xff;
	
	Delay5ms();    //这里不加延时不能正常工作
	mode_delay[0] = read_eeprom(1) * 10;
	mode_delay[1] = read_eeprom(2) * 10;
	mode_delay[2] = read_eeprom(3) * 10;
	mode_delay[3] = read_eeprom(4) * 10;
	
	for(i = 0; i < 4; i++)
	{
		if((mode_delay[i] < 400) || (mode_delay[i] > 1200))
		{
			mode_delay[i] = 400;
		}
	}
}

void smg_delay(u16 t)
{
	while(t--);
}

void smgshow_bit(u8 pos, u8 dat)
{
	XBYTE[0xc000] = 1 << pos;
	XBYTE[0xe000] = dat;
	smg_delay(100);
	XBYTE[0xe000] = 0xff;
}

void pcf8591_adc(void)
{
	IIC_Start();
	IIC_SendByte(0x90);
	IIC_WaitAck();
	IIC_SendByte(0x43);        //AIN3
	IIC_WaitAck();
	
	IIC_Start();
	IIC_SendByte(0x91);
	IIC_WaitAck();
	adc_dat = IIC_RecByte();
	IIC_SendAck(0);
	IIC_Stop();
	
	if(adc_dat < 60)
	{
		duty = 25;
	}
	else if((adc_dat > 60) && (adc_dat < 120))
	{
		duty = 50;
	}
	else if((adc_dat > 120) && (adc_dat < 180))
	{
		duty = 75;
	}
	else if(adc_dat > 180)
	{
		duty = 100;
	}
}

void write_eeprom(u8 pos, u8 dat)
{
	IIC_Start();
	IIC_SendByte(0xa0);
	IIC_WaitAck();
	IIC_SendByte(pos);
	IIC_WaitAck();
	
	IIC_SendByte(dat);
	IIC_WaitAck();
	IIC_Stop();
}

u8 read_eeprom(u8 pos)
{
	u8 dat;
	
	IIC_Start();
	IIC_SendByte(0xa0);
	IIC_WaitAck();
	IIC_SendByte(pos);
	IIC_WaitAck();
	
	IIC_Start();
	IIC_SendByte(0xa1);
	IIC_WaitAck();
	dat = IIC_RecByte();
	IIC_SendAck(1);
	IIC_Stop();
	
	return dat;
}

void tim_init(void)
{
	TMOD = 0x01;
	TH0 = (65535 - 100) / 256;
	TL0 = (65535 - 100) % 256;
	ET0 = 1;
	TR0 = 1;
	EA = 1;
}

void mode0(void)
{
	static u8 i = 0;
	
	if((mode_flag == 1) && (mode3_finish == 1))
	{
		mode = 0;
		mode_flag = 0;
		led_state = ~(1 << i);
		XBYTE[0x8000] = led_state;
		
		i++;
		if(i == 8)
		{
			i = 0;
			mode3_finish = 0;
			mode0_finish = 1;
		}
	}
}

void mode1(void)
{
	static u8 j = 0;
	
	if((mode_flag == 1) && (mode0_finish == 1))
	{
		mode = 1;
		mode_flag = 0;
		led_state = ~(0x80 >> j);
		XBYTE[0x8000] = led_state;
		
		j++;
		if(j == 8)
		{
			j = 0;
			mode0_finish = 0;
			mode1_finish = 1;
		}
	}
}

void mode2(void)
{
	static u8 k = 0;
	
	if((mode_flag == 1) && (mode1_finish == 1))
	{
		mode = 2;
		mode_flag = 0;
		led_state = (~(0x80 >> k)) & (~(1 << k));
		XBYTE[0x8000] = led_state;
		
		k++;
		if(k == 4)
		{
			k = 0;
			mode1_finish = 0;
			mode2_finish = 1;
		}
	}
}

void mode3(void)
{
	static u8 l = 0;
	
	if((mode_flag == 1) && (mode2_finish == 1))
	{
		mode = 3;
		mode_flag = 0;
		led_state = (~(0x08 >> l)) & (~(0x10 << l));
		XBYTE[0x8000] = led_state;
		
		l++;
		if(l == 4)
		{
			l = 0;
			mode2_finish = 0;
			mode3_finish = 1;
		}
	}
}

void led_work(void)
{
	if(s7_flag)
	{
		mode0();
		mode1();
		mode2();
		mode3();
	}
	else
	{
		XBYTE[0x8000] = 0xff;
	}
}

void smg_lightgrade(void)
{
	smgshow_bit(6, ~smg_display[17]);
	smgshow_bit(7, ~smg_display[duty / 25]);   // 25/25=1 75/25=2 50/25=2 100/25=4 
}

void smg_mode(void)
{
	if(s6_flag == 0)
	{
		XBYTE[0xc000] = 0xff;
		XBYTE[0xe000] = 0xff;
	}
	else if(s6_flag == 1)
	{
		if(ms800_flag == 1)
		{
			smgshow_bit(0, ~smg_display[17]);
			smgshow_bit(1, ~smg_display[show_mode + 1]);
			smgshow_bit(2, ~smg_display[17]);
		}
		else
		{
			smgshow_bit(0, 0xff);
			smgshow_bit(1, 0xff);
			smgshow_bit(2, 0xff);
		}
		smgshow_bit(4, ~smg_display[mode_delay[show_mode] / 1000]);
		smgshow_bit(5, ~smg_display[mode_delay[show_mode] / 100 % 10]);
		smgshow_bit(6, ~smg_display[mode_delay[show_mode] / 10 % 10]);
		smgshow_bit(7, ~smg_display[mode_delay[show_mode] % 10]);
		
	}
	else if(s6_flag == 2)
	{
		smgshow_bit(0, ~smg_display[17]);
		smgshow_bit(1, ~smg_display[show_mode + 1]);
		smgshow_bit(2, ~smg_display[17]);
	
		if(ms800_flag == 1)
		{
			smgshow_bit(4, ~smg_display[mode_delay[show_mode] / 1000]);
			smgshow_bit(5, ~smg_display[mode_delay[show_mode] / 100 % 10]);
			smgshow_bit(6, ~smg_display[mode_delay[show_mode] / 10 % 10]);
			smgshow_bit(7, ~smg_display[mode_delay[show_mode] % 10]);
		}
		else
		{
			smgshow_bit(4, 0xff);
			smgshow_bit(5, 0xff);
			smgshow_bit(6, 0xff);
			smgshow_bit(7, 0xff);
		}
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

void Delay5ms()		//@11.0592MHz
{
	unsigned char i, j;

	i = 54;
	j = 199;
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
				led_work();
				smg_mode();
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
				smg_mode();
				led_work();
			}
			s6_flag++;
			if(s6_flag == 3)
			{
				s6_flag = 0;
			}
		}
	}
	
	if((P32 == 0) && (s6_flag))
	{
		Delay10ms();
		if(P32 == 0)
		{
			while(P32 == 0)
			{
				smg_mode();
				led_work();
			}
			if(s6_flag == 1)
			{
				if(show_mode < 3)
				{
					show_mode++;
				}
				
			}
			else if(s6_flag == 2)
			{
				if(mode_delay[show_mode] < 1200)
				{
					mode_delay[show_mode] += 100;
					write_eeprom(show_mode + 1, mode_delay[show_mode] / 10);
					Delay5ms();   //等待写入完成
				}
			}
		}
	}
	
	if(P33 == 0)
	{
		Delay10ms();
		if(P33 == 0)
		{
			while(P33 == 0)
			{
				if(s6_flag == 0)
					smg_lightgrade();
				else
					smg_mode();
				led_work();
			}
			if(s6_flag == 1)
			{
				if(show_mode)
				{
					show_mode--;
				}
			}
			else if(s6_flag == 2)
			{
				if(mode_delay[show_mode] > 400)
				{
					mode_delay[show_mode] -= 100;
					write_eeprom(show_mode + 1, mode_delay[show_mode] / 10);
					Delay5ms();   //等待写入完成
				}
			}
		}
	}
}

void main()
{
	sys_init();
	tim_init();
	while(1)
	{
		led_work();
		smg_mode();
		key_scan();
		pcf8591_adc();
	}
}

void tim0() interrupt 1
{
	TH0 = (65535 - 100) / 256;
	TL0 = (65535 - 100) % 256;
	
	mode_cnt++;
	if(mode_cnt == (mode_delay[mode] * 10))
	{
		mode_cnt = 0;
		mode_flag = 1;
	}
	
	pwm_cnt++;
	if(pwm_cnt == 100)
	{
		pwm_cnt = 0;
	}
	
	if(s7_flag)
	{
		if(pwm_cnt < duty)
		{
			XBYTE[0x8000] = led_state;
		}
		else if(pwm_cnt > duty)
		{
			XBYTE[0x8000] = 0xff;
		}
	}
	
	ms800_cnt++;
	if(ms800_cnt == 8000)
	{
		ms800_cnt = 0;
		ms800_flag = !ms800_flag;
		
	}
}





