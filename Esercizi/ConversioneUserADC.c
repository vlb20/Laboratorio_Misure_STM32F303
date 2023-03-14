```c
#include <stm32f30x.h>
#include <stdio.h>

float ris;

void main(){
	//abilito la coppia di ADC12
	RCC->AHBENR |= RCC_AHBENR_ADC12EN;
	//scelgo il clock di lavoro (uso quello di sistema)
	//bit CKMODE presenti nel registro comune CCR
	ADC1_2->CCR |= ADC12_CCR_CKMODE_0; //CKMODE=01, clock del bus AHB
	//clock di sistema senza alcuna divisione CKMODE= 01
	//00 -> clock esterni
	//altre configurazioni per abbattere le frequenze di funzionamento
	//ad esempio per i sensori di temperatura

	//Abilitazione GPIOA (per il pulsante USER)
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	//PA0 in modalità analogica (MODER0=11)
	GPIOA->MODER |= GPIO_MODER_MODER0;

	//tensione di riferimento pulita per non avere incertezze
	//generatore di tensione di riferimento interno in questo caso
	//sequenza particolare di abilitazione
	ADC1->CR &= ~ADC_CR_ADVREGEN_1; //ADVREGEN = 10->00 (reset)
	ADC1->CR |= ADC_CR_ADVREGEN_0;  //ADVREGEN = 00->01 (abilitazione)
	//attendiamo che la tensione si stabilizzi
	for(int i=0; i<1000; i++);      //attesa di 10 µs (indicato nel manuale)

	//verifica che l'ADC sia disabilitato (ADEN==0)
	ADC1->CR &= ~ADC_CR_ADEN; 

	//modalità di acquisizione single-ended 
	ADC1->DIFSEL &= ADC_DIFSEL_DIFSEL_1; //DIFSEL=0->single ended

	//Calibrazione 
	ADC1->CR &= ~ADC_CR_ADCALDIF; //Calibrazione single-ended (ADCALDIF = 0)
	ADC1->CR |= ADC_CR_ADCAL; //ADCAL=1, avvio la calibrazione
	while((ADC1->CR & ADC_CR_ADCAL) == ADC_CR_ADCAL); //attendo ADCAL=0 (fine calib)
	//IL reset avviene in automatico -> dobbiamo solo aspettarlo
	//Il while è lento
	//per una startup rapida -> aggancio qualcosa  a questi eventi mentre 
	//eseguo i task che posso eseguire

	//ABILITAZIONE ADC
	ADC1->CR |= ADC_CR_ADEN;  //ADEN=1, abilitazione ADC
	while((ADC1->ISR & ADC_ISR_ADRD) != ADC_ISR_ADRD); //attesa ADRDY=1
	//(nell' Interrupt and Status register)

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
	
	//attendo EOC=1 -> FINE della conversione
	while((ADC1->ISR & ADC_ISR_EOC) != ADC_ISR_EOC);
	
	ris = ADC1->DR*(3.0/4096.0); //risultato * VDD/2^n

	printf("risultato: %f\n", ris);
}
```
