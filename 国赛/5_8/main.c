/*
 * 2022/6/14
 */

#include <STC15F2K60S2.H>
#include "iic.h"

typedef unsigned char u8;
typedef unsigned int u16;

sbit TX = P1^0;
sbit RX = P1^1;

u8 dist[10] = {0};
u8 dist_i = 0;
bit s4_flag = 1;
bit s4_mode = 0;
u8 s5_flag = 0;
u8 show_back = 0;
bit s6_flag = 0;
u8 set_dist = 20;
u8 realset_dist = 20;
bit measure_flag = 0;
u8 t0_cnt = 0;

u8 code t_display[]={                       //标准字库
//   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71,
//black  -     H    J    K    L    N    o   P    U     t    G    Q    r   M    y
    0x00,0x40,0x76,0x1E,0x70,0x38,0x37,0x5C,0x73,0x3E,0x78,0x3d,0x67,0x50,0x37,0x6e,
    0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};    //0. 1. 2. 3. 4. 5. 6. 7. 8. 9. -1

void wr_dat();
void wr_eeprom(u8 addr, u8 dat);
void Delay5ms();
	
void hc138_select(u8 n)
{
	switch(n){
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

void Delay12us()		//@11.0592MHz
{
	unsigned char i;

	_nop_();
	_nop_();
	_nop_();
	i = 30;
	while (--i);
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
void measure_dist()
{
	u16 t;
	send_wave();
	TMOD = TMOD & 0x0f;
	TH1 = 0;
	TL1 = 0;
	TR1 = 1;
	while((RX == 1) && (TF1 == 0));
	TR1 = 0;
	if(TF1 == 0){
		t = TH1;
		t = (t << 8) | TL1;
		dist[dist_i] = t * 0.017;
	}else{
		TF1 = 0;
		dist[dist_i] = 255;
	}
	wr_eeprom(dist_i + 2, dist[dist_i]);
	Delay5ms();
	measure_flag = 1;
	dist_i++;
	if(dist_i > 9){
		dist_i = 0;
	}
}
//void wr_dat()
//{
//	u8 i;
//	wr_eeprom(1, realset_dist);
//	Delay5ms();
//	for(i = 0; i < 10; i++){
//		wr_eeprom(i+2, dist[i]);
//		Delay5ms();
//	}
//}

void smgshow_dist()
{
	u8 show_seq;
	u8 add;
	if(s4_mode == 0){        //无操作
		smgshow_bit(0, ~t_display[0]);
		/* 本次 */
		if(dist_i == 0)
			show_seq = 9;
		else
			show_seq = dist_i - 1;
		smgshow_bit(5, ~t_display[dist[show_seq] / 100]);
		smgshow_bit(6, ~t_display[dist[show_seq] / 10 % 10]);
		smgshow_bit(7, ~t_display[dist[show_seq] % 10]);
		/* 上一次 */
		if(show_seq == 0)
			show_seq = 9;
		else
			show_seq -= 1;
		
		if(dist[show_seq] > 99)
			smgshow_bit(2, ~t_display[dist[show_seq] / 100]);
		if(dist[show_seq] > 9)
			smgshow_bit(3, ~t_display[dist[show_seq] / 10 % 10]);
		smgshow_bit(4, ~t_display[dist[show_seq] % 10]);
	}
	else{                        //和
		smgshow_bit(0, ~t_display[1]);
		/* 本次 */
		if(dist_i == 0)
			show_seq = 9;
		else
			show_seq = dist_i - 1;
		smgshow_bit(5, ~t_display[dist[show_seq] / 100]);
		smgshow_bit(6, ~t_display[dist[show_seq] / 10 % 10]);
		smgshow_bit(7, ~t_display[dist[show_seq] % 10]);
		add = dist[show_seq];
		/* 和 */
		if(show_seq == 0)
			show_seq = 9;
		else
			show_seq -= 1;
		add += dist[show_seq];
		
		if(add > 99)
			smgshow_bit(2, ~t_display[add / 100]);
		if(add > 9)
			smgshow_bit(3, ~t_display[add / 10 % 10]);
		smgshow_bit(4, ~t_display[add % 10]);
	}
}
void smgshow_back()
{
	smgshow_bit(0, ~t_display[(show_back + 1) / 10]);
	smgshow_bit(1, ~t_display[(show_back + 1) % 10]);
	smgshow_bit(5, ~t_display[dist[show_back] / 100]);
	smgshow_bit(6, ~t_display[dist[show_back] / 10 % 10]);
	smgshow_bit(7, ~t_display[dist[show_back] % 10]);
}
void smgshow_setpara()
{
	smgshow_bit(0, ~t_display[15]);
	smgshow_bit(6, ~t_display[set_dist / 10]);
	smgshow_bit(7, ~t_display[set_dist % 10]);
}

void smgshow()
{
	if((s5_flag == 0) && (s6_flag == 0))
		smgshow_dist();
	else if((s5_flag == 1) && (s6_flag == 0))
		smgshow_back();
	else if(s6_flag)
		smgshow_setpara();
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
void dac_out()
{
	u8 dist_seq;
	u8 volt;
	if(dist_i == 0)
		dist_seq = 9;
	else
		dist_seq = dist_i - 1;
	if(dist[dist_seq] <= realset_dist)
		pcf8591_dac(0);
	else{
		volt = (dist[dist_seq] - realset_dist) * 0.02;
		if(volt > 5)
			volt = 5;
		pcf8591_dac((dist[dist_seq] - realset_dist) * 1.02);
	}
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

void tim0_init()
{
	TMOD = TMOD & 0xf0 | 0x01;
	TH0 = (65536 - 50000) / 256;
	TL0 = (65536 - 50000) % 256;
	ET0 = 1;
	TR0 = 0;
	EA = 1;
}
bit L1_flag = 0;
void tim0() interrupt 1
{
	static u8 j = 0;
	TH0 = (65536 - 50000) / 256;
	TL0 = (65536 - 50000) % 256;
	t0_cnt++;
	if(t0_cnt == 4){
		j++;
		if(j == 20){
			j = 0;
			measure_flag = 0;
			TR0 = 0;
		}
		t0_cnt = 0;
		L1_flag = !L1_flag;
	}
}
void led_work()
{
	if(measure_flag){
		TR0 = 1;
		if(L1_flag){
			hc138_select(4);
			P00 = 0;
		}
		else{
			hc138_select(4);
			P00 = 1;
		}
	}
	if(s6_flag){
		hc138_select(4);
		P06 = 0;
	}else{
		hc138_select(4);
		P06 = 1;
	}
	if((s6_flag == 0) && s5_flag){
		hc138_select(4);
		P07 = 0;
	}else{
		hc138_select(4);
		P07 = 1;
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
	if(P33 == 0){             //S4
		Delay10ms();
		if(P33 == 0){
			while(P33 == 0){
				smgshow();
			}
			measure_dist();
		}
	}
	if(P32 == 0){             //S5
		Delay10ms();
		if(P32 == 0){
			while(P32 == 0){
				smgshow();
			}
			s5_flag = !s5_flag;
		}
	}
	if(P31 == 0){             //S6
		Delay10ms();
		if(P31 == 0){
			while(P31 == 0){
				smgshow();
			}
			s6_flag = !s6_flag;
			if(s6_flag == 0){             //退出参数界面参数生效
				realset_dist = set_dist;
			}
		}
	}
	if(P30 == 0){             //S7
		Delay10ms();
		if(P30 == 0){
			while(P30 == 0){
				smgshow();
			}
			if(s6_flag == 0){
				if(s5_flag == 0)              //测距界面
					s4_mode = !s4_mode;
				if(s5_flag){                  //回显界面
					show_back++;
					if(show_back > 9)
						show_back = 0;
				}
			}
			else{                  //参数界面
				set_dist += 10;
				if(set_dist > 90)
					set_dist = 0;
				wr_eeprom(1, set_dist);
				Delay5ms();
			}
		}
	}
}

void sys_init()
{
	u8 i;
	hc138_select(4);
	P0 = 0xff;
	hc138_select(5);
	P0 = 0x00;
	
	realset_dist = rd_eeprom(1);          //设定参数
	set_dist = realset_dist;
	for(i = 0; i < 8; i++){
		dist[i] = rd_eeprom(i + 2);
	}
}
void main()
{
	sys_init();
	tim0_init();
	while(1)
	{
		key_scan();
		led_work();
		dac_out();
		smgshow();
	}
}