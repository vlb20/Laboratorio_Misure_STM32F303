#define RCC_AHBENR 0x40021014 
#define GPIOE_MODER 0x48001000
#define GPIOE_ODR 0x48001014
unsigned int* p;

int main(){
	p=(unsigned int*) RCC_AHBENR;
	*p=(1<<21)
	p=(unsigned int*) GPIOE_MODER;
	*p =0x00010000;  
	*p |= 1<<18;     
	p=(unsigned int*) GPIOE_ODR;
	*p = 1<<8; 
	//oppure *p &= ~(1<<8) per spegnere PE8
	*p |= 1<<9;
	*p &= ~0xFFFFFDFF //per spegnere PE9
	
	while(1);

}
