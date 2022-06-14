/*
 * @brief : 先向ds1302写入时间数据，再读出显示在数码管上
 *
 * @date  : 2022/3/13
 */

#include <STC15F2K60S2.H>
#include <ds1302.h>
#include <absacc.h>

/* 要读和写的地址 */
u8 write_ds1302_addr[] = {0x80, 0x82, 0x84, 0x86, 0x88, 0x8a, 0x8c};
u8 read_ds1302_addr[] = {0x81, 0x83, 0x85, 0x87, 0x89, 0x8b, 0x8d};

/* 读写缓冲区 */
u8 ds1302_buff[7] = {0x30, 0x59, 0x23, 0x13, 0x03, 0x06, 0x22};

u8 code t_display[]={           //标准字库
//   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71,
//black  -     H    J    K    L    N    o   P    U     t    G    Q    r   M    y
    0x00,0x40,0x76,0x1E,0x70,0x38,0x37,0x5C,0x73,0x3E,0x78,0x3d,0x67,0x50,0x37,0x6e,
    0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};    //0. 1. 2. 3. 4. 5. 6. 7. 8. 9. -1

void smg_delay(void)
{
	u8 i = 100;
	while(i--);
}

void showsmg_bit(u8 pos, u8 dat)
{
	XBYTE[0xc000] = 1 << pos;
	XBYTE[0xe000] = dat;
	smg_delay();
	P0 = 0xff;    //消影
}

/* 向ds1302写入数据 */
void ds1302_config(void)
{
	u8 i;
	Write_Ds1302_Byte(0x8e, 0x00);   //关闭写保护
	for(i = 0; i < 7; i++)
	{
		Write_Ds1302_Byte(write_ds1302_addr[i], ds1302_buff[i]);
	}
	Write_Ds1302_Byte(0x8e, 0x80);   //打开写保护
}

void read_ds1302(void)
{
	u8 i;
	for(i = 0; i < 7; i++)
	{
		ds1302_buff[i] = Read_Ds1302_Byte(read_ds1302_addr[i]);
	}
}

void showsmg_ds1302(void)
{
	showsmg_bit(0, ~t_display[ds1302_buff[2] / 16]);  //十六进制除以16类似十进制除以10
	showsmg_bit(1, ~t_display[ds1302_buff[2] % 16]);  //时
	
	showsmg_bit(2, ~t_display[17]);
	
	showsmg_bit(3, ~t_display[ds1302_buff[1] / 16]);  //分
	showsmg_bit(4, ~t_display[ds1302_buff[1] % 16]);
	
	showsmg_bit(5, ~t_display[17]);
	
	showsmg_bit(6, ~t_display[ds1302_buff[0] / 16]);  //秒
	showsmg_bit(7, ~t_display[ds1302_buff[0] % 16]);
	
	showsmg_bit(7, 0xff);    //关闭第七位数码管防止它太亮，具体见note
}

void main()
{
	ds1302_config();
	while(1)
	{
		read_ds1302();
		showsmg_ds1302();
	}
}