/*
 * @brief : pcf8591 ADC转换，将转换完成的数字量显示在数码管上
 *
 * @date  : 2022/3/14
 *
 */


#include <STC15F2K60S2.H>
#include <absacc.h>
#include "iic.h"

float dat = 0;

u8 code smg_display[]={           //标准字库
//   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71,
//black  -     H    J    K    L    N    o   P    U     t    G    Q    r   M    y
    0x00,0x40,0x76,0x1E,0x70,0x38,0x37,0x5C,0x73,0x3E,0x78,0x3d,0x67,0x50,0x37,0x6e,
    0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};    //0. 1. 2. 3. 4. 5. 6. 7. 8. 9. -1

void pcf8591_getdat(addr)   //此参数见数据手册P6
{	
	IIC_Start();
	IIC_SendByte(0x90);    //pcf8591的地址，最后一位高读低写
	IIC_WaitAck();
	IIC_SendByte(addr);
	IIC_WaitAck();
	IIC_Stop();
	
	IIC_Start();
	IIC_SendByte(0x91);
	IIC_WaitAck();
	dat = IIC_RecByte();
	IIC_SendAck(0);
	IIC_Stop();
	
	dat = (dat * 500) / 255;   //转换成电压值

}

void smg_delay(void)
{
	u8 i = 100;
	while(i--);
}

void smgshow_bit(u8 pos, u8 dat)
{
	XBYTE[0xc000] = 1 << pos;
	XBYTE[0xe000] = dat;
	smg_delay();
	XBYTE[0xe000] = 0xff;
}

void smgshow_pcf(void)
{
	smgshow_bit(5, ~smg_display[(int)dat / 100]);
	smgshow_bit(6, ~smg_display[(int)dat / 10 % 10]);
	smgshow_bit(7, ~smg_display[(int)dat % 10]);
}

void main()
{
	while(1)
	{
		pcf8591_getdat(0x43);
//		pcf8591_getdat(0x41);     选择AIN1
		smgshow_pcf();
	}
}

