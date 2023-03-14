#include <stdio.h>
#include <stm32f30x.h>
#include <math.h>

#define N 200

double tau;             //costante di tempo

//variabili del DAC
int codice_in =2048;    //setto il codice in ingresso fra [0;4095]
double tensione_out;    //stima della tensione generata da DAC

//variabili dell'ADC
double LUT[N];                   //stima della tensione letta dall'ADC
short int codice_out[N];         //risultato dell'ADC

void abilitazione_periferiche();
void disabilitazione_periferiche();

void setup_DAC();
void setup_DMA1();

void setup_slow_conversion();
void setup_ADC_slow();
void setup_TIM_slow();

//prototipi di funzioni per l'analisi con campionamento lento
void analisi_regime();
double valore_regime();
void calcolo_tempo_di_assestamento(int epsilon);

void setup_fast_conversion();
void setup_ADC_fast(); 
void setup_TIM_fast();

//prototipi di funzioni per l'analisi con campionamento veloce
void analisi_transitorio();
void tempo_salita(double vr);
void sovraelongazione(double vr);
double tempo_perc(double perc);
void perc_transitorio(double vr);

void main(){
  //Abilitazione DAC1, ADC12, GPIOA, SYSCFG, TIM2
  abilitazione_periferiche();
  
  //PA4, PA2,PA0
  GPIOA->MODER |= GPIO_MODER_MODER4;    //configuro PA4 come uscita analogica(MODER4=11)
  GPIOA->MODER |= GPIO_MODER_MODER2;    //configuro PA2 come uscita analogica(MODER2=11)
  GPIOA->MODER &= ~GPIO_MODER_MODER0;   //configuro PA0 come uscita digitale(input)(MODER=00)
  
  
  //Sottofunzioni per il setup di DAC, ADC, DMA e TIMER
  setup_DAC();
  setup_DMA1();

  /*
  * Selezionare una delle due funzioni in base all'analisi scelta
  *  setup_fast_conversion();
  *  setup_slow_conversion();
  */
  
  //Configurazione delle interrupt
  SYSCFG->EXTICR[1] &= ~7;              //EXTI=000: porta A
  EXTI->IMR |= EXTI_IMR_MR0;            //Interrupt Mask on line 0
  EXTI->RTSR |= EXTI_RTSR_TR0;          //Rising trigger event configuration bit of line 0
  NVIC->ISER[0] |= (1<<6);              //NVIC serve EXTI0
  
  //Risultato (DAC)
  tensione_out=codice_in * (3.0/4095.0);
  
  printf("DAC\n");
  printf("ingresso: %d\n",codice_in);
  printf("uscita: %f V\n\n",tensione_out);
  printf("-----------------\n\n");
  
  //conversione
  for(int i=0;i<10000000;i++);
  ADC1->CR |= ADC_CR_ADSTART;        //ADSTART=1, avvio conversione
  
  while((DMA1->ISR & DMA_ISR_TCIF1) != DMA_ISR_TCIF1);  //Attesa Transfer Complete Interrupt Flag
  DMA1->IFCR |= DMA_IFCR_CTCIF1;    //Clear Transfer Complete Interrupt Flag
  
  //Copia e traduzione in tensione delle parole codice in un vettore apposito
  for(int j=0;j<N;j++){
    LUT[j]=(double) (codice_out[j]*(3.0/4096.0));                      //risultato * VDD/(2^n)
  }
  
  double R= (2*pow(10,3));        //Valore Resistenza utilizzata
  double C= (1000*pow(10, -6));   //Valore Condensatore utilizzato
  tau = R*C;                      //Costante di tempo relativa alla carica di un condensatore
  
  /*
  *   In base al campionamento scelto scegliamo una delle due analisi
  *   analisi_regime();
  *   analisi_transitorio();
  */
  
  
  //Disabilitazione DAC1, ADC12, GPIOA, SYSCFG, TIM2, DMA
  disabilitazione_periferiche();
  

  while(1);
}
void abilitazione_periferiche(){
  RCC->AHBENR  |= RCC_AHBENR_DMA1EN;     //abilitazione DMA1
  RCC->AHBENR  |= RCC_AHBENR_GPIOAEN;    //abilitazione GPIOA
  RCC->AHBENR  |= RCC_AHBENR_ADC12EN;    //abilitazione ADC12
  RCC->APB1ENR |= RCC_APB1ENR_DACEN;     //abilitazione DAC
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;    //abilitazione TIM2
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;  //abilitazione SYSCFG
}

