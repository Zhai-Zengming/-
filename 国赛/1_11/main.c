/*
 * @brief: 第十一届国赛
 *
 * @data: 2022/6/10
 */
 
#include <STC15F2K60S2.H>
#include "onewire.h"
#include "ds1302.h"  
#include "iic.h"

typedef unsigned char u8;
typedef unsigned int u16;

u8 templ, temph;
u16 temp;
u16 volt;
char settime = 17;  char real_settime = 17;     //real为退出参数界面有效
u8 settemp = 25;    u8 real_settemp = 25;
u8 setled = 4;      u8 real_setled = 4;
u8 s4_flag = 0;
u8 s5_flag = 0;
u8 volt_flag = 0;
u8 pre_voltflag = 0;
u8 t0_cnt = 0;
u8 L3_flag = 0;

u8 code t_display[]={                       //标准字库
//   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71,
//black  -     H    J    K    L    N    o   P    U     t    G    Q    r   M    y
    0x00,0x40,0x76,0x1E,0x70,0x38,0x37,0x5C,0x73,0x3E,0x78,0x3d,0x67,0x50,0x37,0x6e};
u8 code dot_display[]={0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};    //0. 1. 2. 3. 4. 5. 6. 7. 8. 9. -1

u8 wrds1302_addr[] = {0x80, 0x82, 0x84, 0x86, 0x88, 0x8a, 0x8c};
u8 rdds1302_addr[] = {0x81, 0x83, 0x85, 0x87, 0x89, 0x8b, 0x8d};
u8 ds1302_buff[] = {0x50, 0x59, 0x16, 0x10, 0x06, 0x01, 0x22};

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

void delay(u8 t)
{
	while(t--)
		smgshow();
}

void get_temp()
{
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0x44);
	delay(20);
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0xbe);
	
	templ = Read_DS18B20();
	temph = Read_DS18B20();
	temp = temph;
	temp = (temp << 8) | templ;
	if((temp & 0xf800) == 0x0000){
		temp = (temp >> 4) * 10 + (temp & 0x0f) * 0.625;
	}
}

void smgshow_temp()
{
	smgshow_bit(0, ~t_display[12]);
	smgshow_bit(5, ~t_display[temp / 100]);
	smgshow_bit(6, ~dot_display[temp / 10 % 10]);
	smgshow_bit(7, ~t_display[temp % 10]);
}

void smgshow_settemp()
{
	smgshow_bit(0, ~t_display[24]);
	smgshow_bit(1, ~t_display[2]);
	smgshow_bit(6, ~t_display[settemp / 10]);
	smgshow_bit(7, ~t_display[settemp % 10]);
}

void pcf8591_adc(u8 channel)
{
	u16 dat;
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
	
	volt = dat * 1.96;
}

