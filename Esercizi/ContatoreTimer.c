#include "stm32f30x.h"

unsigned char c=0; //contatore 

void main(){
	RCC->AHBEN |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOEEN; //abilito le porte PA e PE
	GPIOE->MODER = 0x5555000; //abilito tutti i led in output (PE8-PE15)
	GPIOA->MODER &= ~GPIO_MODER_MODER0; //abilito PA0 come input digitale

	while(1){
		if(GPIOA->IDR & GPIO_IDR_0) == GPIO_IDR_0){ //se USER è premuto
			while((GPIOA->IDR & GPIO_IDR_0) == GPIO_IDR_0); //attende mentre USER è premuto
			c++;
			GPIOE->ODR = (c<<8);   //cont traslato a sinistra 
			//scriviamo il valore di cont nei bit dal 15° all' 8°
			//assegnamo bit a bit -> trasforma il cont in una parola codice binaria
		}
	}
}
