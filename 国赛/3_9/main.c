/*
 * 2022/6/11
 */

#include <STC15F2K60S2.H>
#include "iic.h"
#include "onewire.h"

typedef unsigned char u8;
typedef unsigned int u16;

u8 templ, temph;
u16 temp; u16 pre_temp;
u16 t0_cnt = 0;
u16 t1_cnt = 0;
u16 pulse; u16 pre_pulse;
u8 volt; u8 pre_volt;
u8 setvolt = 10;  //扩大了十倍
u8 s4_flag = 0;
u8 s6_flag = 0;
bit showdat_flag = 1;     //显示数据
bit showback_flag = 0;     //显示回显
u8 s7_flag = 0;
bit flag_1s = 0;
u8 cnt_200ms = 0;
bit flag_200ms = 0;

u8 code t_display[]={                       //标准字库
//   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71,
//black  -     H    J    K    L    N    o   P    U     t    G    Q    r   M    y
    0x00,0x40,0x76,0x1E,0x70,0x38,0x37,0x5C,0x73,0x3E,0x78,0x3d,0x67,0x50,0x37,0x6e};
u8 code dot_display[]={0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};    //0. 1. 2. 3. 4. 5. 6. 7. 8. 9. -1

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
	while(t--){
		smgshow();
	}
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
		temp = (temp >> 4) * 100 + (temp & 0x0f) * 6.25;
	}
}
void smgshow_temp()
{
	smgshow_bit(0, ~t_display[12]);
	smgshow_bit(4, ~t_display[temp / 1000]);
	smgshow_bit(5, ~dot_display[temp / 100 % 10]);
	smgshow_bit(6, ~t_display[temp / 10 % 10]);
	smgshow_bit(7, ~t_display[temp % 10]);
}
void smg_callbacktemp()
{
	smgshow_bit(0, ~t_display[18]);
	smgshow_bit(1, ~t_display[12]);
	smgshow_bit(4, ~t_display[pre_temp / 1000]);
	smgshow_bit(5, ~dot_display[pre_temp / 100 % 10]);
	smgshow_bit(6, ~t_display[pre_temp / 10 % 10]);
	smgshow_bit(7, ~t_display[pre_temp % 10]);
}

void tim_init()
{
	TMOD = 0x16;
	TH1 = (65535 - 50000) / 256;
	TL1 = (65535 - 50000) % 256;
	TH0 = 0xff;
	TL0 = 0xff;
	ET1 = 1;
	ET0 = 1;
	TR0 = 1;
	TR1 = 1;
	EA = 1;
}
void tim0() interrupt 1
{
	t0_cnt++;
}
void tim1() interrupt 3
{
	TH1 = (65535 - 50000) / 256;
	TL1 = (65535 - 50000) % 256;
	t1_cnt++;
	cnt_200ms++;
	if(cnt_200ms == 4){
		cnt_200ms = 0;
		flag_200ms = !flag_200ms;
	}
	if(t1_cnt == 20){
		flag_1s = 1;
		t1_cnt = 0;
		pulse = t0_cnt;
		t0_cnt = 0;
	}
}
void smgshow_pulse()
{
	smgshow_bit(0, ~t_display[15]);
	if(pulse > 9999)
		smgshow_bit(3, ~t_display[pulse / 10000]);
	if(pulse > 999)
		smgshow_bit(4, ~t_display[pulse / 1000 % 10]);
	if(pulse > 99)
		smgshow_bit(5, ~t_display[pulse / 100 % 10]);
	if(pulse > 9)
		smgshow_bit(6, ~t_display[pulse / 10 % 10]);
	smgshow_bit(7, ~t_display[pulse % 10]);
}
void smg_callbackpulse()
{
	smgshow_bit(0, ~t_display[18]);
	smgshow_bit(1, ~t_display[15]);
	if(pre_pulse > 9999)
		smgshow_bit(3, ~t_display[pre_pulse / 10000]);
	if(pre_pulse > 999)
		smgshow_bit(4, ~t_display[pre_pulse / 1000 % 10]);
	if(pre_pulse > 99)
		smgshow_bit(5, ~t_display[pre_pulse / 100 % 10]);
	if(pre_pulse > 9)
		smgshow_bit(6, ~t_display[pre_pulse / 10 % 10]);
	smgshow_bit(7, ~t_display[pre_pulse % 10]);
}

