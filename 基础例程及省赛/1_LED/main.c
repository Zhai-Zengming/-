/*
 * @brief: led����ƣ�8·led��˸���Σ����ε�����������Ϩ��
 *  
 * @date : 2022/3/10
 *
 */

#include <reg52.h>

void delay(unsigned int t)
{
	unsigned int i = 60000;
	while(t--);
	while(i--);
	
}

void led_running(void)
{
	unsigned char i,j;
	
	P27 = 1;     //138 C
	P26 = 0;     //138 B
	P25 = 0;     //138 A
	
	for(i = 0; i < 3; i++)    //8·led��˸����
	{
		P0 = 0;
		delay(60000);
		P0 = 0xff;
		delay(60000);
	}
	
	for(j = 0; j < 8; j++)    //8·led���ε�����������Ϩ��
	{
		P0 = (0xff << j);
		delay(30000);
	}
	
	for(j = 0; j < 8; j++)
	{
		P0 = ~(0xff << j);
		delay(30000);
	}
}


void main()
{
	while(1)
	{
		led_running();
	}
}