//generare una sinusoide che abbia un ampiezza di 1 volt con offset di 1 volt
//Vsin= 1 + 1*(2pi*i/100) -> decidiamo di avere 100 campioni ad esempio
//1 è l'offset mentre l'ampiezza oscilla tra [-0,5 e 5]
//i varia tra 0 e 99

#include<math.h>
#include "stm32f30x.h"

#define N 100 
#define PI 3.14

short int vett[N]; //vettore per la codifica dei valori della sinusoide
//Il DAC richiede in ingresso un codice
void gen_sin(float);//prototipo

void main(){
   
	//abilito le periferiche interessate
	 RCC->AHBENR |= RCC_AHBENR_GPIOAEN; //abilito GPIOA
	 RCC->APB1ENR |= RCC_APB1ENR_DAC1EN; //abilito DAC1
   RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; //abilito TIM2
  
   gen_sin(0.5); //genero la codifica della sinusoide 
   
   GPIOA->MODER |= GPIOA_MODER_MODER4; //PA4 in uscita analogica per evitare il sovraccarico del DAC
   
   DAC->CR |= DAC_CR_EN1; //abilito il canale 1
   
   //aggancio la generazione del campione a un update event (ogni 0.5 sec ad esempio)
   TIM2->ARR = 36000000; //0.5 secondi
   TIM2->CNT = 0; //azzero il conteggio
   TIM2->CR1 = TIM_CR1_CEN; //avvio il conteggio

	
   
   for(int i=0; i<N; i++){
	   //scrivo il codice nel DHR
	   DAC1->DHR12R1 = vett[i];
	   while((TIM2->SR & TIM_SR_UIF) != TIM_SR_UIF); //attesa UIF=1->UEV avvenuto
	   TIM2->SR &= ~TIM_SR_UIF; //UIF=0;
	   for(int j=0; j<1000; j++); //attesa generazione di tensione
   }
   
   while(1);
}

void gen_sin(float ampiezza){
  float Vsin;
  for(int i=0; i<N; i++){
    Vsin = 1 + ampiezza*sin(2*PI*i/N);
    vett[i] = (short int) (Vsin*4095.0/3.0); //codifica dei valori
  }
}
