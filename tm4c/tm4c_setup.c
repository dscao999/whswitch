#include "tm4c_setup.h"

const uint32_t MEMADDR = 0x20000000;
volatile uint32_t sys_ticks;

void tm4c_setup(void)
{
	ROM_SysCtlClockSet(SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN|SYSCTL_USE_PLL|SYSCTL_SYSDIV_2_5);
	ROM_SysTickPeriodSet(HZ/CYCLES-1);
	sys_ticks = 0;
	HWREG(NVIC_ST_CURRENT) = 0;
	ROM_IntMasterEnable();
	ROM_SysTickIntEnable();
	ROM_SysTickEnable();
}