void pcf8591_adc(u8 channel)
{
	u8 dat;
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
	volt = dat * 0.1961;
	if(volt > setvolt){
		hc138_select(4);
		if(flag_200ms)
			P07 = 0;
		else
			P07 = 1;
	}
}
void smgshow_volt()
{
	smgshow_bit(0, ~t_display[25]);
	smgshow_bit(6, ~dot_display[volt / 10]);
	smgshow_bit(7, ~t_display[volt % 10]);
}
void smgshow_setvolt()
{
	smgshow_bit(0, ~t_display[24]);
	smgshow_bit(6, ~dot_display[setvolt / 10]);
	smgshow_bit(7, ~t_display[setvolt % 10]);
}
void smg_callbackvolt()
{
	smgshow_bit(0, ~t_display[18]);
	smgshow_bit(1, ~t_display[25]);
	smgshow_bit(6, ~dot_display[pre_volt / 10]);
	smgshow_bit(7, ~t_display[pre_volt % 10]);
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
u8 rd_eeprom(u8 addr)
{
	u8 dat;
	IIC_Start();
	IIC_SendByte(0xa0);
	IIC_WaitAck();
	IIC_SendByte(addr);
	IIC_WaitAck();
	
	IIC_Start();
	IIC_SendByte(0xa1);
	IIC_WaitAck();
	dat = IIC_RecByte();
	IIC_SendAck(1);
	IIC_Stop();
	return dat;
}
void wr_eeprom(u8 addr, u8 dat)
{
	IIC_Start();
	IIC_SendByte(0xa0);
	IIC_WaitAck();
	IIC_SendByte(addr);
	IIC_WaitAck();
	IIC_SendByte(dat);
	IIC_WaitAck();
	IIC_Stop();
}
void rd_eepdat()
{
	u8 tempbuffl;
	u16 tempbuffh;
	u8 pulsebuffl;
	u16 pulsebuffh;
	
	tempbuffh = rd_eeprom(1);
	tempbuffl = rd_eeprom(2);
	pre_temp = (tempbuffh << 8) | tempbuffl;
	Delay5ms();
	pre_volt = rd_eeprom(3);
	pulsebuffh = rd_eeprom(4);
	pulsebuffl = rd_eeprom(5);
	pre_pulse = (pulsebuffh << 8) | pulsebuffl;
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
	u16 i = 0;
	if(s7_flag == 0){
		if(P33 == 0){             //S4
			Delay10ms();
			if(P33 == 0){
				while(P33 == 0){
					smgshow();
				}
				showdat_flag = 1;
				showback_flag = 0;
				s4_flag++;
				if(s4_flag > 2)
					s4_flag = 0;
			}
		}
		if(P32 == 0){             //S5
			Delay10ms();
			if(P32 == 0){
				while(P32 == 0){
					smgshow();
				}
				wr_eeprom(1, (temp >> 8));      //存储温度高八位
				Delay5ms();
				wr_eeprom(2, (temp & 0x00ff));  //存储温度低八位
				Delay5ms();
				wr_eeprom(3, volt);             //存储电压
				Delay5ms();
				wr_eeprom(4, (pulse >> 8));     //存储频率高八位
				Delay5ms();
				wr_eeprom(5, (pulse & 0x00ff)); //存储频率低八位
				Delay5ms();
				pre_pulse = pulse;              //回显变量
				pre_temp = temp;
				pre_volt = volt;
			}
		}
	}
	if(P31 == 0){             //S6
		Delay10ms();
		if(P31 == 0){
			if(s7_flag == 0){          //切换回显界面
				while(P31 == 0){
					smgshow();
				}
				showdat_flag = 0;
				showback_flag = 1;
				s6_flag++;
				if(s6_flag > 2)
					s6_flag = 0;
			}else{                    //调整参数
				while(P31 == 0)       //长按快增
				{
					t1_cnt = 0;
					while(P31 == 0){
						if(flag_1s){
							i++;
							if(i > 1000){
								i = 0;
								if(setvolt < 50)
									setvolt++;
							}
						}
						smgshow();
						pcf8591_adc(3);
					}
					flag_1s = 0;
				}
				if(setvolt < 50)
					setvolt++;
			}
		}
	}
	if(P30 == 0){             //S7
		Delay10ms();
		if(P30 == 0){
			while(P30 == 0){
				smgshow();
			}
			showdat_flag = 0;
			showback_flag = 0;
			s7_flag = !s7_flag;
			if(s7_flag == 0)      //退出参数设置界面,进入数据界面
				showdat_flag = 1;
		}
	}
}

void smgshow()
{
	if(showdat_flag){
		if(s4_flag == 0){
			smgshow_temp();
			hc138_select(4);
			P00 = 0; P01 = 1; P02 = 1;
		}
		else if(s4_flag == 1){
			smgshow_volt();
			hc138_select(4);
			P00 = 1; P01 = 1; P02 = 0;
		}
		else if(s4_flag == 2){
			smgshow_pulse();
			hc138_select(4);
			P00 = 1; P01 = 0; P02 = 1;
		}
	}
	if(showback_flag){
		if(s6_flag == 1)
			smg_callbacktemp();
		else if(s6_flag == 2)
			smg_callbackvolt();
		else if(s6_flag == 0)
			smg_callbackpulse();
	}
	if(s7_flag)
	{
		smgshow_setvolt();
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
	tim_init();
	rd_eepdat();

	while(1)
	{
		get_temp();
		pcf8591_adc(3);
		key_scan();
		smgshow();
	}
}