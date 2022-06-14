/*
 * 2022/6/12
 */

#include <STC15F2K60S2.H>
#include "iic.h"
#include "ds1302.h"  									

typedef unsigned char u8;
typedef unsigned int u16;

sbit TX = P1^0;
sbit RX = P1^1;

u8 dist;
u8 gettime[] = {2, 3, 5, 7, 9};     //采集时间
u8 time_i = 0;    u8 real_timei;
u8 setdist = 20;  u8 real_setdist;
u8 s5_flag = 0;
u8 s4_flag = 0;
u8 s8_flag = 0;
bit mode_flag = 0;
u8 time_s;
u8 adc_dat;
u8 dist_dat[50] = {0};
u16 dist_i = 0;
u8 dist_max = 0, dist_min = 200;
u16 dist_ave = 0;
bit L5_flag = 0;

u8 wrds1302_addr[] = {0x80, 0x82, 0x84, 0x86, 0x88, 0x8a, 0x8c};
u8 rdds1302_addr[] = {0x81, 0x83, 0x85, 0x87, 0x89, 0x8b, 0x8d};
u8 ds1302_buff[] = {0x01, 0x20, 0x20, 0x01, 0x01, 0x01, 0x01};

u8 code t_display[]={                       //标准字库
//   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71,
//black  -     H    J    K    L    N    o   P    U     t    G    Q    r   M    y
    0x00,0x40,0x76,0x1E,0x70,0x38,0x37,0x5C,0x73,0x3E,0x78,0x3d,0x67,0x50,0x37,0x6e};
u8 code dot_display[]={0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};    //0. 1. 2. 3. 4. 5. 6. 7. 8. 9. -1

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

void Delay12us()		//@12.000MHz
{
	unsigned char i;

	_nop_();
	_nop_();
	i = 33;
	while (--i);
}
void send_wave()
{
	u8 i;
	for(i = 0; i < 8; i++)
	{
		TX = 1;
		Delay12us();
		TX = 0;
		Delay12us();
	}
}
void measure_dist()
{
	u16 t;
	
	TMOD &= 0x0f;     //TIM1
	TH1 = 0;
	TL1 = 0;
	send_wave();
	TR1 = 1;
	while((RX == 1) && (TF1 == 0));
	TR1 = 0;
	if(TF1 == 0){
		t = TH1;
		t = (t << 8) | TL1;
		dist = t * 0.017;
	}else{
		TF1 = 0;
		dist = 99;
	}
	dist_dat[dist_i] = dist;
	dist_i++;
	
}
void smgshow_dist()
{
	smgshow_bit(0, ~t_display[21]);
	if(mode_flag == 0)
		smgshow_bit(1, ~t_display[12]);
	else
		smgshow_bit(1, ~t_display[15]);
	if(dist > 99)
		smgshow_bit(5, ~t_display[dist / 100]);
	if(dist > 9)
		smgshow_bit(6, ~t_display[dist / 10 % 10]);
	smgshow_bit(7, ~t_display[dist % 10]);
}
void smgshow_gettime()        //采集时间
{
	smgshow_bit(0, ~t_display[24]);
	smgshow_bit(1, ~t_display[1]);
	smgshow_bit(6, ~t_display[gettime[time_i] / 10]);
	smgshow_bit(7, ~t_display[gettime[time_i] % 10]);
}
void smgshow_distpara()        //距离参数
{
	smgshow_bit(0, ~t_display[24]);
	smgshow_bit(1, ~t_display[2]);
	smgshow_bit(6, ~t_display[setdist / 10]);
	smgshow_bit(7, ~t_display[setdist % 10]);
}
void find_dist()          //寻找最大值等
{
	u8 i, j;
	u8 temp;
	if(dist_i == 0){
		dist_max = 0;
		dist_min = 0;
		dist_ave = 0;
	}else{
		for(i = 0; i < dist_i - 1; i++){        //冒泡从小到大
			for(j = 0; j  < (dist_i - i - 1); j++){
				if(dist_dat[j] > dist_dat[j + 1]){
					temp = dist_dat[j];
					dist_dat[j] = dist_dat[j + 1];
					dist_dat[j + 1] = temp;
				}
			}
		}
		for(i = 0; i < dist_i - 1; i++){
			dist_ave += dist_dat[i];
		}
		dist_ave = dist_ave * 10 / (dist_i - 1);
		dist_max = dist_dat[dist_i - 1];
		dist_min = dist_dat[0];
	}
}
void smgshow_record()           //显示记录数据
{
	smgshow_bit(0, ~t_display[18]);
	if(s8_flag == 0){
		smgshow_bit(1, 0xfe);
		if(dist_max > 999)
			smgshow_bit(4, ~t_display[dist_max / 1000]);
		if(dist_max > 99)
			smgshow_bit(5, ~t_display[dist_max / 100 % 10]);
		if(dist_max > 9)
			smgshow_bit(6, ~t_display[dist_max / 10 % 10]);
		smgshow_bit(7, ~t_display[dist_max % 10]);
	}
	else if(s8_flag == 1){
		smgshow_bit(1, 0xf7);
		if(dist_min > 999)
			smgshow_bit(4, ~t_display[dist_min / 1000]);
		if(dist_min > 99)
			smgshow_bit(5, ~t_display[dist_min / 100 % 10]);
		if(dist_min > 9)
			smgshow_bit(6, ~t_display[dist_min / 10 % 10]);
		smgshow_bit(7, ~t_display[dist_min % 10]);
	}
	else if(s8_flag == 2){
		smgshow_bit(1, 0xbf);
		if(dist_ave > 999)
			smgshow_bit(4, ~t_display[dist_ave / 1000]);
		if(dist_ave > 99)
			smgshow_bit(5, ~t_display[dist_ave / 100 % 10]);
		if(dist_ave > 9)
			smgshow_bit(6, ~dot_display[dist_ave / 10 % 10]);
		smgshow_bit(7, ~t_display[dist_ave % 10]);
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
void smgshow_time()
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

u8 pfc8591_adc(u8 channel)
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
	return dat;
}
void pfc8591_dac(u8 dat)
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
	if((dist > 0) && (dist <= 10))
		pfc8591_dac(51);
	else if((dist > 10) && (dist <= 80))
		pfc8591_dac((dist * 0.057 + 0.429) * 51);   //先扩大100倍，再缩小
	else
		pfc8591_dac(255);
}