void disabilitazione_periferiche(){
  ADC1->CR |=ADC_CR_ADDIS;              //ADDIS=1
  DAC->CR &= ~DAC_CR_EN1;               //EN1=0
  TIM2->CR1 &= ~TIM_CR1_CEN;            //CEN=0
  DMA1_Channel1->CCR &= ~DMA_CCR_EN;    //Attivo il canale 1 del DMA per l'ADC
}

void setup_ADC_slow(){ //setup ADC con Tconv= 604 ADC clock cycles
  //Abilitazione regolatore di tensione
  ADC1->CR &= ~ADC_CR_ADVREGEN_1;       //ADVREGEN = 10->00
  ADC1->CR |= ADC_CR_ADVREGEN_0;        //ADVREGEN = 00->01
  for(int i=0;i<1000;i++);              //attesa di 10 microsec
  
  //Configurazione clock
  ADC1_2->CCR |= ADC12_CCR_CKMODE_0;    //CKMODE=01 attivo clock del bus AHB
  
  //Calibrazione ADC
  ADC1->CR |= ADC_CR_ADCAL;                             //ADCAL=1, avvio calibrazione
  while((ADC1->CR & ADC_CR_ADCAL) == ADC_CR_ADCAL);     //attesa calibrazione ADCAL=0
  
  //Abilitazione ADC
  ADC1->CR |= ADC_CR_ADEN;                              
  while((ADC1->CR & ADC_ISR_ADRD) != ADC_ISR_ADRD);     //attendiamo l'ADC Ready
  
  //Configurazione ADC
  ADC1->CFGR &= ~ADC_CFGR_CONT;         //CONT=0,conversione singola
  ADC1->CFGR |= ADC_CFGR_DMAEN;         //modalità DMA
  ADC1->CFGR |= ADC_CFGR_EXTEN_0;       //Trigger abilitati sul fronte di salita
  ADC1->CFGR |=(11<<6);                 //EXTSEL=1011: trigger del TIM2
  ADC1->SQR1 = (3<<6);                  //SQ1=00011: canale 3 (PA2)
  ADC1->SQR1 &= ~ADC_SQR1_L;            //L=0: 1 conversione
  ADC1->SMPR1 |= ADC_SMPR1_SMP3;        //SMP3=111, 601.5 CK
}

void setup_ADC_fast(){ //setup ADC con Tconv= 14 ADC clock cycles
  //Abilitazione regolatore di tensione
  ADC1->CR &= ~ADC_CR_ADVREGEN_1;       //ADVREGEN = 10->00
  ADC1->CR |= ADC_CR_ADVREGEN_0;        //ADVREGEN = 00->01
  for(int i=0;i<1000;i++);              //attesa di 10 microsec
  
  //Configurazione clock
  ADC1_2->CCR |= ADC12_CCR_CKMODE_0;    //CKMODE=01 attivo clock del bus AHB
  
  //Calibrazione ADC
  ADC1->CR |= ADC_CR_ADCAL;                             //ADCAL=1, avvio calibrazione
  while((ADC1->CR & ADC_CR_ADCAL) == ADC_CR_ADCAL);     //attesa calibrazione ADCAL=0
  
  //Abilitazione ADC
  ADC1->CR |= ADC_CR_ADEN;
  while((ADC1->CR & ADC_ISR_ADRD) != ADC_ISR_ADRD);     //attendiamo l'ADC Ready
  
  //Configurazione ADC
  ADC1->CFGR &= ~ADC_CFGR_CONT;         //CONT=0,conversione singola
  ADC1->CFGR |= ADC_CFGR_DMAEN;         //modalità DMA
  ADC1->CFGR |= ADC_CFGR_EXTEN_0;       //Trigger abilitati sul fronte di salita
  ADC1->CFGR |=(11<<6);                 //EXTSEL=1011: trigger del TIM2
  ADC1->SQR1 = (3<<6);                  //SQ1=00011: canale 3 (PA2)
  ADC1->SQR1 &= ~ADC_SQR1_L;            //L=0: 1 conversione
  ADC1->SMPR1 &= ~ADC_SMPR1_SMP3;       //SMP3=000, 1.5 CK
}

