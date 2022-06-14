#include <STC15F2K60S2.H>
#include <intrins.h>
typedef unsigned int u16;
typedef unsigned char u8;
sbit TX = P1^0;
sbit RX = P1^1;
float distance;
u8 code t_display[]={                       //标准字库
//   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71,
//black  -     H    J    K    L    N    o   P    U     t    G    Q    r   M    y
    0x00,0x40,0x76,0x1E,0x70,0x38,0x37,0x5C,0x73,0x3E,0x78,0x3d,0x67,0x50,0x37,0x6e,
    0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};    //0. 1. 2. 3. 4. 5. 6. 7. 8. 9. -1

void smgshow_dist();
	
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
			smgshow_dist();             //added
		} while (--j);
	} while (--i);
}


void sendwave()          //发出8个40Khz的脉冲
{
	u8 i;
	
	for(i = 0; i < 8; i++){
		TX = 1;
		Delay12us();
		TX = 0;
		Delay12us();
	}
}

void tim0_init()
{
	TMOD &= 0xf0;         //tim0十六位自动重装
	TH0 = 0x00;
	TL0 = 0x00;
	TR0 = 0;
}

void measure_distance()
{
	u16 t;
	
	tim0_init();
	sendwave();
	TR0 = 1;
	while((RX == 1) && (TF0 == 0));      //TF0是TH0和TL0溢出标志
	TR0 = 0;                             //测量结束停止计时
	
	if(TF0 == 0){                        //说明测距正常结束
		t = TH0;
		t = (t << 8) | TL0;
		distance = t * 0.017;
	}else{
		TF0 = 0;
		distance = 999;
	}
	
	Delay60ms();                         //两次测量之间加上延迟
}

void smgshow_dist()
{
	smgshow_bit(5, ~t_display[(int)distance / 100]);
	smgshow_bit(6, ~t_display[(int)distance / 10 % 10]);
	smgshow_bit(7, ~t_display[(int)distance % 10]);
}

void main()
{
	while(1)
	{
		measure_distance();
	}
}


/* 小蜜蜂
#include "reg52.h"
#include "absacc.h"
#include "intrins.h"
 
sbit TX = P1^0;
sbit RX = P1^1;
 
unsigned int distance = 0;
 
unsigned char code SMG_duanma[18]=
		{0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,
		 0x88,0x80,0xc6,0xc0,0x86,0x8e,0xbf,0x7f};
 
void DelaySMG(unsigned int t)
{
	while(t--);
}
 
void DisplaySMG_Bit(unsigned char pos, unsigned char value)
{
	XBYTE[0xE000] = 0xFF;
	XBYTE[0xC000] = 0x01 << pos;
	XBYTE[0xE000] = value;
}
 
void Display_Distance()
{
	if(distance == 999)
	{
		DisplaySMG_Bit(0, SMG_duanma[15]);			//超出测量范围标志：F
		DelaySMG(500);
	}
	else
	{
		DisplaySMG_Bit(5, SMG_duanma[distance / 100]);
		DelaySMG(500);
		DisplaySMG_Bit(6, SMG_duanma[(distance % 100) / 10]);
		DelaySMG(500);
		DisplaySMG_Bit(7, SMG_duanma[distance % 10]);
		DelaySMG(500);
	}
}
 
void Delay12us()                  //@12.000MHz 延时12us
{
	unsigned char i;
 
	_nop_();
	_nop_();
	i = 33;
	while (--i);
}
 
void Send_Wave()                 //产生8个40KHx超声波信号
{
	unsigned char i;
	for(i = 0; i < 8; i++)
	{
		TX = 1;
		Delay12us();	
		TX = 0;
		Delay12us();
	}
}
 
void Measure_Distance()	        //超声波测距
{
	unsigned int time = 0;
	
	TMOD &= 0x0f;	            //定时器1模式0，13位，最大8192个计数脉冲								
	TL1 = 0x00;										
	TH1 = 0x00;		
	
	Send_Wave();		        //发送超声波信号							
	TR1 = 1;            //启动定时器						
	while((RX == 1) && (TF1 == 0));    //等待超声波信号返回或者等到测量超出范围
	TR1 = 0;            //停止定时器				
	
	if(TF1 == 0)	            //正常测量范围							
	{
		time = TH1;									
		time = (time << 8) | TL1;		
		distance = ((time / 10) * 17) / 100 + 3;
	}
	else                        //超出测量范围			
	{
		TF1 = 0;
		distance = 999;
	}
}
 
void Delay(unsigned char n)        //数码管显示增强
{
	while(n--)
	{
		Display_Distance();
	}
}
 
void main()
{
	while(1)
	{
		Measure_Distance();
		Delay(10);
	}
}
*/