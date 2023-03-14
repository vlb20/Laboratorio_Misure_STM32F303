#define RCC_AHBENR 0x40021014 //Abilito come prima cosa il clock della periferica
#define GPIOE_MODER 0x48001000
#define GPIOE_ODR 0x48001014
unsigned int* p;

int main(){
	p=(unsigned int*) RCC_AHBENR;
	*p=(1<<21)
	p=(unsigned int*) GPIOE_MODER;
	*p =0x00010000;  //moder 8 (1<<16)
	*p |= 1<<18;     //moder 9 (OR Uguale per non alterare i precedenti
	//o direttamente al posto di questi due *p=0x00050000
	p=(unsigned int*) GPIOE_ODR;
	*p = 1<<8; //ODR 8 -> led blu
	*p |= 1<<9;//ODR 9 -> led rosso
	//oppure direttamente *p=0x00000300

	while(1);


}
