/*
 * @brief : ����������
 *			����S7����S7״̬��L1����S5,S4����L3,L4,��ʱ��S6û�з�Ӧ�����������ٴΰ���S7�˳�S7״̬
 * 			����S6����S6״̬��L2����S5,S4����L5,L6,��ʱ��S7û�з�Ӧ�����������ٴΰ���S6�˳�S6״̬
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
			 P2 = ((P2 & 0x1f) | 0x80); break;     //Y4����͵�ƽ
		 case 5:
			 P2 = ((P2 & 0x1f) | 0xa0); break;     //Y5����͵�ƽ
		 case 6:
			 P2 = ((P2 & 0x1f) | 0xc0); break;     //Y6����͵�ƽ
		 case 7:
			 P2 = ((P2 & 0x1f) | 0xe0); break;     //Y7����͵�ƽ
	 }
 }
 
 void system_init(void)    //��ʼ�����رշ��������̵�����
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
	 
	 if((P30 == 0) && (s6_state == 0))          //S7�����Ҳ���S6״̬,���ƻ���
	 {
		 Delay10ms();
		 if(P30 == 0)
		 {
			 while(!P30);   //���ּ��
			 
			 s7_state++;
			 P00 = 0;       //����L1
			 
			 if(s7_state > 1)
			 {
				 s7_state = 0;
				 P00 = 1;
			 }
		 }
	 }
	 
	 if(s7_state)           //S7״̬��
	 {
		 if(P32 == 0)    //S5����
		 {
			  Delay10ms();
			  if(P32 == 0)
			  {
				  while(P32 == 0)
					  P02 = 0;     //����L3(�㶯)
				  P02 = 1;
			  }
		 }
		 if(P33 == 0)    //S4����
		 {
			  Delay10ms();
			  if(P33 == 0)
			  {
				  while(P33 == 0)
					  P03 = 0;     //����L4(�㶯)
				  P03 = 1;
			  }
		 }
	}
	 
	 if((P31 == 0) && (s7_state == 0))          //S6�����Ҳ���S7״̬,���ƻ���
	 {
		 Delay10ms();
		 if(P31 == 0)
		 {
			 while(!P31);   //���ּ��
			 
			 s6_state++;
			 P01 = 0;       //����L2
			 
			 if(s6_state > 1)
			 {
				 s6_state = 0;
				 P01 = 1;
			 }
		 }
	 }
	 
	 if(s6_state)         //S6״̬��
	 {
		 if(P32 == 0)    //S5����
		 {
			  Delay10ms();
			  if(P32 == 0)
			  {
				  while(P32 == 0)
					  P04 = 0;     //����L5(�㶯)
				  P04 = 1;
				  
			  }
		 }
		 if(P33 == 0)    //S4����
		 {
			  Delay10ms();
			  if(P33 == 0)
			  {
				  while(P33 == 0)
					  P05 = 0;     //����L6(�㶯)
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
 
 
 