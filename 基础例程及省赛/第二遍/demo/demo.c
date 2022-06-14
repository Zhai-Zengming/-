#include <STC15F2K60S2.H>
#include "onewire.h"
#include "ds1302.h"
#include "iic.h"

typedef unsigned char u8;
typedef unsigned int u16;


u8 cnt = 0;
u8 pwm_cnt = 0;
u8 time_s = 0;
u8 cnt_flag = 0;
u8 s7_flag = 0, s6_flag = 0, s5_flag = 0;
u8 pwm_duty = 0;
u8 templ, temph;
u16 temp;
u16 pulse;        //555
u16 pulse_cnt;
float v_dat;
u8 at24c02_dat;
u8 code t_display[]={                       //标准字库
//   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71,
//black  -     H    J    K    L    N    o   P    U     t    G    Q    r   M    y
    0x00,0x40,0x76,0x1E,0x70,0x38,0x37,0x5C,0x73,0x3E,0x78,0x3d,0x67,0x50,0x37,0x6e};
u8 code dot_display[]={0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};    //0. 1. 2. 3. 4. 5. 6. 7. 8. 9. -1
   
u8 ds1302_writeaddr[] = {0x80, 0x82, 0x84, 0x86, 0x88, 0x8a, 0x8c};
u8 ds1302_readaddr[] =  {0x81, 0x83, 0x85, 0x87, 0x89, 0x8b, 0x8d};

u8 ds1302_buf[] = {0x45, 0x59, 0x23, 0x07, 0x05, 0x06, 0x22};

void smgshow_temp();

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

//void led_work()
//{
//	hc138_select(4);
//	if(cnt_flag)
//		P0 = 0x00;
//	else
//		P0 = 0xff;
//}

void smg_delay()
{
	u8 t = 100;
	while(t--);
}

void smg_showbit(u8 pos, u8 dat)
{
	hc138_select(6);
	P0 = 1 << pos;
	hc138_select(7);
	P0 = dat;
	smg_delay();
	P0 = 0xff;
}

//void smgshow()
//{
//	/* SSPU */
//	smg_showbit(0, ~t_display[5]);
//	smg_showbit(1, ~t_display[5]);
//	smg_showbit(2, ~t_display[24]);
//	smg_showbit(3, ~t_display[25]);
//	
//	/* cnt */
//	if(time_s > 999)
//	{
//		smg_showbit(4, ~t_display[time_s / 1000]);
//	}
//	if(time_s > 99)
//	{
//		smg_showbit(5, ~t_display[time_s % 1000 / 100]);
//	}
//	if(time_s > 9)
//	{
//		smg_showbit(6, ~t_display[time_s % 1000 % 100 / 10]);
//	}
//	smg_showbit(7, ~t_display[time_s % 10]);
//}

//void Tim_init()
//{
//	TMOD = 0x11;
//	TH0 = (65535 - 50000) / 256;
//	TL0 = (65535 - 50000) % 256;
//	ET0 = 1;
//	TR0 = 1;
//	
//	TH1 = (65535 - 100) / 256;
//	TL1 = (65535 - 100) % 256;
//	ET1 = 1;
//	TR1 = 1;
//	
//	EA = 1;
//}

/* 555 */
//void Tim_init()
//{
//	TMOD = 0x16;
//	TH1 = (65535 - 50000) / 256;
//	TL1 = (65535 - 50000) % 256;    //计时
//	ET1 = 1;
//	TR1 = 1;
//	
//	TH0 = 0xff;                     //计数
//	TL0 = 0xff;
//	ET0 = 1;
//	TR0 = 1;
//	
//	EA = 1;
//}

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


//void key_scan()
//{
//	if(P30 == 0){
//		Delay10ms();
//		if(P30 == 0){
//			while(P30 == 0);
//			s7_flag = !s7_flag;
//		}
//	}
//	if(P31 == 0){
//		Delay10ms();
//		if(P31 == 0){
//			while(P31 == 0);
//			s6_flag++;
//			if(s6_flag == 5){
//				s6_flag = 0;
//			}
//			pwm_duty = 25 * s6_flag;
//		}
//	}
//	if(P32 == 0){
//		Delay10ms();
//		if(P32 == 0){
//			while(P32 == 0);
//			s6_flag = !s6_flag;
//		}
//	}
//}

//void temp_delay()
//{
//	u8 t = 10;
//	while(t--)
//		smgshow_temp();
//}

//void get_temp()
//{
//	init_ds18b20();
//	Write_DS18B20(0xcc);
//	Write_DS18B20(0x44);
//	temp_delay();
//	init_ds18b20();
//	Write_DS18B20(0xcc);
//	Write_DS18B20(0xbe);
//	
//	templ = Read_DS18B20();
//	temph = Read_DS18B20();
//	temp = temph;
//	temp = (temp << 8) | templ;
//	if((temp & 0xf800) == 0x0000)
//	{
//		temp = (temp >> 4) * 10 + (temp & 0x0f) * 10 * 0.0625;
//	}
//}

//void smgshow_temp()
//{
//	smg_showbit(0, 0xff);
//	smg_showbit(1, 0xff);
//	smg_showbit(2, 0xff);
//	smg_showbit(3, 0xff);
//	smg_showbit(4, 0xff);
//	
//	smg_showbit(5, ~t_display[temp / 100]);
//	smg_showbit(6, ~dot_display[temp % 100 / 10]);
//	smg_showbit(7, ~t_display[temp % 10]);
//}

void sys_init()
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

