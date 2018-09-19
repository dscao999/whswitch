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
		ROM_GPIOIntTypeSet(gpio->base, pinintr, GPIO_FALLING_EDGE);
		HWREG(gpio->base+GPIO_O_ICR) = 0x0ff;
		HWREG(gpio->base+GPIO_O_IM) = pinintr;
		ROM_IntPrioritySet(intr, 0x60);
		ROM_IntEnable(intr);
	}
}

static void gpio_isr(struct gpio_port *gpio)
{
	uint32_t mis, icr, isrnum;
	int idx;
	uint32_t mask;

	mis = HWREG(gpio->base+GPIO_O_MIS) & 0x0ff;
	icr = HWREG(gpio->base+GPIO_O_ICR);
	HWREG(gpio->base+GPIO_O_ICR) = (icr & 0xffffff00) | mis;
	mask = 0x0f;
	idx = 0;
	while (mis != 0 && (mis & 1) == 0) {
		idx++;
		mask <<= 4;
		mis >>= 1;
	}
	isrnum = (gpio->isrnum & mask) >> (4*idx);
	isrnum += 1;
	isrnum &= 0x0f;
	isrnum <<= (4*idx);
	gpio->isrnum = (gpio->isrnum & ~mask)|isrnum;
}

static uint32_t debug_isrnum = 0;
void gpiob_isr(void)
{
	struct gpio_port *gpio = gpioms+GPIOB;

	gpio_isr(gpio);
	debug_isrnum++;
}

int  tm4c_gpio_isrnum(enum GPIOPORT port, uint8_t pin)
{
	struct gpio_port *gpio = gpioms+port;
	uint32_t mask;
	int idx;

	idx = 0;
	mask = 0x0f;
	while (pin != 0 && (pin & 1) == 0) {
		idx++;
		mask <<= 4;
		pin >>= 1;
	}
	
	return (gpio->isrnum & mask) >> (4*idx);
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

void tm4c_gpio_write(enum GPIOPORT port, uint8_t pins, int onoff)
{
	struct gpio_port *gpio = gpioms+port;
	int v;

	v = onoff? pins : 0;
	ROM_GPIOPinWrite(gpio->base, pins, v);
}

int tm4c_gpio_read(enum GPIOPORT port, uint8_t pins)
{
	struct gpio_port *gpio = gpioms+port;
	return ROM_GPIOPinRead(gpio->base, pins);
}
