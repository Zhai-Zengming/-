/*
 * @brief: AT24C02写读数据，将读出的数据修改后显示在数码管上
 *
 * @date : 2022/3/15
 *
 */


#include <STC15F2K60S2.H>
#include <absacc.h>
#include "iic.h"

u8 dat1 = 0, dat2 = 0, dat3 = 0;

u8 code smg_display[]={                       //标准字库
//   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71,
//black  -     H    J    K    L    N    o   P    U     t    G    Q    r   M    y
    0x00,0x40,0x76,0x1E,0x70,0x38,0x37,0x5C,0x73,0x3E,0x78,0x3d,0x67,0x50,0x37,0x6e,
    0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};    //0. 1. 2. 3. 4. 5. 6. 7. 8. 9. -1

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

	
void smg_delay(u16 t)
{
	while(t--);
}
	
u8 read_eeprom(u8 addr)
{
	u8 dat = 0;
	
	IIC_Start();
	IIC_SendByte(0xa0);
	IIC_WaitAck();
	IIC_SendByte(addr);    //要读取AT24C02的内部存储地址
	IIC_WaitAck();
	
	IIC_Start();
	IIC_SendByte(0xa1);
	IIC_WaitAck();
	dat = IIC_RecByte();
	IIC_SendAck(1);                 //非应答
	IIC_Stop();
	
	return dat;
}

void write_eeprom(u8 addr, u8 dat)
{
	IIC_Start();
	IIC_SendByte(0xa0);
	IIC_WaitAck();
	IIC_SendByte(addr);    //要写入AT24C02的内部存储地址
	IIC_WaitAck();
	IIC_SendByte(dat);
	IIC_WaitAck();
	IIC_Stop();
}


void read_write(void)
{
	Delay5ms();
	dat1 = read_eeprom(1);
	Delay5ms();
	dat2 = read_eeprom(2);
	Delay5ms();
	dat3 = read_eeprom(3);
	Delay5ms();
	
	dat1 += 1;
	dat2 += 1;
	dat3 += 1;
	
	if(dat1 > 10)
		dat1 = 0;
	if(dat2 > 10)
		dat2 = 0;
	if(dat3 > 10)
		dat3 = 0;
	
	write_eeprom(1, dat1);
	Delay5ms();
	write_eeprom(2, dat2);
	Delay5ms();
	write_eeprom(3, dat3);
	Delay5ms();
}

void smgshow_bit(u8 pos, u8 dat)
{
	XBYTE[0xc000] = 1 << pos;
	XBYTE[0xe000] = dat;
	smg_delay(500);
	XBYTE[0xe000] = 0xff;
}

void smgshow_AT24C02(void)
{
	smgshow_bit(0, ~smg_display[dat1 / 10]);
	smgshow_bit(1, ~smg_display[dat1 % 10]);
	
	smgshow_bit(2, ~smg_display[17]);
	
	smgshow_bit(3, ~smg_display[dat2 / 10]);
	smgshow_bit(4, ~smg_display[dat2 % 10]);
	
	smgshow_bit(5, ~smg_display[17]);
	
	smgshow_bit(6, ~smg_display[dat3 / 10]);
	smgshow_bit(7, ~smg_display[dat3 % 10]);
}

void main()
{
	read_write();
	while(1)
	{
		smgshow_AT24C02();
	}
}




