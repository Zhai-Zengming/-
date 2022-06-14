/*
 * @brief : 第八届省赛
 *
 * @date  : 2022/3/18
 */


#include <STC15F2K60S2.H>
#include <absacc.h>
#include "onewire.h"
#include "ds1302.h"

u8 templ, temph;
u16 temp;
u8 ds1302_wraddr[] = {0x80, 0x82, 0x84, 0x86, 0x88, 0x8a,0x8c};
u8 ds1302_rdaddr[] = {0x81, 0x83, 0x85, 0x87, 0x89, 0x8b,0x8d};
char ds1302_buff[] = {0x50, 0x59, 0x23, 0x18, 0x03, 0x05,0x22};
char set_time[3] = {0x23, 0x59, 0x55};
static u8 s7_flag = 1, s6_flag = 0, s5_flag = 0, s4_flag = 0;
u8 stop_rdds1302 = 0;
u8 time_same = 0;
u8 key_down = 0;

u8 code t_display[]={                       //标准字库
//   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71,
//black  -     H    J    K    L    N    o   P    U     t    G    Q    r   M    y
    0x00,0x40,0x76,0x1E,0x70,0x38,0x37,0x5C,0x73,0x3E,0x78,0x3d,0x67,0x50,0x37,0x6e,
    0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};    //0. 1. 2. 3. 4. 5. 6. 7. 8. 9. -1

void smg_show(void);

	
void sys_init(void)
{
	XBYTE[0x8000] = 0xff;
	XBYTE[0xa000] = 0x00;
	XBYTE[0xc000] = 0xff;
	XBYTE[0xe000] = 0xff;
}

void tim0_init(void)
{
	TMOD = 0x01;
	TH0 = (65535 - 50000) / 256;
	TH0 = (65535 - 50000) % 256;
	ET0 = 1;
	TR0 = 1;
	EA = 1;
}

u8 cnt_1s = 0, t0_flag = 0, cnt_200ms = 0, flag_200ms = 0, cnt_5s = 0;
void tim0() interrupt 1
{
	TH0 = (65535 - 50000) / 256;
	TH0 = (65535 - 50000) % 256;
	cnt_1s++;
	cnt_200ms++;
	if(cnt_1s == 20)
	{
		if(time_same == 1)
			cnt_5s++;
		t0_flag++;
		if(cnt_5s == 6)
		{
			cnt_5s = 0;
			time_same = 0;
			key_down = 0;
		}
	}
	if(cnt_1s == 40)
	{
		t0_flag = 0;
		cnt_1s = 0;
	}
	
	if(cnt_200ms == 4)
		flag_200ms++;
	if(cnt_200ms == 8)
	{
		flag_200ms = 0;
		cnt_200ms = 0;
	}
}

void smg_delay(u8 i)
{
	while(i--);
}

void delay(u16 t)
{
	while(t--)
		smg_show();
}

void smgshow_bit(u8 pos, u8 dat)
{
	XBYTE[0xc000] = 1 << pos;
	XBYTE[0xe000] = dat;
	smg_delay(500);
	XBYTE[0xe000] = 0xff;
}

void get_temp(void)
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
	
	temp = (temph << 8) | templ;
	if((temp & 0xf800) == 0)
	{
		temp = temp >> 4;
	}
}

void smgshow_temp(void)
{
	smgshow_bit(5, ~t_display[temp / 10]);
	smgshow_bit(6, ~t_display[temp % 10]);
	smgshow_bit(7, ~t_display[12]);
}

void write_ds1302(void)
{
	int i;
	Write_Ds1302_Byte(0x8e, 0x00);   //turn off the write protect
	for(i = 0; i < 7; i++)
	{
		Write_Ds1302_Byte(ds1302_wraddr[i], ds1302_buff[i]);
	}
	Write_Ds1302_Byte(0x8e, 0x80);
}

void read_ds1302(void)
{
	int i;
	for(i = 0; i < 7; i++)
	{
		ds1302_buff[i] = Read_Ds1302_Byte(ds1302_rdaddr[i]);
	}
}

char sixteen_ten(char dat)
{
	if((dat & 0x0f) == 0x0a)
	{
		dat += 6;
	}
	if((dat & 0x0f) == 0x0f)
	{
		dat -= 6;
	}
	return dat;
}

