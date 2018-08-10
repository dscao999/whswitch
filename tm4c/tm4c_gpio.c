#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_gpio.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "tm4c_miscs.h"
#include "tm4c_gpio.h"

uint32_t gpioc_isr_nums = 0;

static struct gpio_port gpioms[] = {
	{
		.base = GPIO_PORTA_BASE,
	},
	{
		.base = GPIO_PORTB_BASE,
	},
	{
		.base = GPIO_PORTC_BASE,
	},
	{
		.base = GPIO_PORTD_BASE,
	},
	{
		.base = GPIO_PORTE_BASE,
	},
	{
		.base = GPIO_PORTF_BASE,
	}
};

void tm4c_gpio_setup(enum GPIOPORT port, uint8_t inps, uint8_t outps, uint8_t intrps)
{
	struct gpio_port *gpio = gpioms+port;
	uint32_t intr;
	uint32_t sysperip;

	switch(port) {
	case GPIOA:
		intr = INT_GPIOA;
		sysperip = SYSCTL_PERIPH_GPIOA;
		break;
	case GPIOB:
		intr = INT_GPIOB;
		sysperip = SYSCTL_PERIPH_GPIOB;
		break;
	case GPIOC:
		intr = INT_GPIOC;
		sysperip = SYSCTL_PERIPH_GPIOC;
		break;
	case GPIOD:
		intr = INT_GPIOD;
		sysperip = SYSCTL_PERIPH_GPIOD;
		break;
	case GPIOE:
		intr = INT_GPIOE;
		sysperip = SYSCTL_PERIPH_GPIOE;
		break;
	case GPIOF:
		intr = INT_GPIOF;
		sysperip = SYSCTL_PERIPH_GPIOF;
		break;
	default:
		while (1)
			;
	}
	ROM_SysCtlPeripheralEnable(sysperip);
	while(!ROM_SysCtlPeripheralReady(sysperip))
                        ;
	if (inps) {
		ROM_GPIODirModeSet(gpio->base, inps, GPIO_DIR_MODE_IN);
		HWREG(gpio->base+GPIO_O_DEN) |= inps;
	}
	if (outps) {
		ROM_GPIOPadConfigSet(gpio->base, outps, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_OD);
		ROM_GPIODirModeSet(gpio->base, outps, GPIO_DIR_MODE_OUT);
	}

	if (intrps) {
		HWREG(gpio->base+GPIO_O_IM) = 0;
		ROM_GPIOIntTypeSet(gpio->base, intrps, GPIO_BOTH_EDGES);
		HWREG(gpio->base+GPIO_O_ICR) = 0x0ff;
		HWREG(gpio->base+GPIO_O_IM) = intrps;
		ROM_IntPrioritySet(intr, 0x60);
		ROM_IntEnable(intr);
	}
}

static void gpio_isr(struct gpio_port *gpio)
{
	int mis, icr;

	mis = HWREG(gpio->base+GPIO_O_MIS) & 0x0ff;
	icr = HWREG(gpio->base+GPIO_O_ICR);
	HWREG(gpio->base+GPIO_O_ICR) = (icr & 0xffffff00) | mis;
	if (mis & 0x01)
		gpio->pinisr.isr4.p0++;
	if ((mis >> 1) & 0x01)
		gpio->pinisr.isr4.p1++;
	if ((mis >> 2) & 0x01)
		gpio->pinisr.isr4.p2++;
	if ((mis >> 3) & 0x01)
		gpio->pinisr.isr4.p3++;
	if ((mis >> 4) & 0x01)
		gpio->pinisr.isr4.p4++;
	if ((mis >> 5) & 0x01)
		gpio->pinisr.isr4.p5++;
	if ((mis >> 6) & 0x01)
		gpio->pinisr.isr4.p6++;
	if ((mis >> 7) & 0x01)
		gpio->pinisr.isr4.p7++;
	gpio->isr_nums++;
}

void gpioc_isr(void)
{
	struct gpio_port *gpio = gpioms+GPIOC;

	gpio_isr(gpio);
}

void tm4c_gpio_write(enum GPIOPORT port, uint8_t pins, int on_off)
{
	struct gpio_port *gpio = gpioms+port;
	uint8_t v;

	v = on_off? 0xff : 0x0;
	ROM_GPIOPinWrite(gpio->base, pins, v);
}

int  tm4c_gpio_isrtimes(enum GPIOPORT port)
{
	struct gpio_port *gpio = gpioms+port;
	
	return gpio->isr_nums;
}

int  tm4c_gpio_isrnum(enum GPIOPORT port, uint8_t pin)
{
	struct gpio_port *gpio = gpioms + port;
	int idx;

	idx = 0;
	while ((pin & 0x01) == 0 && idx < 8) {
		idx++;
		pin >>= 1;
	}
	if (idx < 8)
		return ((gpio->pinisr.isr32) >> (idx*4)) & 0x0f;
	else
		return 0;
}
