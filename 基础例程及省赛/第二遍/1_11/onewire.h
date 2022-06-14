#ifndef __ONEWIRE_H
#define __ONEWIRE_H

//#include "reg52.h"
#include <STC15F2K60S2.H>

sbit DQ = P1^4;  

//unsigned char rd_temperature(void);  
bit init_ds18b20(void);
void Write_DS18B20(unsigned char dat);
unsigned char Read_DS18B20(void);


#endif
