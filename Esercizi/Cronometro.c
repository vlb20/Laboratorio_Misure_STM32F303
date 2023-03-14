#include <stdio.h> //per la printf
#include <math.h>
#include "stm32f30x.h"

#define Tck 125*pow(10,-9) //Tck=125ns

float durata = 0;

void main(){
	int Ncont=0;
	
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;  //per abilitare la porta A
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; //per abilitare il tim2

	while(1){
		if((GPIOA->IDR & GPIO_IDR_0) == GPIO_IDR_0){ //se USER è premuto
			TIM2->CR1 |= TIM_CR1_CEN;  //abilito il conteggio
			TIM2->CNT = 0; //azzero il conteggio
	}
		while((GPIOA->IDR & GPIO_IDR_0) == GPIO_IDR_0);//attende mentre user è premuto
		
		TIM2->CR1 &= ~TIM_CR1_CEN;
		Ncont = TIM2->CNT;
		durata = (float)Ncont*Tck;
		
		printf("durata: %f secondi", durata);		
}
