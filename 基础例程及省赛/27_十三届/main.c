/*
 * 2022/5/9
 */

#include <STC15F2K60S2.H>
#include "ds1302.h"  									
#include "onewire.h"

u8 templ, temph;
u16 temp;
u8 settemp = 23;
u8 s12_flag = 0, s13_flag = 0;
u8 flag_5s = 0;
u8 cnt = 0;
u8 int_time = 0, relay_close = 0;
u8 t1_cnt = 0, t1_flag_5s = 0;
u8 L3_flag = 0, L3_cnt = 0;
u8 ds1302_rdaddr[] = {0x81, 0x83, 0x85, 0x87, 0x89, 0x8b, 0x8d};
u8 ds1302_wraddr[] = {0x80, 0x82, 0x84, 0x86, 0x88, 0x8a, 0x8c};
u8 ds1302_buff[] =   {0x50, 0x59, 0x23, 0x09, 0x05, 0x01, 0x22};

u8 code t_display[]={                       //标准字库
//   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71,
//black  -     H    J    K    L    N    o   P    U     t    G    Q    r   M    y
    0x00,0x40,0x76,0x1E,0x70,0x38,0x37,0x5C,0x73,0x3E,0x78,0x3d,0x67,0x50,0x37,0x6e};

u8 code dot_display[] = {0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};    //0. 1. 2. 3. 4. 5. 6. 7. 8. 9. -1

void smgshow();
void relay_work();
void led_work();

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

void smg_delay()
{
	u8 t = 100;
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

void delay()
{
	u16 t = 30;
	while(t--)
	{
		smgshow();
	}
}

void get_temp()
{
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0x44);
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0xbe);
	delay();
	
	templ = Read_DS18B20();
	temph = Read_DS18B20();
	temp = temph;
	temp = (temp << 8) | templ;
	if((temp & 0xf800) == 0x0000)
	{
		temp = (temp >> 4) * 10 + (temp & 0x000f) * 0.625;
	}
}

void get_temp_init()
{
	do{
		init_ds18b20();
		Write_DS18B20(0xcc);
		Write_DS18B20(0x44);
		init_ds18b20();
		Write_DS18B20(0xcc);
		Write_DS18B20(0xbe);
//		delay();
		
		templ = Read_DS18B20();
		temph = Read_DS18B20();
		temp = temph;
		temp = (temp << 8) | templ;
		if((temp & 0xf800) == 0x0000)
		{
			temp = (temp >> 4) * 10 + (temp & 0x000f) * 0.625;
		}
	}while(temp == 850);
}

void smgshow_temp()
{
	smgshow_bit(0, ~t_display[25]);  //U
	smgshow_bit(1, ~t_display[1]);
	
	smgshow_bit(5, ~t_display[temp / 100]);
	smgshow_bit(6, ~dot_display[temp / 10 % 10]);
	smgshow_bit(7, ~t_display[temp % 10]);
}

void smgshow_settemp()
{
	smgshow_bit(0, ~t_display[25]);  //U
	smgshow_bit(1, ~t_display[3]);
	
	smgshow_bit(6, ~t_display[settemp / 10]);
	smgshow_bit(7, ~t_display[settemp % 10]);
}

void write_ds1302()
{
	u8 i;
	
	Write_Ds1302_Byte(0x8e, 0x00);
	for(i = 0; i < 7; i++){
		Write_Ds1302_Byte(ds1302_wraddr[i], ds1302_buff[i]);
	}
	Write_Ds1302_Byte(0x8e, 0x80);
}

void read_ds1302()
{
	u8 i;
	
	for(i = 0; i < 7; i++){
		ds1302_buff[i] = Read_Ds1302_Byte(ds1302_rdaddr[i]);
	}
}

void smgshow_time()
{
	smgshow_bit(0, ~t_display[25]);  //U
	smgshow_bit(1, ~t_display[2]);
	
	smgshow_bit(3, ~t_display[ds1302_buff[2] / 16]);
	smgshow_bit(4, ~t_display[ds1302_buff[2] % 16]);
	smgshow_bit(5, ~t_display[17]);
	smgshow_bit(6, ~t_display[ds1302_buff[1] / 16]);
	smgshow_bit(7, ~t_display[ds1302_buff[1] % 16]);
}

void smgshow_min_sec()
{
	smgshow_bit(0, ~t_display[25]);  //U
	smgshow_bit(1, ~t_display[2]);
	
	smgshow_bit(3, ~t_display[ds1302_buff[1] / 16]);
	smgshow_bit(4, ~t_display[ds1302_buff[1] % 16]);
	smgshow_bit(5, ~t_display[17]);
	smgshow_bit(6, ~t_display[ds1302_buff[0] / 16]);
	smgshow_bit(7, ~t_display[ds1302_buff[0] % 16]);
}

void smgshow()
{
	if(s12_flag == 0)
		smgshow_temp();
	if(s12_flag == 1)
		smgshow_time();
	if(s12_flag == 2)
		smgshow_settemp();
}

void Delay10ms()		//@12.000MHz
{
	unsigned char i, j;

	i = 117;
	j = 184;
	do
	{
		while (--j);
	} while (--i);
}