void setup_DAC(){
  DAC->CR |= DAC_CR_TEN1;        //abilitazione trigger
  DAC->CR |= DAC_CR_TSEL1_2;     //TSEL=100:trigger TIM2
  DAC->CR |= DAC_CR_EN1;         //abilitazione canale 1
  //scrivo il valore nel DHR
  DAC->DHR12R1 |= codice_in;
}

void setup_TIM_fast(){
  TIM2->ARR=9000000;            //aspetto 0.125sec
  TIM2->CR2=TIM_CR2_MMS_1;      //MMS=010 per far scattare il trigger
}

void setup_TIM_slow(){
  TIM2->ARR=36000000;           //aspetto 0.5sec
  TIM2->CR2=TIM_CR2_MMS_1;      //MMS=010 per far scattare il trigger
}

void setup_DMA1(){
  DMA1_Channel1->CPAR = (uint32_t)&ADC1->DR; //indirizzo periferica
  DMA1_Channel1->CMAR = (uint32_t)codice_out;//indirizzo memoria
  DMA1_Channel1->CNDTR = N;                  //numero trasferimenti
  DMA1_Channel1->CCR |= DMA_CCR_MSIZE_0;     //MSIZE = 16 bit
  DMA1_Channel1->CCR |= DMA_CCR_PSIZE_0;     //PSIZE = 16 bit
  DMA1_Channel1->CCR |= DMA_CCR_MINC;        //incremento indirizzo memoria
  DMA1_Channel1->CCR &= ~DMA_CCR_DIR;        //DIR=0; periferica->memoria
  DMA1_Channel1->CCR |= DMA_CCR_EN;          //abilitazione canale 1
}
void setup_fast_conversion(){
  setup_TIM_fast();
  setup_ADC_fast();
}

void setup_slow_conversion(){
  setup_TIM_slow();
  setup_ADC_slow();
}

double valore_regime(){
  //Considerando che il transitorio si estingue dopo 4 costanti di tempo, 
  //determino il numero del campione da cui partire per calcolare il valore di regime
  int campione =(int) ((8*tau)-1);
  
  //faccio una media dei valori su cui si assesta la nostra risposta
  double reg = 0;       //valore di regime
  int n_campioni = 0;   
  double somma = 0;
  for(int i=campione; i<N; i++){
      n_campioni++;
      somma += LUT[i];
    }
     
  reg = (somma/n_campioni); //media aritmetica
  
  return reg;
}

void calcolo_tempo_di_assestamento(int epsilon){  
  //calcolo la fascia di assestamento +- epsilon%
  double min_fascia = valore_regime() - ((valore_regime()*epsilon)/100);  //estremo minore
  double max_fascia = valore_regime() + ((valore_regime()*epsilon)/100);  //estremo superiore 
  printf("\nLa fascia di assestamento e': [%lf,%lf]", min_fascia, max_fascia);
  int n_campioni = 0;   
  
  //Verifico quando la risposta entra in questa fascia e non vi esce più
  for(int i=0; i<N; i++){                          //scorro il vettore delle tensioni
    if(LUT[i]>=min_fascia && LUT[i]<=max_fascia){  //se un valore è compreso nella fascia
      n_campioni++;                            //aumento il conteggio
    }
    
    if(LUT[i]<min_fascia && LUT[i]>max_fascia){    //se un valore della risposta non è compreso nella fascia
      n_campioni=0;                                //azzero il conteggio
    }
  }
  
  //calcolo il tempo di assestamento
  double N_tot = (N - n_campioni)/2.0;      //numero di campioni che precedono l'assestamento
  //per 0.5 s a causa della temporizzazione del TIM2
  printf("\nTempo di assestamento: %g", N_tot); 
    
}

