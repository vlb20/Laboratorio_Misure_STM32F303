#include<math.h>
#include "stm32f30x.h"

#define N 100 
#define PI 3.14

short int vett[N]; //vettore per la codifica dei valori della sinusoide
//Il DAC richiede in ingresso un codice
void gen_sin(float);//prototipo
void abilitazione_periferiche();
void setup_DMA2();
void setup_DAC();
void setup_TIM2();
void disabilitazione_periferiche();

void main(){
  
	gen_sin(0.5); //genero la codifica della sinusoide 

	//abilito le periferiche interessate
	abilitazione_periferiche();

	GPIOA->MODER |= GPIOA_MODER_MODER4; //PA4 in uscita analogica per evitare il sovraccarico del DAC
	
	setup_DMA2();
	setup_DAC();
	setup_TIM2();

  TIM2->CNT = 0; //azzero il conteggio
  TIM2->CR1 = TIM_CR1_CEN; //avvio il conteggio
	 
 /* for(int i=0; i<N; i++){
	  //scrivo il codice nel DHR
    DAC1->DHR12R1 = vett[i];
	  while((TIM2->SR & TIM_SR_UIF) != TIM_SR_UIF); //attesa UIF=1->UEV avvenuto
	  TIM2->SR &= ~TIM_SR_UIF; //UIF=0;
	  for(int j=0; j<1000; j++); //attesa generazione di tensione
   }*/

   disabilitazione_periferiche();

   while(1);
}

void gen_sin(float ampiezza){
  float Vsin;
  for(int i=0; i<N; i++){
    Vsin = 1 + ampiezza*sin(2*PI*i/N);
    vett[i] = (short int) (Vsin*4095.0/3.0); //codifica dei valori
  }
}

void abilitazione_periferiche(){
	 RCC->AHBENR |= RCC_AHBENR_GPIOAEN; //abilito GPIOA
	 RCC->APB1ENR |= RCC_APB1ENR_DAC1EN; //abilito DAC1
   RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; //abilito TIM2
	 RCC->AHBENR |= RCC_AHBENR_DMA2EN; //abilitazione DMA2
	 //Per non complicarci la vita usiamo il DMA2 per il DAC, 
	 //così evitiamo il remapping
}

void setup_DMA2(){
	DMA2_Channel3->CPAR = (uint32_t)&DAC1->DHR12R1; //indirizzo del DHR
	DMA2_Channel3->CMAR = (uint32_t)vett; //indirizzo del vettore
	DMA2_Channel3->CNDTR = N; //numero trasferimenti
	DMA2_Channel3->CCR |= DMA_CCR_MSIZE_0;  //MSIZE = 16 bit
	DMA2_Channel3->CCR |= DMA_CCR_PSIZE_0;  //PSIZE = 16 bit
	DMA2_Channel3->CCR |= DMA_CCR_MINC;    //incremento automatico ind. di memoria
	DMA2_Channel3->CCR |= DMA_CCR_CIRC;    //modalità circolare
	DMA2_Channel3->CCR |= DMA_CCR_DIR;    //DIR=1: memoria -> periferica
	DMA2_Channel3->CCR |= DMA_CCR_EN;       //abilitazione canale 3
}

void disabilitazione_periferiche(){
	DAC1->CR &= ~DAC_CR_EN1;
	TIM2_CR1 &= ~TIM_CR1_CEN;
	DMA2_Channel3->CCR &= ~DMA_CCR_EN;
}

void setup_DAC(){
		DAC->CR |= DAC_CR_TEN1; //abilitazione trigger
		DAC->CR |= DAC_CR_TSEL1_2; //TSEL=100: trigger TIM2
		DAC->CR |= DAC_CR_EN1;  //abilitazione canale 1
		DAC->CR |= DAC_CR_DMAEN1; //abilitazione modalità DMA
}

void setup_TIM2(){
	//aggancio la generazione del campione a un update event (ogni 0.5 sec ad esempio)
  TIM2->ARR = 36000000; //0.5 secondi
	TIM->CR2 = TIM_CR2_MMS_1; //MMS = 010 per far scattare il trigger
}
