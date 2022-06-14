/*
 * @brief : led��˸���κ󣬷������У��̵������ϣ�
 * 			Ȼ��������ͼ̵����رգ�led����������
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
			 P2 = ((P2 & 0x1f) | 0x80); break;     //Y4����͵�ƽ
		 case 5:
			 P2 = ((P2 & 0x1f) | 0xa0); break;     //Y5����͵�ƽ
		 case 6:
			 P2 = ((P2 & 0x1f) | 0xc0); break;     //Y6����͵�ƽ
		 case 7:
			 P2 = ((P2 & 0x1f) | 0xe0); break;     //Y7����͵�ƽ
	 }
 }
 
 void system_init(void)    //��ʼ�����رշ��������̵�����
 {
	 hc138_select(5);
	 P0 = 0;
 }
 
 void system_work(void)
 {
	 u8 i;
	 
	 hc138_select(4);
	 for(i = 0; i < 3; i++)  //led��˸����
	 {
		 P0 = 0;
		 delay(60000);
		 P0 = 0xff;
		 delay(60000);
	 }
	 
	 hc138_select(5);
	 P06 = 1;         //�򿪷�����
	 P04 = 1;         //�򿪼̵���
	 delay(60000);
	 P04 = 0;         //�رռ̵���
	 P06 = 0;         //�رշ�����
	 
	 hc138_select(4);
	 for(i = 0; i < 8; i++)  //led���ε���
	 {
		 P0 = (0xff << i);
		 delay(30000);
	 }
	 for(i = 0; i < 8; i++)  //led����Ϩ��
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