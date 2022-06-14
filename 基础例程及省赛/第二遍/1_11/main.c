/*
 * @brief : 第二遍第十一届省赛
 *
 * @date  : 2022/4/2
 *
 */

//#include "reg52.h"
#include <STC15F2K60S2.H>
#include "iic.h"
#include "onewire.h"

u8 init_flag = 0;
u8 temph, templ;
u16 temp;
u8 t_min = 20, t_max = 30;
u8 t_max_pre, t_min_pre;
u8 s4_flag = 0, s5_flag = 0, s6_flag = 0, s7_flag = 0;

u8 code t_display[]={                       //标准字库
//   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71,
//black  -     H    J    K    L    N    o   P    U     t    G    Q    r   M    y
    0x00,0x40,0x76,0x1E,0x70,0x38,0x37,0x5C,0x73,0x3E,0x78,0x3d,0x67,0x50,0x37,0x6e,
    0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};    //0. 1. 2. 3. 4. 5. 6. 7. 8. 9. -1

void smgshow();
	
void hc138_select(u8 n)
{
	switch(n)
	{
		case 4:
			P2 = P2 & 0x1f | 0x80;  break;
		case 5:
			P2 = P2 & 0x1f | 0xa0;  break;
		case 6:
			P2 = P2 & 0x1f | 0xc0;  break;
		case 7:
			P2 = P2 & 0x1f | 0xe0;  break;
	}
}

void smg_delay(u16 i)
{
	while(i--);
}

void smgshow_bit(u8 pos, u8 dat)
{
	hc138_select(6);
	P0 = 1 << pos;
	hc138_select(7);
	P0 = dat;
	smg_delay(100);
	P0 = 0xff;
}

void delay(u16 t)
{
	while(t--)
	{
		if(init_flag)
			smgshow();
	}
}

void get_temp()
{
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0x44);
	delay(10);
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0xbe);
	
	templ = Read_DS18B20();
	temph = Read_DS18B20();
	temp = temph;
	temp = (temp << 8) | templ;
	if((temp & 0xf800) == 0x0000)
	{
		temp = temp >> 4;
	}
}

void smgshow_temp()
{
	smgshow_bit(0, ~t_display[12]);
	smgshow_bit(6, ~t_display[temp / 10]);
	smgshow_bit(7, ~t_display[temp % 10]);
}

void smgshow_para()
{
	smgshow_bit(0, ~t_display[24]);
	smgshow_bit(3, ~t_display[t_max / 10]);
	smgshow_bit(4, ~t_display[t_max % 10]);
	smgshow_bit(6, ~t_display[t_min / 10]);
	smgshow_bit(7, ~t_display[t_min % 10]);
}

void sys_init()
{
	hc138_select(4);
	P0 = 0xff;
	hc138_select(5);
	P0 = 0x00;
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


void key_scan()
{
	if(P33 == 0)
	{
		Delay10ms();
		if(P33 == 0)
		{
			while(P33 == 0)
			{
				smgshow();
			}
			s4_flag = !s4_flag;
			if(s4_flag)
			{
				s5_flag = 0;
				t_min_pre = t_min;   //进入参数界面保存参数
				t_max_pre = t_max;
			}
			else                     //进入数据界面检查参数合理性
			{
				if(t_min > t_max)
				{
					t_min = t_min_pre;
					t_max = t_max_pre;
				}
			}
		}
	}
	if((P32 == 0) && (s4_flag == 1))
	{
		Delay10ms();
		if(P32 == 0)
		{
			while(P32 == 0)
			{
				smgshow();
			}
			s5_flag = !s5_flag;
		}
	}
	if((P31 == 0) && (s4_flag == 1))
	{
		Delay10ms();
		if(P31 == 0)
		{
			while(P31 == 0)
			{
				smgshow();
			}
			if((s5_flag == 0) && (t_min < 99))
			{
				t_min++;
			}
			else if((s5_flag == 1) && (t_max < 99))
			{
				t_max++;
			}
		}
	}
	if((P30 == 0) && (s4_flag == 1))
	{
		Delay10ms();
		if(P30 == 0)
		{
			while(P30 == 0)
			{
				smgshow();
			}
			if((s5_flag == 0) && (t_min > 0))
			{
				t_min--;
			}
			else if((s5_flag == 1) && (t_max > 0))
			{
				t_max--;
			}
		}
	}
}

void smgshow()
{
	if(s4_flag == 0)
	{
		smgshow_temp();
	}
	else
		smgshow_para();
}

void detect_temp()      //监测温度，以下四种情况只执行一种
{
	if(t_max < t_min)   //这一句放在最前面，优先级最高
	{
		hc138_select(4);
		P00 = 1;
		P01 = 1;
		P02 = 1;
		P03 = 0;
	}
	else if(temp > t_max)
	{
		pcf8591_dac(4 * 255 / 5);
		hc138_select(4);
		P00 = 0;
		P01 = 1;
		P02 = 1;
		P03 = 1;
	}
	else if((temp <= t_max) && (temp >= t_min))
	{
		pcf8591_dac(3 * 255 / 5);
		hc138_select(4);
		P00 = 1;
		P01 = 0;
		P02 = 1;
		P03 = 1;
	}
	else if(temp < t_min)
	{
		pcf8591_dac(2 * 255 / 5);
		hc138_select(4);
		P00 = 1;
		P01 = 1;
		P02 = 0;
		P03 = 1;
	}
}

void main()
{
	u8 i;
	
	sys_init();
	for(i = 0; i < 100; i++)
	{
		get_temp();
	}
	init_flag = 1;
	while(1)
	{
		key_scan();
		get_temp();
		smgshow();
		detect_temp();
	}
}