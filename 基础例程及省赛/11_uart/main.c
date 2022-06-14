/*
 * @brief : 串口通讯
 * 
 * @date  : 2022/3/12
 */


#include <STC15F2K60S2.H>

u8 rcv_dat = 0;

void uart_init(void)
{
	/* 定时器1产生波特率，八位自动重装 */
	TMOD = ((TMOD & 0x0f) | 0x20);
	TH1 = 0xfd;      //由公式计算得到
	TL1 = TH1;
	TR1 = 1;
	
	SCON = 0x50;    //串口相关寄存器
	ES = 1;         //允许串口中断
	
	EA = 1;
	AUXR = 0;
}

void uart_sendbyte(u8 dat)
{
	SBUF = dat;
	while(TI == 0);
	TI = 0;
}

void main()
{
	uart_init();
	uart_sendbyte(0x5a);
	uart_sendbyte(0xa5);
	while(1);
}

void uart() interrupt 4
{
	if(RI == 1)
	{
		RI = 0;
		rcv_dat = SBUF;
		uart_sendbyte(rcv_dat + 1);
	}
}