void smgshow()
{
	if((s4_flag == 0) && (s5_flag == 0)){
		smgshow_time();
	}
	else if((s4_flag == 0) && (s5_flag == 1)){
		smgshow_dist();
	}
	else if((s4_flag == 0) && (s5_flag == 2)){
		dist_ave = 0;
		find_dist();
		smgshow_record();
	}
	else if((s4_flag == 1) && (s5_flag == 0))
		smgshow_gettime();
	else if((s4_flag == 1) && (s5_flag == 1))
		smgshow_distpara();
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
	if(P44 == 0){                           //S5
		Delay10ms();
		if(P44 == 0){
			while(P44 == 0){
				rd_ds1302();
				smgshow();
			}
			s5_flag++;
			if(s4_flag == 0){                      //时间数据界面
				if(s5_flag > 2)
					s5_flag = 0;
				if(s5_flag == 2)                   //进入数据记录界面
					s8_flag = 0;
			}else{
				if(s5_flag > 1)
					s5_flag = 0;
			}
		}
	}
	if(P42 == 0){                           //S9
		Delay10ms();
		if(P42 == 0){
			while(P42 == 0){
				rd_ds1302();
				smgshow();
			}
			if((s4_flag == 1) && (s5_flag == 0)){               //采集时间参数界面
				time_i++;
				if(time_i > 4)
					time_i = 0;
			}
			if((s4_flag == 1) && (s5_flag == 1)){               //距离参数界面
				setdist += 10;
				if(setdist > 80)
					setdist = 10;
			}
		}
	}
	P30 = 1; P31 = 1; P32 = 1; P33 = 0;
	P34 = 1; P35 = 1; P42 = 1; P44 = 1;
	if(P44 == 0){                           //S4
		Delay10ms();
		if(P44 == 0){
			while(P44 == 0){
				rd_ds1302();
				smgshow();
			}
			s4_flag = !s4_flag;
			s5_flag = 0;
			if(s4_flag == 0){                           //退出参数界面参数生效
				real_setdist = setdist;
				real_timei = time_i;
			}
		}
	}
	if(P42 == 0){                           //S8
		Delay10ms();
		if(P42 == 0){
			while(P42 == 0){
				rd_ds1302();
				smgshow();
			}
			if((s5_flag == 1) && (s4_flag == 0))                    //测距数据显示界面
				mode_flag = !mode_flag;
			if((s5_flag == 2) && (s4_flag == 0)){                   //数据记录显示界面
				s8_flag++;
				if(s8_flag > 2)
					s8_flag = 0;
			}
		}
	}
}

void led_work()
{
	if((s4_flag == 0) && (s5_flag == 0)){
		hc138_select(4);
		P00 = 0; P01 = 1; P02 = 1;
	}
	else if((s4_flag == 0) && (s5_flag == 1)){
		hc138_select(4);
		P00 = 1; P01 = 0; P02 = 1;
	}
	else if((s4_flag == 0) && (s5_flag == 2)){
		hc138_select(4);
		P00 = 1; P01 = 1; P02 = 0;
//		dist_ave = 0;
//		find_dist();
//		smgshow_record();
	}
	if(mode_flag){        //定时模式
		hc138_select(4);
		P03 = 1;
	}else{
		hc138_select(4);
		P03 = 0;
	}
	if(L5_flag){
		hc138_select(4);
		P04 = 0;
	}
	if(mode_flag == 0){
		if(pfc8591_adc(1) > 125){
			hc138_select(4);
			P05 = 0;
		}else{
			hc138_select(4);
			P05 = 1;
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
void delay(u8 t)
{
	while(t--){
		smgshow();
	}
}
void main()
{
	u8 i = 0;
	u8 measure_flag = 0;
	u8 cnt = 0;
	
	sys_init();
	wr_ds1302();
	while(1)
	{
		rd_ds1302();
		if(mode_flag){        //定时模式
			hc138_select(4);
			P03 = 1;
			if((ds1302_buff[0] % gettime[real_timei]) == 0){
				i++;
				if(i == 60){            //触发后仅测量一次
					i = 0;
					measure_dist();
				}
				if((dist > (real_setdist - 5)) && (dist < (real_setdist + 5)))
					cnt++;
				if(cnt == 3){
					cnt = 0;
					L5_flag = 1;
				}
			}
		}else{                //触发模式
			hc138_select(4);
			P03 = 0;
			pfc8591_adc(1);            //不加这一句的话仅ADC转换一次，结果不对
			if(measure_flag){          //触发后仅测量一次
				measure_flag = 0;
				if(pfc8591_adc(1) < 125){
					measure_dist();
				}
			}
			if(pfc8591_adc(1) > 125){
				measure_flag = 1;
			}
		}
		smgshow();
		if((s4_flag == 0) && (s5_flag == 2)){
			dist_ave = 0;
			find_dist();
			smgshow_record();         //数码管增强函数
		}
		key_scan();
		dac_out();
		led_work();
		delay(30);
	}
}