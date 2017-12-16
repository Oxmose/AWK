
#include "../drivers/pit.h"
void kernel_kickstart(void)
{
	init_pit();

	__asm__ __volatile__("sti");
}