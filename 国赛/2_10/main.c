#include <STC15F2K60S2.H>
#include <string.h>
#include <stdio.h>
#include "onewire.h"
#include "iic.h"

typedef unsigned char u8;
typedef unsigned int u16;

u8 templ, temph;
u16 temp;
u8 dist;
u8 set_temp = 30;  u8 preset_temp;
u8 set_dist = 35;  u8 preset_dist;
u8 s13_flag = 0, s12_flag = 0, s16_flag = 0, s17_flag = 0;
u16 change_time = 0;   //参数改变次数
u8 cnt_1s = 0;
bit flag_1s = 0;
bit dac_flag = 0;
u8 uart_rcvdat[12] = {'\0'};
u8 rcv_seq = 0;
bit rcv_flag = 0;        //串口接收标志

u8 code t_display[]={                       //标准字库
//   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71,
//black  -     H    J    K    L    N    o   P    U     t    G    Q    r   M    y
    0x00,0x40,0x76,0x1E,0x70,0x38,0x37,0x5C,0x73,0x3E,0x78,0x3d,0x67,0x50,0x37,0x6e,0xc8};
u8 code dot_display[]={0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};    //0. 1. 2. 3. 4. 5. 6. 7. 8. 9. -1

sbit TX = P1^0;
sbit RX = P1^1;

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
		temp = (temp >> 4) * 100 + (temp & 0x0f) * 6.25;
	}
}

