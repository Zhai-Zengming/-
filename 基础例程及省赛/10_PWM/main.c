/*
 * @brief : PWM 控制LED0的亮度0-->10%-->20%-->90%，按键S7控制
 *
 * @date  : 2022/3/11
 */


#include <reg52.h>

u8 pwm_duty = 0;

void hc138_select(u8 n)
{
	switch(n)
	{
		case 4:
			P2 = ((P2 & 0x1f) | 0x80);  break;
		case 5:
			P2 = ((P2 & 0x1f) | 0xa0);  break;
		case 6:
			P2 = ((P2 & 0x1f) | 0xc0);  break;
		case 7:
			P2 = ((P2 & 0x1f) | 0xe0);  break;
	}
}

void timer0_init(void)
{
	TMOD = ((TMOD & 0xf0) | 0x01);
	TH0 = (65535 - 100) / 256;
	TL0 = (65535 - 100) % 256;
	TR0 = 1;
	ET0 = 1;
	EA = 1;
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

u8 key_state = 0;
void key_scan(void)
{
	if(P30 == 0)
	{
		Delay10ms();
		if(P30 == 0)
		{
			while(P30 == 0);       //松手检测
			key_state++;
			if(key_state == 4)
				key_state = 0;
			switch(key_state)
			{
				case 0:
					pwm_duty = 0;   break;
				case 1:
					pwm_duty = 10;  break;
				case 2:
					pwm_duty = 20;  break;
				case 3:
					pwm_duty = 90;  break;
			}
		}
	}
	
}

void main()
{
	hc138_select(4);
	timer0_init();
	
	while(1)
	{
		key_scan();
	}
}

u8 cnt = 0;

void tim0() interrupt 1
{
	TH0 = (65535 - 100) / 256;
	TL0 = (65535 - 100) % 256;
	cnt++;
	if(cnt == 100)
		cnt = 0;
	if(cnt < pwm_duty)        //小于占空比亮灯
		P00 = 0;
	if(cnt >= pwm_duty)        //大于占空比灭灯
		P00 = 1;
}

