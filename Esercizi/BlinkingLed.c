#include<stm32f30x.h>


int stato=0; //contatore 

void main(){
	RCC->AHBENR |= RCC_AHBENR_GPIOEEN; //abilito la porta e
	RCC->APB1ENR |= RCC_APB1ENR_TIME2EN; //abilito il timer2 (non usiamo il prescaler)
	GPIOE->MODER = 0x55550000 //abilito tutti i led in output
	
	TIM2->CR1 |= 1; //Alzo il bit CEN (Counter Enable) per abilitare il conteggio
	//La frequenza di clock fornita dalla scheda è di 8 MHz, 
	//cioè 8 milioni di conteggi al secondo. Poiché vogliamo che il contatore cambi 
	//stato ogni secondo, il contatore deve arrivare a 8 milioni.
	TIM2->ARR = 8000000; //auto reload register
	TIM2->CNT=0;
//Il CNT contiene il valore del conteggio, che comincia a contare quando viene abilitato.
	
	while(1){
		//controlliamo se l'uif c'è stato(primo bit del SR)
		if(TIM2->SR&1 == 1){ //se uif è alzato è avvenuto il reset del timer)
			TIM2->SR &= ~0x00000001; //lo abbassiamo (~TIM_SR_UIF)
		
			if(stato==0){ //se i led sono spenti
				GPIOE->ODR|=0x000000; //accendo i led
				stato=1; //segnalo che sono accesi
			else{ //se il led è acceso (stato==1)
				GPIOE->ODR=0x00000000; //spengo i led 
				stato=0; //segnalo che sono spenti
			}
		}
	}
}