void get_temp_init()   //防止出现85
{
	do{
		init_ds18b20();
		Write_DS18B20(0xcc);
		Write_DS18B20(0x44);
	//	delay();
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
	}while(temp == 8500);
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
	smgshow_bit(3, ~t_display[1]);
	smgshow_bit(6, ~t_display[set_temp / 10]);
	smgshow_bit(7, ~t_display[set_temp % 10]);
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

void Delay12us()		//@12.000MHz
{
	unsigned char i;

	_nop_();
	_nop_();
	i = 33;
	while (--i);
}

void sendwave()
{
	u8 i;
	for(i = 0; i < 8; i++){
		TX = 1;
		Delay12us();
		TX = 0;
		Delay12us();
	}
}

void measure_dist()   //定时器1 16位自动
{
	u16 time;
	
	TMOD &= 0x0f;
	TH1 = 0;
	TL1 = 0;
	sendwave();
	TR1 = 1;
	while((RX == 1) && (TF1 == 0));
	TR1 = 0;
	if(TF1 == 0){
		time = TH1;
		time = (time << 8) | TL1;
		dist = time * 0.017;
		if((dist < 10) || (dist > 50))
			dist = 99;
	}else{
		TF1 = 0;
		dist = 99;
	}
}

void smgshow_dist()
{
	smgshow_bit(0, ~t_display[21]);
	if(dist > 9)
		smgshow_bit(6, ~t_display[dist / 10]);
	smgshow_bit(7, ~t_display[dist % 10]);
}

void smgshow_setdist()
{
	smgshow_bit(0, ~t_display[24]);
	smgshow_bit(3, ~t_display[2]);
	smgshow_bit(6, ~t_display[set_dist / 10]);
	smgshow_bit(7, ~t_display[set_dist % 10]);
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

//u8 rd_eeprom(u8 addr)
//{
//	u8 dat
//	IIC_Start();
//	IIC_SendByte(0xa0);
//	IIC_WaitAck();
//	IIC_SendByte(addr);
//	IIC_WaitAck();
//	
//	IIC_Start();
//	IIC_SendByte(0xa1);
//	IIC_WaitAck();
//	IIC_RecByte();
//	IIC_Stop();
//}

void smgshow_change()
{
	smgshow_bit(0, t_display[32]);
	if(change_time > 9999)
		smgshow_bit(3, ~t_display[change_time / 10000]);
	if(change_time > 999)
		smgshow_bit(4, ~t_display[change_time / 1000 % 10]);
	if(change_time > 99)
		smgshow_bit(5, ~t_display[change_time / 100 % 10]);
	if(change_time > 9)
		smgshow_bit(6, ~t_display[change_time / 10 % 10]);
	smgshow_bit(7, ~t_display[change_time % 10]);
}

void tim0_init()
{
	TMOD = TMOD & 0xf0 | 0x01;
	TH0 = (65535 - 50000) / 256;
	TL0 = (65535 - 50000) % 256;
	TR0 = 1;
	ET0 = 1;
	EA = 1;
}

void tim0() interrupt 1
{
	TH0 = (65535 - 50000) / 256;
	TL0 = (65535 - 50000) % 256;
	cnt_1s++;
	if(cnt_1s == 20){
		cnt_1s = 0;
		flag_1s = 1;
	}
}

void smgshow()
{
	if(s13_flag == 0){             //数据界面
		if(s12_flag == 0)
			smgshow_temp();
		else if(s12_flag == 1)
			smgshow_dist();
		else if(s12_flag == 2)             //变更次数
			smgshow_change();
	}
	else if(s13_flag){             //参数界面
		if(s12_flag == 0)
			smgshow_settemp();
		else if(s12_flag == 1)
			smgshow_setdist();
	}
}

void UartInit(void)		//4800bps@12.000MHz
{
	SCON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x01;		//串口1选择定时器2为波特率发生器
	AUXR |= 0x04;		//定时器2时钟为Fosc,即1T
	T2L = 0x8F;		//设定定时初值
	T2H = 0xFD;		//设定定时初值
	AUXR |= 0x10;		//启动定时器2
	
	ES = 1;
}

void uart_send(u8 dat)
{
	SBUF = dat;
	while(TI == 0);
	TI = 0;
}

void send_str(u8 *str)
{
	while(*str != '\0'){
		uart_send(*str);
		str++;
	}
}

void uart_rcv() interrupt 4
{
	if(RI == 1){
		RI = 0;
		rcv_flag = 1;
		uart_rcvdat[rcv_seq] = SBUF;
		rcv_seq++;
	}
}

void uart_detect()
{
	u8 sendbuff[20] = {'\0'};
	
	if(rcv_flag){
		rcv_flag = 0;
		delay(100);      //等待串口接收完成
		if(strcmp(uart_rcvdat, "ST\r\n") == 0){
			rcv_seq = 0;
			memset(uart_rcvdat, '\0', sizeof(uart_rcvdat));
			sprintf(sendbuff, "$%d,%d.%d\r\n", (u16)dist, (u16)(temp / 100), (u16)(temp % 100));
			send_str(sendbuff);
		}else if(strcmp(uart_rcvdat, "PARA\r\n") == 0){
			rcv_seq = 0;
			memset(uart_rcvdat, '\0', sizeof(uart_rcvdat));
			sprintf(sendbuff, "#%d,%d\r\n", (u16)set_dist, (u16)set_temp);
			send_str(sendbuff);
		}else if(strlen(uart_rcvdat)){
			rcv_seq = 0;
			memset(uart_rcvdat, '\0', sizeof(uart_rcvdat));
			sprintf(sendbuff, "ERROR\r\n");
			send_str(sendbuff);
		}
	}
}

void led_work()
{
	if(temp > (set_temp * 100)){
		hc138_select(4);
		P00 = 0;
	}else{
		hc138_select(4);
		P00 = 1;
	}
	if(dist < set_dist){
		hc138_select(4);
		P01 = 0;
	}else{
		hc138_select(4);
		P01 = 1;
	}
	if(dac_flag){
		hc138_select(4);
		P02 = 0;
	}else{
		hc138_select(4);
		P02 = 1;
	}
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
	if(P35 == 0){                        //S13
		Delay10ms();
		if(P35 == 0){
			while(P35 == 0)
			{
				cnt_1s = 0;
				flag_1s = 0;
				while(P35 == 0){
					measure_dist();
					get_temp();
					smgshow();
					led_work();
				}
			}
			if(flag_1s){            //长按
				flag_1s = 0;
				dac_flag = !dac_flag;
			}
			else{                   //短按
				s13_flag = !s13_flag;
				s12_flag = 0;
				if(s13_flag){         //进入参数界面
					preset_temp = set_temp;
					preset_dist = set_dist;
				}else{                //退出参数界面
					if(preset_temp != set_temp)
						change_time++;
					if(preset_dist != set_dist)
						change_time++;
					if((preset_temp != set_temp) || (preset_dist != set_dist)){
						if(change_time > 255){
							wr_eeprom(2, (change_time >> 8));
							Delay10ms();
						}
						wr_eeprom(1, (change_time & 0xff));
					}
				}
			}
		}
	}
	if(P34 == 0){                 //S17 加
		Delay10ms();
		if(P34 == 0){
			while(P34 == 0)
			{
				get_temp();
				smgshow();
			}
			if(s13_flag == 1){
				if(s12_flag == 0){      //温度参数加
					if(set_temp < 98){
						set_temp += 2;
					}
				}
				else{
					if(set_dist < 95)  //距离参数加
						set_dist += 5;
				}
			}
		}
	}
	P30 = 1; P31 = 1; P32 = 1; P33 = 0;
	P34 = 1; P35 = 1; P42 = 1; P44 = 1;
	if(P35 == 0){                         //S12
		Delay10ms();
		if(P35 == 0){
			while(P35 == 0)
			{
				cnt_1s = 0;
				flag_1s = 0;
				while(P35 == 0){
					get_temp();
					measure_dist();
					smgshow();
					led_work();
				}
			}
			if(flag_1s){               //长按
				flag_1s = 0;
				change_time = 0;
			}
			else{                      //短按
				s12_flag++;
				if((s13_flag == 0) && (s12_flag > 2))
					s12_flag = 0;
				if((s13_flag == 1) && (s12_flag > 1))
					s12_flag = 0;
			}
		}
	}
	if(P34 == 0){                 //S16减
		Delay10ms();
		if(P34 == 0){
			while(P34 == 0)
			{
				get_temp();
				smgshow();
			}
			if(s13_flag == 1){
				if(s12_flag == 0){      //温度参数减
					if(set_temp > 0)
						set_temp -= 2;
				}
				else{
					if(set_dist > 0)  //距离参数减
						set_dist -= 5;
				}
			}
		}
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
	u16 t = 0;
	
//	wr_eeprom(1, 0);
//	wr_eeprom(2, 0);
	get_temp_init();
	sys_init();
	tim0_init();
	UartInit();
	while(1)
	{
		t++;
		if(t == 30){
			t = 0;
			measure_dist();
		}
		get_temp();
		key_scan();
		uart_detect();
		smgshow();
		led_work();
		if(dac_flag){
			if(dist <= set_dist)
				pcf8591_dac(102);
			else
				pcf8591_dac(204);
		}else{
			pcf8591_dac(21);
		}
	}
}