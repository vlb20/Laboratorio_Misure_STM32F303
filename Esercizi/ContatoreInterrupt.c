#include "stm32f30x.h"

unsigned char c=0;

void main(){
  
  RCC->AHBENR |= RCC_AHBENR_GPIOEEN; //porta E
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
  GPIOE->MODER |= 0x55550000; //porta 8 a 15
  
  
  //Abilitiamo l'interrupt sul Timer2
  TIM2->DIER |= TIM_DIER_UIE; //Upadate Interrupt Enable
  NVIC->ISER[0] |= 1<<28;
  
  TIM2->CNT = 0;
  TIM2->ARR=72000000;//il moltiplicatore di frequenza moltiplica 8000000 per 9, quindi ho 72
  TIM2->CR1 |= 1; //alzo il CEN
  
  while(1);
}

void TIM2_IRQHandler(){
  
  if(TIM2->SR&1 ==1){ //se UIF è alzato
    c++;
    GPIOE->ODR = (c<<8); //nell'output data register
    TIM2->SR &= ~0x00000001; //abbassiamo UIF
    
  }
}
