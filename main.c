#include "stm32f10x.h" 
#include "systick_time.h"
#include "gp_drive.h" 
#include "SPI_drive.h"
#include "n5110_drive.h"

/*
Pin kurulumu

SPI - 1
--> 
PA0 -->RST
PA1 --> DC
PA2 -->Ekran isigi
PA4 --> SS
PA5 --> SCLK
PA7 --> MOSI

3.3V --> Vcc
G --> Grd

PA3 -->IR sensor giris
PA9 -->resetleme butonu


harf ayari

# Ü
$ s
~ ç
| i
+ ü
& ö
! g


*/


unsigned char n5110_buffer[6][84];
char rpm_deger[9];
char donmesayisi[9];
uint16_t sayac = 0;
uint64_t rpmzaman=0;
uint8_t tus = 0;
int secim = 0;
uint32_t rpm = 0;
uint32_t rpmonceki = 0;


void int2char(int num, char str[])
{
char lstr[30];
int cnt = 0;
int div = 10;
int j = 0;

	
	
while( num >= div)
{
	lstr[cnt] = num % div + 0x30;
	num /= 10;
	cnt++;
}
	lstr[cnt] = num + 0x30;
for(j= cnt ; j >=0;j--)
{
	str[cnt-j] = lstr[j];
}

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////DWT SAYICI	
 
int main(void)
{

systick_init();
n5110_init(1);
//n5110_clear();

	RCC->APB2ENR |= (1<<2);  //  GPIOA Clock aktif
	GPIOA->CRL &= 0xF0FF00FF; /// A3 pini resetlendi
	GPIOA->CRL |= 0x08008300; /// A3 pini girisi push pull
 
////////////////////////////////////////////////////////////////////////////////////////////////////////////DWT SAYICI	
//register adresleri pointer olarak belirlendi
volatile unsigned int *DWT_CYCCNT   = (volatile unsigned int *)0xE0001004;     

volatile unsigned int *DWT_CONTROL  = (volatile unsigned int *)0xE0001000;     

volatile unsigned int *DWT_LAR      = (volatile unsigned int *)0xE0001FB0;     

volatile unsigned int *SCB_DEMCR    = (volatile unsigned int *)0xE000EDFC;
*DWT_LAR = 0xC5ACCE55; // (CM7) açildi
*SCB_DEMCR |= 0x01000000;
*DWT_CYCCNT = 0; // sayaci sifirla
*DWT_CONTROL |= 1 ; // sayaci ayarla
///////////////////////////////////////////////////////////////////////////////////////////////////////DWT SAYAICI

	
		__disable_irq();
	AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI3_PA; ///   A3 deki interrupt aktif edildi
	EXTI->IMR |= EXTI_IMR_MR3; ///  EXTI3 aktif
	EXTI->RTSR |= EXTI_RTSR_TR3; /// interrupt rising edge olarak ayarlandi
	NVIC_EnableIRQ(EXTI3_IRQn); // Genel interrupt 3 fonksiyonu calistirildi
	__enable_irq();

	
	
	
 ///////////////////////////////////////////////////////////////////////////////////////////Açilis
	update_str_buffer(2, 5,"  Ho$geldiniz  ",n5110_buffer);
	print_buffer(n5110_buffer);
	DelayMs(1500);
 ///////////////////////////////////////////////////////////////////////////////////////////Açilis
	while(1)
	{ 
	  clear_buffer(n5110_buffer);
    update_str_buffer(0, 0,"----Rpm &l~er---",n5110_buffer);	
    update_str_buffer(2, 0,"Rpm De!eri",n5110_buffer);

    tus=GPIOA->IDR & (1<<6);
		/////////////////////////////////////////////////////////////////// hesaplama
    rpm=600000/rpmzaman;
		/////////////////////////////////////////////////////////////////// hesaplama
		if (rpm-rpmonceki>10000 || rpm>599999)
{
	rpm=0;
}		
		///////////////////////////////////////////////////Ani artislari önlemek için
    int2char(rpm,rpm_deger);
		int2char(sayac,donmesayisi);
		update_str_buffer(5, 0,donmesayisi,n5110_buffer);
		update_str_buffer(4, 0,"D&nme say|s|",n5110_buffer);
		
			if(sayac<10){
		update_str_buffer(5, 5,"         ",n5110_buffer); 
		}
		else if(sayac<100){
		update_str_buffer(5, 10,"        ",n5110_buffer); 
		}
		else if(sayac<1000){
		update_str_buffer(5, 15,"          ",n5110_buffer); 
		}
		else if(sayac<10000){
		update_str_buffer(5, 25,"         ",n5110_buffer); 
		
		}
		
	if(tus==64 ){
		rpmzaman=0;
		sayac=0;
		update_str_buffer(3, 0,"0       ",n5110_buffer);
		update_str_buffer(4, 0," Resetleniyor...",n5110_buffer);
		}
	else{
	////tur sayisi 2 sn den fazla sürerse	
	if(rpmzaman<20000){
	
    int2char(rpm,rpm_deger);
		update_str_buffer(3, 0,rpm_deger,n5110_buffer);

		rpmonceki=rpm;
		
		if(rpm<10){
		update_str_buffer(3, 5," Rpm         ",n5110_buffer); 
		}
		else if(rpm<100){
		update_str_buffer(3, 10," Rpm        ",n5110_buffer); 
		}
		else if(rpm<1000){
		update_str_buffer(3, 15," Rpm        ",n5110_buffer); 
		}
		else if(rpm<10000){
		update_str_buffer(3, 25," Rpm        ",n5110_buffer); 
		}
		else if(rpm>100000){
		update_str_buffer(3, 30," Rpm        ",n5110_buffer);
		}
    update_str_buffer(1, 22,"-Aktif-",n5110_buffer);
    GPIOA->ODR |=0x0004;
	
  }
	else
	{
		update_str_buffer(1, 15,"-~ok yava$-",n5110_buffer);
	}
}

    print_buffer(n5110_buffer);
		DelayMs(100);
  }
}

void EXTI3_IRQHandler() // Interrupt Handler fonksiyonu Port A pin 3
{
	EXTI->PR |=1;
	sayac++;
	volatile unsigned int *DWT_CYCCNT   = (volatile unsigned int *)0xE0001004; //register adresi
	rpmzaman=*DWT_CYCCNT/10000;
	*DWT_CYCCNT = 0; // DWT sayaci sifirla
}
