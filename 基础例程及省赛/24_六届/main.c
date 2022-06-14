/*
 * @brief : 蓝桥杯省赛第六届
 *
 * @date  : 2022/3/22
 *
 */

#include <STC15F2K60S2.H>
#include "ds1302.h"
#include "onewire.h"

u8 templ, temph;
u16 temp;
u8 cnt_1s = 0, flag_1s = 0, indiv_cnt = 0, indiv_time = 1, indiv_flag = 0;
u8 s4_flag = 0, s5_flag = 0, s6_flag = 0, s7_flag = 0;
u8 s6_state = 0;
u8 tempera[10] = {0};       //采集到的十个温度
u8 temp_i = 0;
u8 temp_finish = 0;
u8 led_stop = 0;
u8 wr_ds1302addr[] = {0x80, 0x82, 0x84, 0x86, 0x88, 0x8a, 0x8c};
u8 rd_ds1302addr[] = {0x81, 0x83, 0x85, 0x87, 0x89, 0x8b, 0x8d};
u8 ds1302_buff[] = {0x50, 0x59, 0x23};

u8 code t_display[]={                       //标准字库
//   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71,
//black  -     H    J    K    L    N    o   P    U     t    G    Q    r   M    y
    0x00,0x40,0x76,0x1E,0x70,0x38,0x37,0x5C,0x73,0x3E,0x78,0x3d,0x67,0x50,0x37,0x6e,
    0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};    //0. 1. 2. 3. 4. 5. 6. 7. 8. 9. -1


void smgshow(void);
void read_ds1302();

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
	
void delay(u16 dat)
{
	while(dat--)
		smgshow();
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
	smg_delay(500);
	P0 = 0xff;
}

void smg_setpara()
{
	smgshow_bit(0, 0xff);
	smgshow_bit(1, 0xff);
	smgshow_bit(2, 0xff);
	smgshow_bit(3, 0xff);
	smgshow_bit(4, 0xff);
	
	smgshow_bit(5, ~t_display[17]);
	if(s4_flag == 0)
	{
		smgshow_bit(6, ~t_display[0]);
		smgshow_bit(7, ~t_display[1]);
	}
	else if(s4_flag == 1)
	{
		smgshow_bit(6, ~t_display[0]);
		smgshow_bit(7, ~t_display[5]);
	}
	else if(s4_flag == 2)
	{
		smgshow_bit(6, ~t_display[3]);
		smgshow_bit(7, ~t_display[0]);
	}
	else if(s4_flag == 3)
	{
		smgshow_bit(6, ~t_display[6]);
		smgshow_bit(7, ~t_display[0]);
	}
}

void smgshow_time()
{
	smgshow_bit(0, ~t_display[ds1302_buff[2] / 16]);
	smgshow_bit(1, ~t_display[ds1302_buff[2] % 16]);
	
	if(flag_1s == 0)
	{
		smgshow_bit(2, 0xff);
		smgshow_bit(5, 0xff);
	}
	else if(flag_1s == 1)
	{
		smgshow_bit(2, ~t_display[17]);
		smgshow_bit(5, ~t_display[17]);
	}
	
	smgshow_bit(3, ~t_display[ds1302_buff[1] / 16]);
	smgshow_bit(4, ~t_display[ds1302_buff[1] % 16]);
	
	smgshow_bit(6, ~t_display[ds1302_buff[0] / 16]);
	smgshow_bit(7, ~t_display[ds1302_buff[0] % 16]);
}

void smgshow_temp()
{
	smgshow_bit(0, ~t_display[17]);
	
	smgshow_bit(1, ~t_display[0]);
	smgshow_bit(2, ~t_display[s6_flag]);
	
	smgshow_bit(5, ~t_display[17]);
	
	smgshow_bit(6, ~t_display[tempera[s6_flag] / 10]);
	smgshow_bit(7, ~t_display[tempera[s6_flag] % 10]);
}

void smgshow()
{
	if(((s5_flag == 0) && (s6_state == 0)) || (s7_flag))
	{
		smg_setpara();
	}
	
	if((s5_flag) && (!temp_finish))
	{
		smgshow_time();
		
	}
	if((temp_finish == 1) && (s7_flag == 0))
	{
		smgshow_temp();
	}
}

void tim0_init()
{
	TMOD = 0x01;
	TH0 = (65535 - 50000) / 256;
	TL0 = (65535 - 50000) % 256;
	ET0 = 1;
	TR0 = 1;
	EA = 1;
}

void tim0() interrupt 1
{
	TH0 = (65535 - 50000) / 256;
	TL0 = (65535 - 50000) % 256;
	cnt_1s++;
	if(cnt_1s == 20)
	{
		cnt_1s = 0;
		flag_1s = !flag_1s;
		if((temp_finish == 0) && (s5_flag))
		{
			indiv_cnt++;
			if(indiv_cnt == indiv_time)
			{
				indiv_cnt = 0;
				indiv_flag = 1;
			}
		}
	}
}

void get_temp()
{
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0x44);
	delay(100);
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

void s5_work()
{
	if(s5_flag)
	{
		if(indiv_flag)
		{
			indiv_flag = 0;
			get_temp();
			tempera[temp_i] = temp;
			temp_i++;
			if(temp_i == 10)
			{
				temp_i = 0;
				temp_finish = 1;
			}
		}
	}
}

void led_work()
{
	if(temp_finish && (led_stop == 0))
	{
		if(flag_1s == 0)
		{
			hc138_select(4);
			P0 = 0xff;
		}
		else
		{
			hc138_select(4);
			P0 = 0xfe;
		}
	}
	else if(led_stop)
	{
		hc138_select(4);
		P0 = 0xff;
	}
}

void write_ds1302()
{
	int i;
	Write_Ds1302_Byte(0x8e, 0x00);
	for(i = 0; i < 3; i++)
	{
		Write_Ds1302_Byte(wr_ds1302addr[i], ds1302_buff[i]);
	}
	Write_Ds1302_Byte(0x8e, 0x80);
}

void read_ds1302()
{
	int i;
	for(i = 0; i < 3; i++)
	{
		ds1302_buff[i] = Read_Ds1302_Byte(rd_ds1302addr[i]);
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
			s4_flag++;
			
			if(s4_flag == 1)
			{
				indiv_time = 5;
			}
			else if(s4_flag == 2)
			{
				indiv_time = 30;
			}
			else if(s4_flag == 3)
			{
				indiv_time = 60;
			}
			if(s4_flag == 4)
			{
				indiv_time = 1;
				s4_flag = 0;
			}
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
			
			s5_flag++;		
			s4_flag = 0;
			
			temp_finish = 0;
			led_stop = 0;
			s7_flag = 0;
		}
	}
	
	if((P31 == 0) && (temp_finish == 1))
	{
		Delay10ms();
		if(P31 == 0)
		{
			while(P31 == 0)
			{
				smgshow();
			}
			s6_state = 1;
			s6_flag++;
			s5_flag = 0;
			led_stop = 1;
			
			if(s6_flag == 10)
			{
				s6_flag = 0;
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
			s7_flag++;
			s6_flag = 0;
			s6_state = 0;
		}
	}
}

void main()
{
	tim0_init();
	sys_init();
	get_temp();
	write_ds1302();
	while(1)
	{
		read_ds1302();
		key_scan();
		s5_work();
		led_work();
		smgshow();
	}
}

