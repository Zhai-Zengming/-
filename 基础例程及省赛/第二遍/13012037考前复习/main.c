#include <STC15F2K60S2.H>
#include "onewire.h"
#include "iic.h"
#include "ds1302.h"

typedef unsigned char u8;
typedef unsigned int u16;

sbit TX = P1^0;
sbit RX = P1^1;

u8 templ, temph;
u16 temp;
u8 eeprom_dat;
u8 s7_flag = 0;
u8 adc_dat;
float v_dat;
u16 NE555_cnt;
u16 NE555_pulse;
u8 tim1_cnt = 0;
float dist;
u8 ds1302_wdaddr[] = {0x80, 0x82, 0x84, 0x86, 0x88, 0x8a, 0x8c};
u8 ds1302_rdaddr[] = {0x81, 0x83, 0x85, 0x87, 0x89, 0x8b, 0x8d};
u8 ds1302_buf[] = {0x50, 0x59, 0x23, 0x00, 0x00, 0x00, 0x00};

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
	u8 i = 100;
	while(i--);
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
	u8 i = 20;
	while(i--)
		smgshow();
}

void gettemp_init()
{
	do{
		init_ds18b20();
		Write_DS18B20(0xcc);
		Write_DS18B20(0x44);
//		delay();
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
	}while(temp == 850);
}

void get_temp()
{
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0x44);
	delay();
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
	smgshow_bit(5, ~t_display[temp / 100]);
	smgshow_bit(6, ~dot_display[temp / 10 % 10]);
	smgshow_bit(7, ~t_display[temp % 10]);
}

void read_eeprom(u8 addr)
{
	IIC_Start();
	IIC_SendByte(0xa0);
	IIC_WaitAck();
	IIC_SendByte(addr);
	IIC_WaitAck();
	
	IIC_Start();
	IIC_SendByte(0xa1);
	IIC_WaitAck();
	eeprom_dat = IIC_RecByte();
	IIC_SendAck(1);          //非应答
	IIC_Stop();
}

void write_eeprom(u8 addr, u8 dat)
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

