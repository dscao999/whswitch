#ifndef TM4C_GPIO_DSCAO__
#define TM4C_GPIO_DSCAO__
#include <stdint.h>
#include <inc/hw_memmap.h>
#include <inc/hw_gpio.h>
#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>
#include <driverlib/rom.h>

enum GPIOPORT {
	GPIOA = 0, GPIOB = 1, GPIOC = 2, GPIOD = 3,
	GPIOE = 4, GPIOF = 5, GPIOG = 6
};
struct gpio_port {
	uint32_t base;
	volatile uint32_t isrnum;
};

enum ledcolor {RED, BLUE, GREEN};

void gpiob_isr(void);

void tm4c_gpio_setup(enum GPIOPORT port, uint8_t inps, uint8_t outps, uint8_t intrps);
void tm4c_gpio_write(enum GPIOPORT port, uint8_t pins, int onoff);
int tm4c_gpio_isrnum(enum GPIOPORT port, uint8_t pin);
int tm4c_gpio_read(enum GPIOPORT port, uint8_t pins);

void tm4c_ledlit(enum ledcolor led, int onoff);
#endif /* TM4C_GPIO_DSCAO__ */
