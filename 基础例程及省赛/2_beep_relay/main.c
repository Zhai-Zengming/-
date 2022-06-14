/*
 * @brief : led闪烁三次后，蜂鸣器叫，继电器吸合，
 * 			然后蜂鸣器和继电器关闭，led再依次亮灭
 * @date  : 2022/3/10
 */
 
 
 #include <reg52.h>
 
 void delay(u16 t)
 {
	 u16 i = 60000;
	 while(i--);
	 while(t--);
 }
 
 void hc138_select(u8 n)
 {
	 switch(n)
	 {
		 case 4:
			 P2 = ((P2 & 0x1f) | 0x80); break;     //Y4输出低电平
		 case 5:
			 P2 = ((P2 & 0x1f) | 0xa0); break;     //Y5输出低电平
		 case 6:
			 P2 = ((P2 & 0x1f) | 0xc0); break;     //Y6输出低电平
		 case 7:
			 P2 = ((P2 & 0x1f) | 0xe0); break;     //Y7输出低电平
	 }
 }
 
 void system_init(void)    //初始化，关闭蜂鸣器，继电器等
 {
	 hc138_select(5);
	 P0 = 0;
 }
 
 void system_work(void)
 {
	 u8 i;
	 
	 hc138_select(4);
	 for(i = 0; i < 3; i++)  //led闪烁三次
	 {
		 P0 = 0;
		 delay(60000);
		 P0 = 0xff;
		 delay(60000);
	 }
	 
	 hc138_select(5);
	 P06 = 1;         //打开蜂鸣器
	 P04 = 1;         //打开继电器
	 delay(60000);
	 P04 = 0;         //关闭继电器
	 P06 = 0;         //关闭蜂鸣器
	 
	 hc138_select(4);
	 for(i = 0; i < 8; i++)  //led依次点亮
	 {
		 P0 = (0xff << i);
		 delay(30000);
	 }
	 for(i = 0; i < 8; i++)  //led依次熄灭
	 {
		 P0 = ~(0xff << i);
		 delay(30000);
	 }
 }
 
 void main()
 {
	 system_init();
	 while(1)
	 {
		 system_work();
	 }
 }