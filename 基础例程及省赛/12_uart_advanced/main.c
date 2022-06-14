/*
 * @brief : ���ڽ��ף���Ŀ����Ҫ���note
 *
 * @date  : 2022/3/12
 */


#include <STC15F2K60S2.H>

u8 rcv_dat = 0;

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

/* ��ʼ���ر�led,�������� */
void system_init(void)
{
	hc138_select(5);
	P0 = 0;
	hc138_select(4);
	P0 = 0xff;
}

void uart_init(void)
{
	TMOD = ((TMOD & 0x0f) | 0x20);
	TH1 = 0xfd;
	TL1 = TH1;
	TR1 = 1;
	
	SCON = 0x50;
	ES = 1;
	EA = 1;
	AUXR = 0x00;
}

void uart_sendbyte(u8 dat)
{
	SBUF = dat;
	while(TI == 0);
	TI = 0;
}

void uart_sendstr(u8 *str)
{
	while(*str != '\0')
	{
		uart_sendbyte(*str);
		str++;
	}
}


/* ��Ƭ��ִ����λ�������� */
void execute_command(u8 command)
{
	switch(command & 0xf0)
	{
		case 0xa0:
			P0 = ((P0 & 0xf0) | ((~command) & 0x0f));
			rcv_dat = 0;           //ִ������������㣬��ֹ�ظ�ִ��
		break;
		
		case 0xb0:
			P0 = ((P0 & 0x0f) | (~command << 4));
			rcv_dat = 0;           //ִ������������㣬��ֹ�ظ�ִ��
		break;
		
		case 0xc0:
			uart_sendstr("The system is running...\r\n");
			rcv_dat = 0;           //ִ������������㣬��ֹ�ظ�ִ��
		break;
	}
}

void main()
{
	system_init();
	uart_init();
	uart_sendstr("welcome zhaizm! ף�����ճ�ΪǶ��ʽ���У�\r\n");

	while(1)
	{
		execute_command(rcv_dat);
	}
}

void uart() interrupt 4
{
	if(RI == 1)
	{
		rcv_dat = SBUF;
		RI = 0;
	}
}