void sovraelongazione(double vr){
  double sov;
  double percentuale;
  double max=0;
  //ricerca del massimo
  for(int i=0;i<N;i++){
    if(LUT[i]>max)
      max=LUT[i];
  }
  sov=max-vr;
  percentuale=sov*100/vr;
  printf("\nLa sovraelongazione corrisponde al valore di: %g", sov);
  printf("\nIl valore ottenuto corrisponde al %g %% del valore di regime",percentuale);
}

void tempo_salita(double vr){
  //calcolo del tempo in cui mi aspetto di raggiungere il 10%
  double t10=tempo_perc(10.0);
  printf("\ntempo ideale 10%%: %lf",t10);
  //calcolo del tempo in cui mi aspetto di raggiungere il 90%
  double t90=tempo_perc(90.0);
  printf("\ntempo ideale 90%%: %lf",t90);

  double tsi=t90-t10;
  printf("\nMi aspetto un tempo di salita pari a %g sec\n",tsi);
  printf("Confrontando coi risultati ottenuti ottengo:\n");
  //Quando raggiungo il 10%
  double val=vr*0.1;
  int i=0;
  while(LUT[i]<val){
    i++;
  }
  double t10eff=(double) ((i+1)/8.0);
  printf("\nIl valore di tensione di %g (10%%) viene raggiunto dopo %g secondi\n",val,t10eff);

  //Quando ragggiungo il 90%
  double val2=vr*0.9;
  int j=0;
  while(LUT[j]<val2){
    j++;
  }
  double t90eff=(double) ((j+1)/8.0);
  printf("Il valore di tensione di %g (90%%) viene raggiunto dopo %g secondi\n",val2,t90eff);
  double tse=t90eff-t10eff;
  printf("Il tempo di salita effettivo corrisponde a %g\n",tse);

  //Confronto risultato atteso e risultato effettivo
  double errore=fabs(tsi-tse);
  printf("La differenza fra il risultato aspettato e quello ottenuto è di %g\n",errore);
}

double tempo_perc(double perc){
  double t=-1;
  if(perc>0 && perc<100){
    double x=1-(perc/100.0);
    t=-log(x)*tau;
  }else{
    perror("Il valore deve essere compreso fra 0 e 100!");
  }
  return t;
}
                    
void analisi_regime(){ //stampe e chiamate alle funzioni per l'analisi con campionamento lento
  printf("La costante di tempo (R*C) e' Tau: %g secondi", tau);
  double vr=valore_regime();
  printf("\nIl valore di regime trovato è: %g", vr);
  calcolo_tempo_di_assestamento(1);
}

void analisi_transitorio(){ ////stampe e chiamate alle funzioni per l'analisi con campionamento veloce
  //il valore di regime va inserito manualmente
  sovraelongazione(1.495);
  tempo_salita(1.495);
  perc_transitorio(1.495);
}

void perc_transitorio(double vr){
	int index=0;
	double val=0;
	int i=0;
	double teff[9];

	for(int j=1;j<10;j++){
		//Quando raggiungo il j*10%
		val=vr*0.1*j;
		i=0;
		while(LUT[i]<val){
			i++;
		}
		teff[index]=i*0.125;
		printf("\nIl valore di tensione di %g (%d %%) viene raggiunto dopo %g secondi\n",val,j*10,teff[index]);
		index++;
	}
}

void EXTI0_IRQHandler(){          
	EXTI->PR |= EXTI_PR_PR0;        //cancellazione Pending Request
        TIM2->CNT=0;                    //azzero il conteggio
        TIM2->CR1=TIM_CR1_CEN;          //avvio il conteggio
 }
