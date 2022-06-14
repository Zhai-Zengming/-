/*
 * @brief : 第七届省赛
 *
 * @date  : 2022/3/20
 *
 */


#include <STC15F2K60S2.H>
#include <absacc.h>
#include <onewire.h>

u8 temph, templ;
u16 temp;
u8 pwm_cnt = 0;
u16 cnt_1s = 0;
u8 duty = 2;
int set_time[3] = {50, 50, 50};
u8 s4_flag = 0, s5_flag = 0, s7_flag = 0;

u8 code t_display[]={                       //标准字库
//   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71,
//black  -     H    J    K    L    N    o   P    U     t    G    Q    r   M    y
    0x00,0x40,0x76,0x1E,0x70,0x38,0x37,0x5C,0x73,0x3E,0x78,0x3d,0x67,0x50,0x37,0x6e,
    0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};    //0. 1. 2. 3. 4. 5. 6. 7. 8. 9. -1

void smgshow(void);

	
void sys_init(void)
{
	XBYTE[0x8000] = 0xff;
	XBYTE[0xa000] = 0x00;
	XBYTE[0xc000] = 0xff;
	XBYTE[0xe000] = 0xff;
}

void delay(u16 dat)
{
	while(dat--)
	{
		smgshow();
	}
}

void smg_delay(u16 dat)
{
	while(dat--);
}
void smgshow_bit(u8 pos, u8 dat)
{
	XBYTE[0xc000] = 1 << pos;
	XBYTE[0xe000] = dat;
	smg_delay(100);
	XBYTE[0xe000] = 0xff;
}

void get_temp(void)
{
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0x44);
	delay(5);
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

void smgshow_temp(void)
{
	smgshow_bit(0, ~t_display[17]);
	smgshow_bit(1, ~t_display[4]);
	smgshow_bit(2, ~t_display[17]);
	
	smgshow_bit(5, ~t_display[temp / 10]);
	smgshow_bit(6, ~t_display[temp % 10]);
	smgshow_bit(7, ~t_display[12]);
}

void tim0_init(void)
{
	TMOD = 0x01;
	TH0 = (65535 - 100) / 256;
	TL0 = (65535 - 100) % 256;
	ET0 = 1;
	TR0 = 1;
	EA = 1;
}

void tim0() interrupt 1
{
	TH0 = (65535 - 100) / 256;
	TL0 = (65535 - 100) % 256;
	pwm_cnt++;
	cnt_1s++;
	if(cnt_1s == 10000)
	{
		cnt_1s = 0;
		if(set_time[s4_flag] > 0)
		{
			set_time[s4_flag]--;
			if(set_time[s4_flag] == 0)
			{
				duty = 0;
			}
		}
	}
	if(pwm_cnt == 10)
		pwm_cnt = 0;
	if(pwm_cnt < duty)
	{
		XBYTE[0xa000] = 0x40;   //用蜂鸣器代替pwm驱动电机
//		P34 = 1;
	}
	if(pwm_cnt > duty)
	{
//		P34 = 0;
		XBYTE[0xa000] = 0x00;
	}
}

void smg_workmode(void)
{
	if(s4_flag == 0)
	{
		smgshow_bit(0, ~t_display[17]);             //模式1
		smgshow_bit(1, ~t_display[1]);
		smgshow_bit(2, ~t_display[17]);
		smgshow_bit(4, ~t_display[set_time[0] / 1000]);
		smgshow_bit(5, ~t_display[set_time[0] / 100 % 10]);
		smgshow_bit(6, ~t_display[set_time[0] / 10 % 10]);
		smgshow_bit(7, ~t_display[set_time[0] % 10]);
	}
	
	if(s4_flag == 1)
	{
		smgshow_bit(0, ~t_display[17]);             //模式2
		smgshow_bit(1, ~t_display[2]);
		smgshow_bit(2, ~t_display[17]);
		smgshow_bit(4, ~t_display[set_time[1] / 1000]);
		smgshow_bit(5, ~t_display[set_time[1] / 100 % 10]);
		smgshow_bit(6, ~t_display[set_time[1] / 10 % 10]);
		smgshow_bit(7, ~t_display[set_time[1] % 10]);
	}
	
	if(s4_flag == 2)
	{
		smgshow_bit(0, ~t_display[17]);             //模式3
		smgshow_bit(1, ~t_display[3]);
		smgshow_bit(2, ~t_display[17]);
		smgshow_bit(4, ~t_display[set_time[2] / 1000]);
		smgshow_bit(5, ~t_display[set_time[2] / 100 % 10]);
		smgshow_bit(6, ~t_display[set_time[2] / 10 % 10]);
		smgshow_bit(7, ~t_display[set_time[2] % 10]);
	}
}

void smgshow(void)
{
	if(s7_flag == 1)
	{
		smgshow_temp();
	}
	if(s7_flag == 0)
	{
		smg_workmode();
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

void key_scan(void)
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
				duty = 3;
				
			}
			else if(s4_flag == 2)
			{
				duty = 7;
				
			}
			else if(s4_flag == 3)
			{
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
			if(s5_flag == 1)
				set_time[s4_flag] += 60;
			if(s5_flag == 2)
				set_time[s4_flag] += 60;
			if(s5_flag == 3)
			{
				s5_flag = 0;
				set_time[s4_flag] -= 120;
				if(set_time[s4_flag] < 0)
					set_time[s4_flag] = 0;
			}
			
			if(duty == 0)    //占空比清零后重新设置
			{
				if(s4_flag == 1)
					duty = 3;
				if(s4_flag == 2)
					duty = 7;
				if(s4_flag == 3)
					s4_flag = 0;
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
				smgshow();
			}
			duty = 0;
			set_time[s4_flag] = 0;
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
			s7_flag = !s7_flag;
		}
	}
}

void led_work(void)
{
	if((P31 != 0) && (set_time[s4_flag] > 0))
	{
		if(s4_flag == 0)
		{
			XBYTE[0x8000] = 0xfe;
		}
		else if(s4_flag == 1)
		{
			XBYTE[0x8000] = 0xfd;
		}
		else if(s4_flag == 2)
		{
			XBYTE[0x8000] = 0xfb;
		}
	}
	else
		XBYTE[0x8000] = 0xff;
}

void main()
{
	sys_init();
	tim0_init();
	XBYTE[0x8000] = 0xfe;
	while(1)
	{
		get_temp();
		key_scan();
		smgshow();
		led_work();
	}
}