void smgshow_tim(void)
{
	if(s7_flag == 2)
	{
		if(t0_flag == 1)
		{
			smgshow_bit(0, ~t_display[ds1302_buff[2] / 16]);
			smgshow_bit(1, ~t_display[ds1302_buff[2] % 16]);
		}
		if(t0_flag == 0)
		{
			smgshow_bit(0, 0xff);
			smgshow_bit(1, 0xff);
		}
	}
	else
	{
		smgshow_bit(0, ~t_display[ds1302_buff[2] / 16]);
		smgshow_bit(1, ~t_display[ds1302_buff[2] % 16]);
	}
	
	
	smgshow_bit(2, ~t_display[17]);
	
	
	if(s7_flag == 3)
	{
		if(t0_flag == 1)
		{
			smgshow_bit(3, ~t_display[ds1302_buff[1] / 16]);
			smgshow_bit(4, ~t_display[ds1302_buff[1] % 16]);
		}
		else
		{
			smgshow_bit(3, 0xff);
			smgshow_bit(4, 0xff);
		}
		
	}
	else
	{
		smgshow_bit(3, ~t_display[ds1302_buff[1] / 16]);
		smgshow_bit(4, ~t_display[ds1302_buff[1] % 16]);
	}
	
	
	smgshow_bit(5, ~t_display[17]);

	
	if(s7_flag == 4)
	{
		if(t0_flag == 1)
		{
			smgshow_bit(6, ~t_display[ds1302_buff[0] / 16]);
			smgshow_bit(7, ~t_display[ds1302_buff[0] % 16]);
		}
		else
		{
			smgshow_bit(6, 0xff);
			smgshow_bit(7, 0xff);
		}
	}
	else
	{
		smgshow_bit(6, ~t_display[ds1302_buff[0] / 16]);
		smgshow_bit(7, ~t_display[ds1302_buff[0] % 16]);
	}
}

void smgshow_settim(void)
{
	if(s6_flag == 2)
	{
		if(t0_flag == 1)
		{
			smgshow_bit(0, ~t_display[set_time[0] / 0x10]);
			smgshow_bit(1, ~t_display[set_time[0] % 0x10]);
		}
		if(t0_flag == 0)
		{
			smgshow_bit(0, 0xff);
			smgshow_bit(1, 0xff);
		}
	}
	else
	{
		smgshow_bit(0, ~t_display[set_time[0] / 0x10]);
		smgshow_bit(1, ~t_display[set_time[0] % 0x10]);
	}
	
	smgshow_bit(2, ~t_display[17]);
	
	
	if(s6_flag == 3)
	{	
		if(t0_flag == 1)
		{
			smgshow_bit(3, ~t_display[set_time[1] / 0x10]);
			smgshow_bit(4, ~t_display[set_time[1] % 0x10]);
		}
		if(t0_flag == 0)
		{
			smgshow_bit(3, 0xff);
			smgshow_bit(4, 0xff);
		}
	}
	else
	{
		smgshow_bit(3, ~t_display[set_time[1] / 0x10]);
		smgshow_bit(4, ~t_display[set_time[1] % 0x10]);
	}
	
	smgshow_bit(5, ~t_display[17]);

	
	if(s6_flag == 4)
	{
		if(t0_flag == 1)
		{
			smgshow_bit(6, ~t_display[set_time[2] / 0x10]);
			smgshow_bit(7, ~t_display[set_time[2] % 0x10]);
		}
		if(t0_flag == 0)
		{
			smgshow_bit(6, 0xff);
			smgshow_bit(7, 0xff);
		}
	}
	else
	{
		smgshow_bit(6, ~t_display[set_time[2] / 0x10]);
		smgshow_bit(7, ~t_display[set_time[2] % 0x10]);
	}
}

