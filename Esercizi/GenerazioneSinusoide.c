#include <stm32f30x.h>
#include <stdio.h>

float ris;
int eoc = 0;

void main(){
	//configurazione ADC
	RCC->AHBENR |= RCC_AHBENR_ADC12EN; //abilito la coppia di ADC12
	ADC1_2->CCR |= ADC12_CCR_CKMODE_0; //CKMODE=01, clock del bus AHB

	//Abilitazione GPIOA (per il pulsante USER)
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	//PA0 in modalità analogica (MODER0=11)
	GPIOA->MODER |= GPIO_MODER_MODER0;

	//abilitiamo generatore di tensione di riferimento pulita per non avere incertezze-> regolatore di tensione
	ADC1->CR &= ~ADC_CR_ADVREGEN_1; //ADVREGEN = 10->00 (reset)
	ADC1->CR |= ADC_CR_ADVREGEN_0;  //ADVREGEN = 00->01 (abilitazione)
	for(int i=0; i<1000; i++);      //attesa di 10 µs

	//verifica che l'ADC sia disabilitato (ADEN==0)
	ADC1->CR &= ~ADC_CR_ADEN; 

	//modalità di acquisizione single-ended 
	ADC1->DIFSEL &= ADC_DIFSEL_DIFSEL_1; //DIFSEL=0->single ended

	//CALIBRAZIONE 
	ADC1->CR &= ~ADC_CR_ADCALDIF; //Calibrazione single-ended (ADCALDIF = 0)
	ADC1->CR |= ADC_CR_ADCAL; //ADCAL=1, avvio la calibrazione
	while((ADC1->CR & ADC_CR_ADCAL) == ADC_CR_ADCAL); //attendo ADCAL=0 (fine calib)

	//ABILITAZIONE ADC
	ADC1->CR |= ADC_CR_ADEN;  //ADEN=1, abilitazione ADC
	while((ADC1->ISR & ADC_ISR_ADRD) != ADC_ISR_ADRD); //attesa ADRDY=1
	//(nell' Interrupt and Status register)

	//SCELTA CANALE e TSAMPLE VA FATTA DOPO L'ABILITAZIONE ADC
	//Seleziono il TSAMPLE nel registro SMPR1
	ADC1->SMPR1 |= ADC_SMPR1_SMP3; //SMP3=111, 601.5 CK
	//CONT=0, conversione singola
	ADC1->CFGR &= ~ADC_CFGR_CONT;
	//quanti canali dobbiamo convertire (BIT L)
	ADC1->SQR1 &= ~ADC_SQR1_L; //L=0: 1 sola conversione
	//scelta del canale
	ADC1->SQR1 = ADC_SQR1_SQ1_0;  //SQ1=0001: canale 1 (PA0) 
	
	//AVVIO CONVERSIONE
	ADC1->CR |= ADC_CR_ADSTART; //ADSTART=1, avvio conversione

	//interruzioni
	ADC1->IER != ADC_ISR_EOC; //abilito EOC
	NVIC->ISER[0] |= (1<<18); //NVIC serve ADC1 e ADC2 global interrupt

	while(1){
		if(eoc==1){
			eoc=0;
			ris = ADC1->DR*(3.0/4096.0); //risultato * VDD/2^n
		
			printf("risultato: %f\n", ris);
		}
	
	}
void ADC1_2_IRQHandler(){
	if((ADC1->ISR & ADC_ISR_EOC) == ADC_ISR_EOC){ //se EOC = 1
		ADC1->ISR |= ADC_ISR_EOC; //lo azzero
		eoc = 1;
	}
}
