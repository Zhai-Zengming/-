#ifndef __ONEWIRE_H
#define __ONEWIRE_H

#include "onewire.h"

void Delay_OneWire(unsigned int t);  //STC89C52RC
void Write_DS18B20(unsigned char dat);
unsigned char Read_DS18B20(void);
bit init_ds18b20(void);

//unsigned char rd_temperature(void);  //; ;

#endif
