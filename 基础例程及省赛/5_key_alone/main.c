/*
 * @brief : 独立按键，
 *			按下S7进入S7状态，L1亮，S5,S4控制L3,L4,此时按S6没有反应（互锁），再次按下S7退出S7状态
 * 			按下S6进入S6状态，L2亮，S5,S4控制L5,L6,此时按S7没有反应（互锁），再次按下S6退出S6状态
 * @date  : 2022/3/10
 */
 
 
#include <reg52.h>

 

 void delay(u16 t)
 {
	 while(t--);
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

 
 void hc138_select(u8 n)
 {
	 switch(n)
	 {
		 case 4:
			 P2 = ((P2 & 0x1f) | 0x80); break;     //Y4输出低电平
		 case 5:
			 P2 = ((P2 & 0x1f) | 0xa0); break;     //Y5输出低电平
		 case 6:
			 P2 = ((P2 & 0x1f) | 0xc0); break;     //Y6输出低电平
		 case 7:
			 P2 = ((P2 & 0x1f) | 0xe0); break;     //Y7输出低电平
	 }
 }
 
 void system_init(void)    //初始化，关闭蜂鸣器，继电器等
 {
	 hc138_select(5);
	 P0 = 0;
	 hc138_select(4);
	 P0 = 0xff;
 }
 
 
 void key_scan(void)
 {
	 static u8 s7_state = 0;
	 static u8 s6_state = 0;
	 
	 if((P30 == 0) && (s6_state == 0))          //S7按下且不在S6状态,类似互锁
	 {
		 Delay10ms();
		 if(P30 == 0)
		 {
			 while(!P30);   //松手检测
			 
			 s7_state++;
			 P00 = 0;       //点亮L1
			 
			 if(s7_state > 1)
			 {
				 s7_state = 0;
				 P00 = 1;
			 }
		 }
	 }
	 
	 if(s7_state)           //S7状态下
	 {
		 if(P32 == 0)    //S5按下
		 {
			  Delay10ms();
			  if(P32 == 0)
			  {
				  while(P32 == 0)
					  P02 = 0;     //点亮L3(点动)
				  P02 = 1;
			  }
		 }
		 if(P33 == 0)    //S4按下
		 {
			  Delay10ms();
			  if(P33 == 0)
			  {
				  while(P33 == 0)
					  P03 = 0;     //点亮L4(点动)
				  P03 = 1;
			  }
		 }
	}
	 
	 if((P31 == 0) && (s7_state == 0))          //S6按下且不在S7状态,类似互锁
	 {
		 Delay10ms();
		 if(P31 == 0)
		 {
			 while(!P31);   //松手检测
			 
			 s6_state++;
			 P01 = 0;       //点亮L2
			 
			 if(s6_state > 1)
			 {
				 s6_state = 0;
				 P01 = 1;
			 }
		 }
	 }
	 
	 if(s6_state)         //S6状态下
	 {
		 if(P32 == 0)    //S5按下
		 {
			  Delay10ms();
			  if(P32 == 0)
			  {
				  while(P32 == 0)
					  P04 = 0;     //点亮L5(点动)
				  P04 = 1;
				  
			  }
		 }
		 if(P33 == 0)    //S4按下
		 {
			  Delay10ms();
			  if(P33 == 0)
			  {
				  while(P33 == 0)
					  P05 = 0;     //点亮L6(点动)
				  P05 = 1;
			  }
		 }
	 }
 }
 
 void main()
 {
	 system_init();
	 
	 while(1)
	 {
		 key_scan();
		 delay(10);
	 }
 }
 
 
 