void smg_show(void)
{
	if(s7_flag == 1)
	{
		while(P33 == 0)
		{
			smgshow_temp();
		}
	}
	if(s7_flag && (s6_flag == 0))	
		smgshow_tim();
	if(s6_flag)
		smgshow_settim();
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
				smg_show();
			}
			if(s7_flag == 0)
				stop_rdds1302 = 0;     //由s6切换回来后要正常开始读
			
			s6_flag = 0;
			s7_flag += 1;
			if(s7_flag == 2)      //进入闪烁状态
			{
				stop_rdds1302 = 1;
			}
			
			if(s7_flag == 5)
			{
				stop_rdds1302 = 0;
				s7_flag = 1;
				write_ds1302();
			}
		}
	}
	
	if(P31 == 0)
	{
		Delay10ms();
		if(P31 == 0)
		{
			while(P31 == 0)
			{
				smg_show();
			}
			s7_flag = 0;
			s6_flag++;
			if(s6_flag == 5)
			{
				s6_flag = 1;
			}
		}
	}
	
	if(P32 == 0)
	{
		Delay10ms();
		if((P32 == 0) && ((s7_flag != 0) || (s6_flag != 0)))
		{
			while(P32 == 0)
			{
				smg_show();
			}
			if(s7_flag == 2)
			{
				ds1302_buff[2]++;
				ds1302_buff[2] = sixteen_ten(ds1302_buff[2]);
				
				if(ds1302_buff[2] > 0x23)
					ds1302_buff[2] = 0;
				
			}
			else if(s7_flag == 3)
			{
				ds1302_buff[1]++;
				ds1302_buff[1] = sixteen_ten(ds1302_buff[1]);
				if(ds1302_buff[1] > 0x59)
					ds1302_buff[1] = 0;
			}
			else if(s7_flag == 4)
			{
				ds1302_buff[0]++;
				ds1302_buff[0] = sixteen_ten(ds1302_buff[0]);
				if(ds1302_buff[0] > 0x59)
					ds1302_buff[0] = 0;
			}
			
			if(s6_flag == 2)
			{
				set_time[0]++;
				set_time[0] = sixteen_ten(set_time[0]);
				if(set_time[0] > 0x23)
					set_time[0] = 0;
			}
			else if(s6_flag == 3)
			{
				set_time[1]++;
				set_time[1] = sixteen_ten(set_time[1]);
				if(set_time[1] > 0x59)
					set_time[1] = 0;
			}
			else if(s6_flag == 4)
			{
				set_time[2]++;
				set_time[2] = sixteen_ten(set_time[2]);
				if(set_time[2] > 0x59)
					set_time[2] = 0;
			}
		}
	}
	
	if(P33 == 0)
	{
		Delay10ms();
		if((P33 == 0) && ((s7_flag != 0) || (s6_flag != 0)))
		{
			while(P33 == 0)
			{
				smg_show();
			}
			if(s7_flag == 2)
			{
				ds1302_buff[2]--;
				ds1302_buff[2] = sixteen_ten(ds1302_buff[2]);
				if(ds1302_buff[2] < 0)
					ds1302_buff[2] = 0x23;
			}
			else if(s7_flag == 3)
			{
				ds1302_buff[1]--;
				ds1302_buff[1] = sixteen_ten(ds1302_buff[1]);
				if(ds1302_buff[1] < 0)
					ds1302_buff[1] = 0x59;
			}
			else if(s7_flag == 4)
			{
				ds1302_buff[0]--;
				ds1302_buff[0] = sixteen_ten(ds1302_buff[0]);
				if(ds1302_buff[0] < 0)
					ds1302_buff[0] = 0x59;
			}
			
			if(s6_flag == 2)
			{
				set_time[0]--;
				set_time[0] = sixteen_ten(set_time[0]);
				if(set_time[0] < 0)
					set_time[0] = 0x23;
			}
			else if(s6_flag == 3)
			{
				set_time[1]--;
				set_time[1] = sixteen_ten(set_time[1]);
				if(set_time[1] < 0)
					set_time[1] = 0x59;
			}
			else if(s6_flag == 4)
			{
				set_time[2]--;
				set_time[2] = sixteen_ten(set_time[2]);
				if(set_time[2] < 0)
					set_time[2] = 0x59;
			}
		}
	}
}

void clock(void)
{
	if((set_time[2] == ds1302_buff[0]) && (set_time[1] == ds1302_buff[1]) && (set_time[0] == ds1302_buff[2]))
	{
		time_same = 1;
	}
	if((time_same == 1) && (key_down == 0))
	{
		if(flag_200ms == 1)
		{
			XBYTE[0x8000] = 0xff;
		}
		else
			XBYTE[0x8000] = 0xfe;
		if((P30 == 0) | (P31 == 0) | (P32 == 0) | (P33 == 0))
		{
			key_down = 1;
			XBYTE[0x8000] = 0xff;
		}
	}
}

void main()
{
	sys_init();
	write_ds1302();
	tim0_init();
	while(1)
	{
		key_scan();
		get_temp();
		if(stop_rdds1302 == 0)
		{
			read_ds1302();
		}
		
		smg_show();
		clock();
	}
}