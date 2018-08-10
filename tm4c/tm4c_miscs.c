#include "tm4c_miscs.h"
#include "tm4c_gpio.h"

const uint32_t HZ = 80000000;
const uint32_t CYCLES = 50;
const uint32_t MEMADDR = 0x20000000;
volatile uint32_t sys_ticks;
uint32_t cycles;

void tm4c_setup(void)
{
	ROM_SysCtlClockSet(SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN|SYSCTL_USE_PLL|SYSCTL_SYSDIV_2_5);
	ROM_SysTickPeriodSet(HZ/CYCLES-1);
	sys_ticks = 0;
	cycles = CYCLES/10;
	HWREG(NVIC_ST_CURRENT) = 0;
	ROM_IntMasterEnable();
	ROM_SysTickIntEnable();
	ROM_SysTickEnable();
}

void tm4c_ledlit(enum led_type led, int onoff)
{
	uint8_t ledpin, v;

	switch(led) {
	case RED:
		ledpin = GPIO_PIN_1;
		break;
	case BLUE:
		ledpin = GPIO_PIN_2;
		break;
	case GREEN:
		ledpin = GPIO_PIN_3;
		break;
	default:
		ledpin = GPIO_PIN_1;
		break;
	}

	v = onoff? 0xff : 0x0;
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, ledpin, v);
}