//void ds1302_init()
//{
//	u8 i;
//	Write_Ds1302_Byte(0x8e, 0x00);    //关闭写保护
//	for(i = 0; i < 7; i++){
//		Write_Ds1302_Byte(ds1302_writeaddr[i], ds1302_buf[i]);
//	}
//	Write_Ds1302_Byte(0x8e, 0x80);    //打开写保护
//}

//void read_ds1302()
//{
//	u8 i;
//	
//	for(i = 0; i < 7; i++){
//		ds1302_buf[i] = Read_Ds1302_Byte(ds1302_readaddr[i]);
//	}
//}

//void smgshow_ds1302()
//{
//	smg_showbit(0, ~t_display[ds1302_buf[2] / 16]);
//	smg_showbit(1, ~t_display[ds1302_buf[2] % 16]);
//	smg_showbit(2, ~t_display[17]);
//	smg_showbit(3, ~t_display[ds1302_buf[1] / 16]);
//	smg_showbit(4, ~t_display[ds1302_buf[1] % 16]);
//	smg_showbit(5, ~t_display[17]);
//	smg_showbit(6, ~t_display[ds1302_buf[0] / 16]);
//	smg_showbit(7, ~t_display[ds1302_buf[0] % 16]);
//}

//void smgshow_555pulse()
//{
//	if(pulse > 9999)
//		smg_showbit(3, ~t_display[pulse / 10000]);
//	if(pulse > 999)
//		smg_showbit(4, ~t_display[pulse / 1000 % 10]);
//	if(pulse > 99)
//		smg_showbit(5, ~t_display[pulse / 100 % 10]);
//	if(pulse > 9)
//		smg_showbit(6, ~t_display[pulse / 10 % 10]);
//	smg_showbit(7, ~t_display[pulse % 10]);
//}

//void pcf8591_adc(u8 addr)
//{
//	IIC_Start();
//	IIC_SendByte(0x90);
//	IIC_WaitAck();
//	IIC_SendByte(addr);
//	IIC_WaitAck();
//	IIC_Stop();
//	
//	IIC_Start();
//	IIC_SendByte(0x91);
//	IIC_WaitAck();
//	v_dat = IIC_RecByte();
//	IIC_SendAck(0);
//	IIC_Stop();
//	
//	v_dat = v_dat * 500 / 255;
//}

//void pcf8591_dac(u8 dat)
//{
//	IIC_Start();
//	IIC_SendByte(0x90);
//	IIC_WaitAck();
//	IIC_SendByte(0x40);        //表示dac，dac功能与CONTROL BYTE的低七位无关
//	IIC_WaitAck();
//	
//	IIC_SendByte(dat);
//	IIC_WaitAck();
//	IIC_Stop();
//}

//void smgshow_volt()
//{
//	smg_showbit(5, ~t_display[(int)v_dat / 100]);
//	smg_showbit(6, ~t_display[(int)v_dat / 10 % 10]);
//	smg_showbit(7, ~t_display[(int)v_dat % 10]);
//}

void Delay5ms()		//@12.000MHz
{
	unsigned char i, j;

	_nop_();
	_nop_();
	i = 59;
	j = 89;
	do
	{
		while (--j);
	} while (--i);
}


void read_at24c02(u8 addr)
{
	IIC_Start();
	IIC_SendByte(0xa0);
	IIC_WaitAck();
	IIC_SendByte(addr);
	IIC_WaitAck();
	IIC_Stop();
	
	IIC_Start();
	IIC_SendByte(0xa1);
	IIC_WaitAck();
	at24c02_dat = IIC_RecByte();
	IIC_SendAck(1);          //非应答
	IIC_Stop();
}

void write_at24c02(u8 addr, u8 dat)
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

void smgshow_atdat()
{
	smg_showbit(6, ~t_display[at24c02_dat / 10]);
	smg_showbit(7, ~t_display[at24c02_dat % 10]);
}

void main()
{
	Delay5ms();
//	Tim_init();
	sys_init();
//	ds1302_init();
	read_at24c02(0x01);
	Delay5ms();
	at24c02_dat += 1;
	write_at24c02(0x01, at24c02_dat);
	Delay5ms();
	
	while(1){
//		read_ds1302();
//		get_temp();
//		key_scan();
//		if(s7_flag == 0){
//			led_work();
//			smgshow();
//		}
//		else{
//			hc138_select(4);
//			P01 = 0;
//		}
//		smgshow_temp();
//		smgshow_ds1302();
//		smgshow_555pulse();
//		pcf8591_adc(0x01);
//		smgshow_volt();
//		pcf8591_dac(0);
		smgshow_atdat();
	}
}

//void tim0() interrupt 1
//{
//	cnt++;
//	TH0 = (65535 - 50000) / 256;
//	TL0 = (65535 - 50000) % 256;
//		
//	if(cnt == 20)
//	{
//		time_s++;
//		if(time_s > 59)
//			time_s = 0;
//		cnt = 0;
//		cnt_flag = !cnt_flag;
//	}
//}

//void tim1() interrupt 3
//{
//	pwm_cnt++;
//	TH1 = (65535 - 100) / 256;
//	TL1 = (65535 - 100) % 256;
//	if(pwm_cnt == 100)
//		pwm_cnt = 0;
//	if(s7_flag){
//		if(pwm_cnt < pwm_duty)
//			P00 = 0;
//		else
//			P00 = 1;
//	}
//}

//计时
//void tim1() interrupt 3
//{
//	cnt++;
//	TH1 = (65535 - 50000) / 256;
//	TL1 = (65535 - 50000) % 256;
//		
//	if(cnt == 20)
//	{
//		cnt = 0;
//		pulse = pulse_cnt;
//		pulse_cnt = 0;
//	}
//}
////计数
//void tim0() interrupt 1
//{
//	pulse_cnt++;
//}

