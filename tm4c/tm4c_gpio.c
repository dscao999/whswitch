#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_gpio.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "tm4c_setup.h"
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
	},
	{
		.base = GPIO_PORTG_BASE,
	}
};

void tm4c_gpio_setup(enum GPIOPORT port, uint8_t inpin, uint8_t outpin, uint8_t intrpin)
{
	struct gpio_port *gpio = gpioms+port;
	uint32_t intr, pinintr;
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
	case GPIOG:
		intr = INT_GPIOG;
		sysperip = SYSCTL_PERIPH_GPIOG;
		break;
	default:
		while (1)
			;
	}
	ROM_SysCtlPeripheralEnable(sysperip);
	while(!ROM_SysCtlPeripheralReady(sysperip))
                        ;
	if (inpin)
		ROM_GPIOPinTypeGPIOInput(gpio->base, inpin);
	if (outpin)
		ROM_GPIOPinTypeGPIOOutput(gpio->base, outpin);
	if (intrpin & inpin) {
		pinintr = (intrpin & inpin);
		HWREG(gpio->base+GPIO_O_IM) = 0;
		ROM_GPIOIntTypeSet(gpio->base, pinintr, GPIO_BOTH_EDGES);
		HWREG(gpio->base+GPIO_O_ICR) = 0x0ff;
		HWREG(gpio->base+GPIO_O_IM) = pinintr;
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

void tm4c_ledlit(enum ledcolor led, int onoff)
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

	v = onoff? ledpin : 0;
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, ledpin, v);
}
