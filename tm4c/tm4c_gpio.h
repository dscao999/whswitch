#ifndef TM4C_GPIO_DSCAO__
#define TM4C_GPIO_DSCAO__
#include <stdint.h>
#include <inc/hw_memmap.h>
#include <inc/hw_gpio.h>
#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>
#include <driverlib/rom.h>

enum GPIOPORT { GPIOA = 0, GPIOB = 1, GPIOC = 2, GPIOD = 3, GPIOE = 4, GPIOF = 5};
struct gpio_port {
	uint32_t base;
	uint32_t isr_nums;
	union {
		struct {
			uint32_t p0:4;
			uint32_t p1:4;
			uint32_t p2:4;
			uint32_t p3:4;
			uint32_t p4:4;
			uint32_t p5:4;
			uint32_t p6:4;
			uint32_t p7:4;
		} isr4;
		uint32_t isr32;
	} pinisr;
};

enum ledcolor {RED, BLUE, GREEN};

void gpioc_isr(void);

void tm4c_gpio_setup(enum GPIOPORT port, uint8_t inps, uint8_t outps, uint8_t intrps);
void tm4c_gpio_write(enum GPIOPORT port, uint8_t pins, int on_off);
int  tm4c_gpio_isrtimes(enum GPIOPORT port);
int  tm4c_gpio_isrnum(enum GPIOPORT port, uint8_t pin);

void tm4c_ledlit(enum ledcolor led, int onoff);
#endif /* TM4C_GPIO_DSCAO__ */
