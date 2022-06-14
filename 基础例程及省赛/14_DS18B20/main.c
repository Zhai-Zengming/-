/*
 * @brief : ds18b20��ȡ�¶���ʾ���������
 *
 * @date  : 2022/3/13
 *
 */

#include <STC15F2K60S2.H>
#include "onewire.h"
#include <absacc.h>

u8 templ, temph;
u16 temp;

u8 code smgnodot_tab[] = {               //��׼�ֿ�
//   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71,
//black  -     H    J    K    L    N    o   P    U     t    G    Q    r   M    y
    0x00,0x40,0x76,0x1E,0x70,0x38,0x37,0x5C,0x73,0x3E,0x78,0x3d,0x67,0x50,0x37,0x6e};

	//0. 1. 2. 3. 4. 5. 6. 7. 8. 9. -1
u8 code smgdot_tab[] = {
    0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};

	
void smg_showtemp(void);

	
void delay(void)
{
	u16 t = 1000;
	while(t--)
	{
		smg_showtemp();
	}
}
	
void get_temp(void)
{
	init_ds18b20();
	Write_DS18B20(0xcc);   //��Ϊ������ֻ��һ��ds18b20,���Բ���ҪѰַ������ROMָ��
	Write_DS18B20(0x44);   //��ʼ�¶�ת��
	delay();
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0xbe);   //��ʼ��
	
	templ = Read_DS18B20();
	temph = Read_DS18B20();
	init_ds18b20();
	
	temp = temph;
	temp = ((temp << 8) | templ);
	if((temp & 0xf800) == 0x0000)    //��ʾ�¶�Ϊ��
	{
		/* temp����4����������ȡ�����ٷŴ�ʮ�����ڼ��㣬
		 * Ȼ�����С������
		 */
		temp = (temp >> 4) * 10 + (templ & 0x0f) * 10 * 0.0625;
	}
}
	
void smg_delay(void)
{
	u8 i = 500;
	while(i--);
}
	
void showsmg_bit(u8 pos, u8 dat)
{
	XBYTE[0xe000] = 0xff;   //��Ӱ
	XBYTE[0xc000] = (1 << pos);
	XBYTE[0xe000] = dat;
	smg_delay();
}

void smg_showtemp(void)
{
	showsmg_bit(5, ~smgnodot_tab[temp/100]);
	showsmg_bit(6, ~smgdot_tab[temp/10%10]);
	showsmg_bit(7, ~smgnodot_tab[temp%10]);
}

void main(void)
{
	XBYTE[0x8000] = 0xff;  //�ص�
	while(1)
	{
		get_temp();
		smg_showtemp();
	}
}