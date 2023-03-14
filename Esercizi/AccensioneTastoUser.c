#define RCC_AHBENR 0x40021014 //attivazione periferiche (clock)
#define GPIOA_MODER 0x48000000 //pulsante USER - PA0
#define GPIOA_IDR 0x48000010  //lettura input
#define GPIOE_MODER 0x48001000 //abilitare LED - PE8...PE15
#define GPIOE_ODR 0x48001014 //accendere led
unsigned int* p

p=(unsigned int*) RCC_AHBENR;
*p= (1<<21) | (1<<17); //abilito il clock di GPIOA e GPIOE
p=(unsigned int*) GPIOE_MODER;
*p=0x55550000; //rendo output tutte le linee a cui sono collegati i led (PE8-PE15)
p=(unsigned int*) GPIOA_MODER;
*p &= 0xFFFFFFFC; //Ultimi due bit a 0 -> per mettere PA0 come input
//senza modificare gli altri

while(1){ //ciclo indefinito
	p=(unsigned int*) GPIOA_IDR; //lettura input
	if((*p) & 1) == 1 ){//controllo se l'And bit a bit con l'1 sia proprio 1->tasto USER ON
		p=(unsigned int*) GPIOE_ODR; 
		*p= 0x0000FF00; //accendo i led
	}//se il tasto USER non è premuto
	else{
		p=(unsigned int*) GPIOE_ODR;
		*p= 0x00000000; //spengo i led
	}
}