void key_scan()
{
	P30 = 1; P31 = 1; P32 = 1; P33 = 0;
	P34 = 1; P35 = 1; P42 = 1; P44 = 1;
	if((P34 == 0) && (s12_flag == 2)){       //参数增加
		Delay10ms();
		if(P34 == 0){
			while(P34 == 0){
				smgshow();
			}
			if(settemp < 99)
				settemp++;
		}
	}
	if(P35 == 0){                           //界面转换
		Delay10ms();
		if(P35 == 0){
			while(P35 == 0){
				smgshow();
			}
			s12_flag++;
			if(s12_flag == 3)
				s12_flag = 0;
		}
	}
	P30 = 1; P31 = 1; P32 = 0; P33 = 1;
	P34 = 1; P35 = 1; P42 = 1; P44 = 1;
	if(P35 == 0){                           //控制转换
		Delay10ms();
		if(P35 == 0){
			while(P35 == 0){
				smgshow();
			}
			s13_flag = !s13_flag;
			hc138_select(5);
			P0 = 0x00;                       //切换控制后先断开继电器
			relay_close = 0;
		}
	}
	if((P34 == 0) && (s12_flag == 2)){       //参数减少
		Delay10ms();
		if(P34 == 0){
			while(P34 == 0){
				smgshow();
			}
			if(settemp > 10)
				settemp--;
		}
	}
	if((P34 == 0) && (s12_flag == 1)){       //显示分秒
		Delay10ms();
		if(P34 == 0){
			while(P34 == 0){
				read_ds1302();
				relay_work();
				led_work();
				smgshow_min_sec();
			}
		}
	}
}

void relay_work()
{
	if(s13_flag == 0){            //默认温度控制
		if(temp > settemp * 10){
			relay_close = 1;
			hc138_select(5);
			P04 = 1;
			P06 = 0;
		}else{
			relay_close = 0;
			hc138_select(5);
			P04 = 0;
			P06 = 0;
		}
	}else{                         //时间控制
		if((ds1302_buff[0] == 0) && (ds1302_buff[1] == 0)){
			TR0 = 1;
			relay_close = 1;
			hc138_select(5);
			P04 = 1;
			P06 = 0;
		}
		if(flag_5s)
		{
			flag_5s = 0;
			hc138_select(5);
			relay_close = 0;
			P04 = 0;
			P06 = 0;
		}
	}
}

void led_work()
{
	if((ds1302_buff[0] == 0) && (ds1302_buff[1] == 0)){   //L1
		int_time = 1;
	}
	if(int_time){
		hc138_select(4);
		P00 = 0; P03 = 1; P04 = 1;
		P05 = 1; P06 = 1; P07 = 1;
	}
	if(t1_flag_5s){
		int_time = 0;
		t1_flag_5s = 0;
		hc138_select(4);
		P00 = 1; P03 = 1; P04 = 1;
		P05 = 1; P06 = 1; P07 = 1;
	}
	
	if(s13_flag == 0){                       //L2
		hc138_select(4);
		P01 = 0; P03 = 1; P04 = 1;
		P05 = 1; P06 = 1; P07 = 1;
	}else{
		hc138_select(4);
		P01 = 1; P03 = 1; P04 = 1;
		P05 = 1; P06 = 1; P07 = 1;
	}
	
	if(relay_close){                   //L3
		if(L3_flag){
			hc138_select(4);
			P02 = 1; P03 = 1; P04 = 1;
			P05 = 1; P06 = 1; P07 = 1;
		}else{
			hc138_select(4);
			P02 = 0; P03 = 1; P04 = 1;
			P05 = 1; P06 = 1; P07 = 1;
		}
	}
	
}

void tim_init()
{
	TMOD = 0x11;
	TH0 = (65535 - 50000) / 256;
	TL0 = (65535 - 50000) % 256;
	TH1 = (65535 - 50000) / 256;
	TL1 = (65535 - 50000) % 256;
	ET0 = 1;
	TR0 = 0;
	ET1 = 1;
	TR1 = 1;
	EA = 1;
}


void sys_init()
{
	hc138_select(4);
	P0 = 0xff;
	hc138_select(5);
	P0 = 0x00;
}

void main()
{
	sys_init();
	get_temp_init();
	tim_init();
	write_ds1302();
	while(1)
	{
		key_scan();
		get_temp();
		read_ds1302();
		smgshow();
		relay_work();
		led_work();
	}
}


void tim0() interrupt 1
{
	cnt++;
	TH0 = (65535 - 50000) / 256;
	TL0 = (65535 - 50000) % 256;
	if(cnt == 100)
	{
		cnt = 0;
		flag_5s = 1;
	}
}

void tim1() interrupt 3
{
	if(int_time){
		t1_cnt++;
	}
	
	L3_cnt++;
	TH1 = (65535 - 50000) / 256;
	TL1 = (65535 - 50000) % 256;
	if(t1_cnt == 100)
	{
		t1_cnt = 0;
		t1_flag_5s = 1;
	}
	if(L3_cnt == 2)
	{
		L3_cnt = 0;
		L3_flag = !L3_flag;
	}
}