void smgshow_volt()
{
	smgshow_bit(0, ~t_display[14]);
	smgshow_bit(2, ~dot_display[volt / 100]);
	smgshow_bit(3, ~t_display[volt / 10 % 10]);
	smgshow_bit(4, ~t_display[volt % 10]);
	if(volt > 230){
		smgshow_bit(7, ~t_display[0]);
	}
	else{
		smgshow_bit(7, ~t_display[1]);
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
	t0_cnt++;
	if(t0_cnt > 60)
	{
		t0_cnt = 0;
		if(volt_flag == 1){
			L3_flag = 1;
		}else{
			L3_flag = 0;
		}
	}
}

void wr_ds1302()
{
	u8 i;
	Write_Ds1302_Byte(0x8e, 0x00);
	for(i = 0; i < 7; i++){
		Write_Ds1302_Byte(wrds1302_addr[i], ds1302_buff[i]);
	}
	Write_Ds1302_Byte(0x8e, 0x80);
}

void rd_ds1302()
{
	u8 i;
	for(i = 0; i < 7; i++){
		ds1302_buff[i] = Read_Ds1302_Byte(rdds1302_addr[i]);
	}
}

void smgshow_ds1302()
{
	smgshow_bit(0, ~t_display[ds1302_buff[2] / 16]);
	smgshow_bit(1, ~t_display[ds1302_buff[2] % 16]);
	smgshow_bit(2, ~t_display[17]);
	smgshow_bit(3, ~t_display[ds1302_buff[1] / 16]);
	smgshow_bit(4, ~t_display[ds1302_buff[1] % 16]);
	smgshow_bit(5, ~t_display[17]);
	smgshow_bit(6, ~t_display[ds1302_buff[0] / 16]);
	smgshow_bit(7, ~t_display[ds1302_buff[0] % 16]);
}

void smgshow_settime()
{
	smgshow_bit(0, ~t_display[24]);
	smgshow_bit(1, ~t_display[1]);
	smgshow_bit(6, ~t_display[settime / 10]);
	smgshow_bit(7, ~t_display[settime % 10]);
}

void smgshow_setled()
{
	smgshow_bit(0, ~t_display[24]);
	smgshow_bit(1, ~t_display[3]);
	smgshow_bit(7, ~t_display[setled]);
}

void smgshow()
{
	if((s4_flag == 0) && (s5_flag == 0))
		smgshow_ds1302();
	else if((s4_flag == 0) && (s5_flag == 1))
		smgshow_temp();
	else if((s4_flag == 0) && (s5_flag == 2))
		smgshow_volt();
	else if((s4_flag == 1) && (s5_flag == 0))
		smgshow_settime();
	else if((s4_flag == 1) && (s5_flag == 1))
		smgshow_settemp();
	else if((s4_flag == 1) && (s5_flag == 2))
		smgshow_setled();
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
	P30 = 1; P31 = 1; P32 = 0; P33 = 1;
	P34 = 1; P35 = 1; P42 = 1; P44 = 1;
	if(P44 == 0){                        //S5
		Delay10ms();
		if(P44 == 0){
			while(P44 == 0){
				get_temp();
				pcf8591_adc(1);
				rd_ds1302();
				smgshow();
			}
			s5_flag++;
			if(s5_flag == 3)
				s5_flag = 0;
		}
	}if(P42 == 0){                        //S9
		Delay10ms();
		if(P42 == 0){
			while(P42 == 0){
				get_temp();
				pcf8591_adc(1);
				rd_ds1302();
				smgshow();
			}
			if(s4_flag){          //处于参数界面参数加1
				if(s5_flag == 0){
					settime++;
					if(settime > 23)
						settime = 0;
				}else if(s5_flag == 1){
					settemp++;
				}else if(s5_flag == 2){
					setled++;
					if(setled > 8)
						setled = 4;
				}
			}
		}
	}
	P30 = 1; P31 = 1; P32 = 1; P33 = 0;
	P34 = 1; P35 = 1; P42 = 1; P44 = 1;
	if(P44 == 0){                        //S4
		Delay10ms();
		if(P44 == 0){
			while(P44 == 0){
				get_temp();
				pcf8591_adc(1);
				rd_ds1302();
				smgshow();
			}
			s4_flag = !s4_flag;
			s5_flag = 0;              //确保按下S4显示时间参数或时间数据
			if(s4_flag == 0){         //进入数据界面，参数生效
				real_settime = settime;
				real_settemp = settemp;
				real_setled = setled;
			}
		}
	}if(P42 == 0){                        //S8
		Delay10ms();
		if(P42 == 0){
			while(P42 == 0){
				get_temp();
				pcf8591_adc(1);
				rd_ds1302();
				smgshow();
			}
			if(s4_flag){          //处于参数界面参数减1
				if(s5_flag == 0){
					settime--;
					if(settime < 0)
						settime = 23;
				}else if(s5_flag == 1){
					settemp--;
				}else if(s5_flag == 2){
					setled--;
					if(setled < 4)
						setled = 8;
				}
			}
		}
	}
}

void led_work()
{
	u8 hour;
	
	hour = (ds1302_buff[2] / 16) * 10 + (ds1302_buff[2] % 16);  //时
	if(((hour >= real_settime) && (hour < 24)) || ((hour > 0) && (hour < 8))){
		hc138_select(4);
		P0 &= ~(1 << 0);
	}else{
		hc138_select(4);
		P0 |= 1 << 0;
	}
	if(temp < (real_settemp * 10)){
		hc138_select(4);
		P0 &= ~(1 << 1);
	}else{
		hc138_select(4);
		P0 |= (1 << 1);
	}
	if(volt > 230){
		volt_flag = 0;
	}
	else{
		volt_flag = 1;
		hc138_select(4);
		P0 &= ~(1 << (real_setled - 1));
	}
	if(pre_voltflag != volt_flag)
		t0_cnt = 0;
	pre_voltflag = volt_flag;
	if(L3_flag){
		hc138_select(4);
		P0 &= ~(1 << 2);
	}else{
		hc138_select(4);
		P0 |= 1 << 2;
	}
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
	wr_ds1302();
	tim0_init();
	while(1)
	{
		get_temp();
		pcf8591_adc(1);
		rd_ds1302();
		key_scan();
		smgshow();
		led_work();
	}
}