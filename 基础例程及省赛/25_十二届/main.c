/*
 * @brief: 第十二届省赛
 *
 * @date : 2022/3/29
 *
 */
 
#include <STC15F2K60S2.H>
#include "iic.h"

float adc_ain1 = 0, adc_ain3 = 0;
u16 para1 = 250, set1 = 250;
u16 para3 = 300, set3 = 300;
u8 s4_flag = 0, s5_flag = 0;
u8 code smg_display[]={                       //标准字库
//   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71,
//black  -     H    J    K    L    N    o   P    U     t    G    Q    r   M    y
    0x00,0x40,0x76,0x1E,0x70,0x38,0x37,0x5C,0x73,0x3E,0x78,0x3d,0x67,0x50,0x37,0x6e};
u8 code smgdot_display[] = {
    0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};    //0. 1. 2. 3. 4. 5. 6. 7. 8. 9. -1


	
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

void sys_init()
{
	hc138_select(4);
	P0 = 0xff;
	hc138_select(5);
	P0 = 0x00;
}

void pcf8591_adc(u8 channel)
{
	float dat;
	
	IIC_Start();
	IIC_SendByte(0x90);
	IIC_WaitAck();
	IIC_SendByte(channel);
	IIC_WaitAck();
	
	IIC_Start();
	IIC_SendByte(0x91);
	IIC_WaitAck();
	dat = IIC_RecByte();
	IIC_SendAck(1);
	IIC_Stop();
	
	if(channel == 0x03)
	{
		adc_ain3 = dat * 500 / 255;
	}
	else if(channel == 0x01)
	{
		adc_ain1 = dat * 500 / 255;
	}
}

void smg_delay(u16 t)
{
	while(t--);
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

void show_ain1()
{
	smgshow_bit(0, ~smg_display[25]);
	smgshow_bit(1, ~smg_display[1]);
	smgshow_bit(5, ~smgdot_display[(int)adc_ain1 / 100]);
	smgshow_bit(6, ~smg_display[(int)adc_ain1 / 10 % 10]);
	smgshow_bit(7, ~smg_display[(int)adc_ain1 % 10]);
}

void show_ain3()
{
	smgshow_bit(0, ~smg_display[25]);
	smgshow_bit(1, ~smg_display[3]);
	smgshow_bit(5, ~smgdot_display[(int)adc_ain3 / 100]);
	smgshow_bit(6, ~smg_display[(int)adc_ain3 / 10 % 10]);
	smgshow_bit(7, ~smg_display[(int)adc_ain3 % 10]);
}

void show_para1()
{
	smgshow_bit(0, ~smg_display[24]);
	smgshow_bit(1, ~smg_display[1]);
	smgshow_bit(5, ~smgdot_display[set1 / 100]);
	smgshow_bit(6, ~smg_display[set1 / 10 % 10]);
	smgshow_bit(7, ~smg_display[set1 % 10]);
}

void show_para3()
{
	smgshow_bit(0, ~smg_display[24]);
	smgshow_bit(1, ~smg_display[3]);
	smgshow_bit(5, ~smgdot_display[set3 / 100]);
	smgshow_bit(6, ~smg_display[set3 / 10 % 10]);
	smgshow_bit(7, ~smg_display[set3 % 10]);
}

void smgshow()
{
	if(s5_flag == 0)       //显示数据
	{
		if(s4_flag == 1)
		{
			show_ain3();
		}
		else if(s4_flag == 0)
		{
			show_ain1();
		}
	}
	else                  //显示参数
	{
		if(s4_flag == 1)
		{
			show_para3();
		}
		else if(s4_flag == 0)
		{
			show_para1();
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
		}
	}
	if(P32 == 0)
	{
		Delay10ms();
		if(P32 == 0)
		{
			while(P32 == 0)
			{
				smgshow();
			}
			s5_flag = !s5_flag;
			para1 = set1;       //退出参数设置界面参数才有效
			para3 = set3;
		}
	}
	if(P31 == 0)
	{
		Delay10ms();
		if(P31 == 0)
		{
			while(P31 == 0)
			{
				smgshow();
			}
			if(s5_flag == 1)     //在参数界面有效
			{
				if((s4_flag == 0) && (set1 < 490)) //通道1
					set1 += 20;
				else if((s4_flag == 1) && (set3 < 500))
					set3 += 20;
			}
		}
	}
	if(P30 == 0)
	{
		Delay10ms();
		if(P30 == 0)
		{
			while(P30 == 0)
			{
				smgshow();
			}
			if(s5_flag == 1)     //在参数界面有效
			{
				if((s4_flag == 0) && (set1 > 10)) //通道1
					set1 -= 20;
				else if((s4_flag == 1) && (set3 > 0))
					set3 -= 20;
			}
		}
	}
}

void led_work()
{
	hc138_select(4);
	if(adc_ain1 > para1)
	{
		P00 = 0;
	}
	else
	{
		P00 = 1;
	}
	if(adc_ain3 > para3)
	{
		P01 = 0;
	}
	else
	{
		P01 = 1;
	}
	if(s4_flag == 0)
	{
		P02 = 0;
	}
	else
	{
		P02 = 1;
	}
		
	if(s5_flag == 0)
	{
		P03 = 0;
	}
	else
	{
		P03 = 1;
	}
	if(adc_ain1 > adc_ain3)
	{
		P04 = 0;
	}
	else
	{
		P04 = 1;
	}
}

void main()
{
	sys_init();
	Delay10ms();
	while(1)
	{
		pcf8591_adc(0x01);
		pcf8591_adc(0x01);
		pcf8591_adc(0x03);
		pcf8591_adc(0x03);
		key_scan();
		smgshow();
		led_work();
	}
}

