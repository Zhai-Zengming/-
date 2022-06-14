#include <STC15F2K60S2.H>
#include "iic.h"
#include "onewire.h"

float v_dat;
u8 temp_buf = 0;
u8 settemp = 25;
u8 templ, temph;
u16 temp;
u8 s4_flag = 0, s5_flag = 0;
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

void delay(u16 t)
{
	while(t--)
		smgshow();
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

void get_temp_init()
{
	do{
		init_ds18b20();
		Write_DS18B20(0xcc);
		Write_DS18B20(0x44);
//		delay(20);
		init_ds18b20();
		Write_DS18B20(0xcc);
		Write_DS18B20(0xbe);
		
		templ = Read_DS18B20();
		temph = Read_DS18B20();
		temp = temph;
		temp = (temp << 8) | templ;
		if((temp & 0xf800) == 0x0000){
			temp = (temp >> 4) * 100 + (temp & 0x000f) * 6.25;
		}
	}while(temp == 8500);
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
		temp = (temp >> 4) * 100 + (temp & 0x000f) * 6.25;
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

void smgshow_settemp()
{
	smgshow_bit(0, ~t_display[24]);
	smgshow_bit(6, ~t_display[temp_buf / 10]);
	smgshow_bit(7, ~t_display[temp_buf % 10]);
}

void pcf8591_dac(u8 dat)
{
	IIC_Start();
	IIC_SendByte(0x90);
	IIC_WaitAck();
	IIC_SendByte(0x40);
	IIC_WaitAck();
	
//	IIC_Start();
	IIC_SendByte(dat);
	IIC_WaitAck();
	IIC_Stop();
}

void pcf8591_out()
{
	if(s5_flag == 0){       //模式1
		if(temp < (settemp * 100)){
			v_dat = 0;
			pcf8591_dac(0);
		}else{
			v_dat = 5;
			pcf8591_dac(0xff);
		}
	}else{                  //模式2
		if(temp <= 2000){
			v_dat = 1;
			pcf8591_dac(51);                           //1V
		}else if((temp > 2000) && (temp < 4000)){
			v_dat = 0.0015 * temp - 2;
			pcf8591_dac((u8)(v_dat * 51));             //线性
		}else{
			v_dat = 4;
			pcf8591_dac(4 * 51);                       //4V
		}
	}
}

void smgshow_volt()
{
	smgshow_bit(0, ~t_display[10]);
	
	smgshow_bit(5, ~dot_display[(u16)(v_dat * 100) / 100]);
	smgshow_bit(6, ~t_display[(u16)(v_dat * 100) / 10 % 10]);
	smgshow_bit(7, ~t_display[(u16)(v_dat * 100) % 10]);
}

void smgshow()
{
	if(s4_flag == 0)
		smgshow_temp();
	if(s4_flag == 1)
		smgshow_settemp();
	if(s4_flag == 2)
		smgshow_volt();
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
	if(P44 == 0){
		Delay10ms();
		if(P44 == 0){                             //s5模式切换
			while(P44 == 0){
				smgshow();
			}
			s5_flag = !s5_flag;
		}
	}
	if((P42 == 0) && (s4_flag == 1)){                 //s9
		Delay10ms();
		if(P42 == 0){
			while(P42 == 0){
				smgshow();
			}
			temp_buf++;
		}
	}
	
	P30 = 1; P31 = 1; P32 = 1; P33 = 0;
	P34 = 1; P35 = 1; P42 = 1; P44 = 1;
	if(P44 == 0){                              //s4显示界面切换
		Delay10ms();
		if(P44 == 0){
			while(P44 == 0){
				smgshow();
			}
			s4_flag++;
			if(s4_flag == 3)
				s4_flag = 0;
			if(s4_flag == 1)                   //进入参数设置界面将参数赋值给暂时参数
				temp_buf = settemp;
			if(s4_flag == 2)                   //退出参数设置界面参数有效
				settemp = temp_buf;
		}
	}
	if((P42 == 0) && (s4_flag == 1)){                 //s9
		Delay10ms();
		if(P42 == 0){
			while(P42 == 0){
				smgshow();
			}
			temp_buf--;
		}
	}
}

void led_work()
{
	if(s5_flag == 0){
		hc138_select(4);
		P00 = 0;
	}else{
		hc138_select(4);
		P00 = 1;
	}
	if(s4_flag == 0){
		hc138_select(4);
		P01 = 0;
		P02 = 1;
		P03 = 1;
	}else if(s4_flag == 1){
		hc138_select(4);
		P01 = 1;
		P02 = 0;
		P03 = 1;
	}else{
		hc138_select(4);
		P01 = 1;
		P02 = 1;
		P03 = 0;
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
	get_temp_init();
	while(1)
	{
		get_temp();
		key_scan();
		pcf8591_out();
		smgshow();
		led_work();
	}
}