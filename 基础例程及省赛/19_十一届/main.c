/*
 * @brief : 第十一届省赛
 *
 * @date  : 2022/3/15
 */


#include <STC15F2K60S2.H>
#include "iic.h"
#include "onewire.h"

u16 temp;
u8 temph, templ;
char t_max = 30, t_min = 20;
char t_max_pre, t_min_pre;

u8 code smg_display[]={                       //标准字库
//   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71,
//black  -     H    J    K    L    N    o   P    U     t    G    Q    r   M    y
    0x00,0x40,0x76,0x1E,0x70,0x38,0x37,0x5C,0x73,0x3E,0x78,0x3d,0x67,0x50,0x37,0x6e,
    0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};    //0. 1. 2. 3. 4. 5. 6. 7. 8. 9. -1

void smgshow_temp(void);
void smgshow(void);

	
void hc138_select(u8 n)
{
	switch(n)
	{
		case 4:
			P2 = (P2 & 0x1f) | 0x80;  break;
		case 5:
			P2 = (P2 & 0x1f) | 0xa0;  break;
		case 6:
			P2 = (P2 & 0x1f) | 0xc0;  break;
		case 7:
			P2 = (P2 & 0x1f) | 0xe0;  break;
	}
}

void smg_delay(void)
{
	u8 t = 60;
	while(t--);
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

void sys_init(void)
{
	hc138_select(4);
	P0 = 0xff;
	hc138_select(5);
	P0 = 0x00;
	hc138_select(6);
	P0 = 0xff;
	hc138_select(7);
	P0 = 0xff;
}

void delay(u16 tt)
{
	while(tt--)
	{
		smgshow();
	}
}

void get_temp(void)
{
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0x44);
	delay(40);
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0xbe);
	templ = Read_DS18B20();
	temph = Read_DS18B20();
	init_ds18b20();
	
	temp = temph;
	temp = ((temp << 8) | templ);
	if((temp & 0xf800) == 0x0000)
	{
		temp = temp >> 4;
	}
}

void smgshow_temp(void)
{
	smgshow_bit(0, ~smg_display[12]);
	smgshow_bit(6, ~smg_display[temp / 10]);
	smgshow_bit(7, ~smg_display[temp % 10]);
}

void smgshow_settemp(void)
{
	smgshow_bit(0, ~smg_display[24]);
	
	smgshow_bit(3, ~smg_display[t_max / 10]);
	smgshow_bit(4, ~smg_display[t_max % 10]);
	
	smgshow_bit(6, ~smg_display[t_min / 10]);
	smgshow_bit(7, ~smg_display[t_min % 10]);
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

void temp_detect(void)      //检测环境
{
	if(t_min > t_max)
	{
		hc138_select(4);
		P00 = 1;
		P01 = 1;
		P02 = 1;
		P03 = 0;
	}
	else if(temp < t_min)
	{
		pcf8591_dac((2 * 256) / 5);
		hc138_select(4);
		P00 = 1;
		P01 = 1;
		P02 = 0;
		P03 = 1;
	}
	else if(temp > t_max)
	{
		pcf8591_dac(205);
		hc138_select(4);
		P00 = 0;
		P01 = 1;
		P02 = 1;
		P03 = 1;
	}
	else if((t_min <= temp) && (temp <= t_max))
	{
		pcf8591_dac((3 * 256) / 5);
		hc138_select(4);
		P00 = 1;
		P01 = 0;
		P02 = 1;
		P03 = 1;
	}
}

void Delay10ms()		 //@11.0592MHz
{
	unsigned char i, j;

	i = 108;
	j = 145;
	do
	{
		while (--j);
	} while (--i);
}

static u8 s4_flag = 0, s5_flag = 0;
void key_scan(void)
{
	if(P33 == 0)
	{
		Delay10ms();
		if(P33 == 0)
		{
			while(P33 == 0)
				smgshow();
			s4_flag++;
			if(s4_flag == 1)    //每次从数据界面切换到参数界面，默认当前选择的参数是温度下限参数t_min。
			{
				s5_flag = 0;
				t_min_pre = t_min;
				t_max_pre = t_max;
			}
			if(s4_flag == 2)
			{
				s4_flag = 0;
				if(t_min > t_max)     //检查数据合理性
				{
					t_max = t_max_pre;
					t_min = t_min_pre;
				}
			}
		}
	}
	if(P32 == 0)
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
	
	if((P31 == 0) && (s4_flag == 1) )
	{
		Delay10ms();
		if(P31 == 0)
		{
			while(P31 == 0)
				smgshow();
			
			if(s5_flag == 0)     //t_min 加一
			{
				t_min++;
			}
			
			if(s5_flag == 1)     //t_max 加一
			{
				t_max++;
			}
		}
	}
	
	if((P30 == 0) && (s4_flag == 1) )
	{
		Delay10ms();
		if(P30 == 0)
		{
			while(P30 == 0)
				smgshow();
			
			if(s5_flag == 0)     //t_min 减一
			{
				t_min--;
			}
			
			if(s5_flag == 1)     //t_max 减一
			{
				t_max--;
			}
		}
	}
	
	if(t_max > 99)
		t_max = 99;
	if(t_min > 99)
		t_min = 99;
	if(t_max < 0)
		t_max = 0;
	if(t_min < 0)
		t_min = 0;
}


void smgshow(void)
{
	if(s4_flag == 1)
	{
		smgshow_settemp();
	}
	else
		smgshow_temp();
}

void main()
{
	sys_init();
	while(1)
	{
		key_scan();
		get_temp();
		temp_detect();
		smgshow();
	}
}


