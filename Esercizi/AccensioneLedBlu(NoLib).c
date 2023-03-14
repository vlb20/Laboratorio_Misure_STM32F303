#define RCC_AHBENR 0x40021014
#define GPIOE_MODER 0x48001000
#define GPIOE_ODR 0x48001014
unsigned int* p;
int main(){
	p=(unsigned int*) RCC_AHBENR;
	*p= (1<<21);
	p=(unsigned int*) GPIOE_MODER;
	*p=0x00010000;
	p=(unsigned int*) GPIOE_ODR;
	*p=0x00000100;

	while(1); //return 0
}