void pcf8591_adc(u8 channel)
{
	IIC_Start();
	IIC_SendByte(0x90);
	IIC_WaitAck();
	IIC_SendByte(channel);
	IIC_WaitAck();
	
	IIC_Start();
	IIC_SendByte(0x91);
	IIC_WaitAck();
	adc_dat = IIC_RecByte();
	IIC_SendAck(1);          //非应答
	IIC_Stop();
	
	v_dat = adc_dat * 1.97;
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

void smgshow_volt()
{
	smgshow_bit(5, ~dot_display[(int)v_dat / 100]);
	smgshow_bit(6, ~t_display[(int)v_dat / 10 % 10]);
	smgshow_bit(7, ~t_display[(int)v_dat % 10]);
}

void Delay5ms()		//@12.000MHz
{
	unsigned char i, j;

	i = 59;
	j = 90;
	do
	{
		while (--j);
	} while (--i);
}

void smgshow_eeprom()
{
	smgshow_bit(0, ~t_display[eeprom_dat]);
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
	if(P30 == 0){
		Delay10ms();
		if(P30 == 0){
			while(P30 == 0){
				smgshow();
			}
			s7_flag++;
			if(s7_flag == 5)
				s7_flag = 0;
		}
	}
}

//NE555
void tim_init()
{
	TMOD = 0x16;
	TH1 = (65535 - 50000) / 256;
	TL1 = (65535 - 50000) % 256;
	TH0 = 0xff;
	TL0 = 0xff;
	ET0 = 1;
	ET1 = 1;
	TR0 = 1;
	TR1 = 1;
	EA = 1;
}

void smgshow_pulse()
{
	if(NE555_pulse > 9999)
		smgshow_bit(3, ~t_display[NE555_pulse / 10000]);
	if(NE555_pulse > 999)
		smgshow_bit(4, ~t_display[NE555_pulse / 1000 % 10]);
	if(NE555_pulse > 99)
		smgshow_bit(5, ~t_display[NE555_pulse / 100 % 10]);
	if(NE555_pulse > 9)
		smgshow_bit(6, ~t_display[NE555_pulse / 10 % 10]);
	smgshow_bit(7, ~t_display[NE555_pulse % 10]);
}

void write_ds1302()
{
	u8 i;
	
	Write_Ds1302_Byte(0x8e, 0x00);
	for(i = 0; i < 7; i++){
		Write_Ds1302_Byte(ds1302_wdaddr[i], ds1302_buf[i]);
	}
	Write_Ds1302_Byte(0x8e, 0x80);
}

void read_ds1302()
{
	u8 i;

	for(i = 0; i < 7; i++){
		ds1302_buf[i] = Read_Ds1302_Byte(ds1302_rdaddr[i]);
	}
}

void smgshow_time()
{
	smgshow_bit(0, ~t_display[ds1302_buf[2] / 16]);
	smgshow_bit(1, ~t_display[ds1302_buf[2] % 16]);
	smgshow_bit(2, ~t_display[17]);
	smgshow_bit(3, ~t_display[ds1302_buf[1] / 16]);
	smgshow_bit(4, ~t_display[ds1302_buf[1] % 16]);
	smgshow_bit(5, ~t_display[17]);
	smgshow_bit(6, ~t_display[ds1302_buf[0] / 16]);
	smgshow_bit(7, ~t_display[ds1302_buf[0] % 16]);
}

void Delay12us()		//@12.000MHz
{
	unsigned char i;

	_nop_();
	_nop_();
	i = 33;
	while (--i);
}

void tim1_init()
{
	TMOD = 0x00;
	TH1 = 0;
	TL1 = 0;
	TR1 = 0;
}

void send_wave()
{
	u8 i;
	for(i = 0; i < 8; i++){
		TX = 1;
		Delay12us();
		TX = 0;
		Delay12us();
	}
}

void Delay60ms()		//@12.000MHz
{
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 3;
	j = 189;
	k = 92;
	do
	{
		do
		{
			while (--k);
			smgshow();
			key_scan();
		} while (--j);
		smgshow();
		key_scan();
	} while (--i);
}

void measure_dist()
{
	u16 t;
	
	tim1_init();
	send_wave();
	TR1 = 1;
	while((RX == 1) && (TF1 == 0));
	TR1 = 0;
	if(TF1 == 0){
		t = TH1;
		t = (t << 8) | TL1;
		dist = t * 0.017;
	}
	else if(TF1 == 1){
		TF1 = 0;
		dist = 999;
	}
	Delay60ms();
}

void smgshow_dist()
{
	u8 distance;
	distance = (u8)dist;
	if(dist > 99)
		smgshow_bit(5, ~t_display[distance/ 100]);
	if(dist > 9)
		smgshow_bit(6, ~t_display[distance / 10 % 10]);
	smgshow_bit(7, ~t_display[distance % 10]);
}

void smgshow()
{
	if(s7_flag == 1)
		smgshow_temp();
	else if(s7_flag == 0)
		smgshow_eeprom();
	else if(s7_flag == 2)
		smgshow_volt();
	else if(s7_flag == 3)
//		smgshow_pulse();
		smgshow_dist();
	else if(s7_flag == 4)
		smgshow_time();
}

void sys_init()
{
	hc138_select(4);
	P0 = 0xff;
	hc138_select(5);
	P0 = 0x00;
	
	read_eeprom(0);
	Delay5ms();
	eeprom_dat += 1;
	if(eeprom_dat > 10)
		eeprom_dat = 0;
	write_eeprom(0, eeprom_dat);
	Delay5ms();
}

void main()
{
	sys_init();
//	tim_init();
	gettemp_init();
	write_ds1302();
	while(1)
	{
		read_ds1302();
		pcf8591_adc(3);
		measure_dist();
		pcf8591_dac(adc_dat);
		key_scan();
		get_temp();
		smgshow();
	}
}
#if 0
void tim0() interrupt 1
{
	NE555_cnt++;
}

void tim1() interrupt 3
{
	tim1_cnt++;
	TH1 = (65535 - 50000) / 256;
	TL1 = (65535 - 50000) % 256;
	if(tim1_cnt == 20){
		tim1_cnt = 0;
		NE555_pulse = NE555_cnt;
		NE555_cnt = 0;
	}
}
#endif