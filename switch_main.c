//*****************************************************************************
// This is part of revision 2.1.4.178 of the EK-TM4C123GXL Firmware Package.
//
//*****************************************************************************

#include "miscutils.h"
#include "tm4c_setup.h"
#include "tm4c_gpio.h"
#include "tm4c_uart.h"
#include "tm4c_dma.h"

//*****************************************************************************
//
//*****************************************************************************
//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
	while(1);
}
#endif

//*****************************************************************************
//
//
//*****************************************************************************
static const char *RESET = "ReseT";
static const char hello[] = "Initialization Completed!\r\n";

void __attribute__((noreturn)) main(void)
{
	uint16_t plen, clen, iport;
	char mesg[80], buf[80];
	int p_isrs, gpio_isrs, len, usedma;

	tm4c_gpio_setup(GPIOA, 0, 0, 0);
	tm4c_gpio_setup(GPIOB, GPIO_PIN_5, 0, GPIO_PIN_5);
	tm4c_gpio_setup(GPIOC, 0, 0, 0);
	tm4c_gpio_setup(GPIOF, 0, GPIO_PIN_3|GPIO_PIN_2|GPIO_PIN_1, 0); 
	
	tm4c_setup();
	tm4c_dma_enable();
	tm4c_ledlit(GREEN, 1);
	tm4c_delay(2000);
	tm4c_ledlit(GREEN, 0);

	uart_open(0);
	uart_open(1);
	uart_write(0, hello, strlen(hello));
	uart_write(1, hello, strlen(hello));
	tm4c_ledlit(RED, 1);
	tm4c_delay(500);
	tm4c_ledlit(RED, 0);

	usedma = 1;
	iport = 1;
	uart_read_start(iport, mesg, sizeof(mesg), usedma);

	plen = 0;
	p_isrs= 0;
	while(1) {
		clen = uart_read_curlen(iport);
		if ((clen - plen) > 0) {
			uart_write(iport, mesg+plen, clen-plen);
			plen = clen;
		}
		gpio_isrs = tm4c_gpio_isrnum(GPIOB, GPIO_PIN_5);
		if (gpio_isrs != p_isrs) {
			uart_wait_txdma(0);
			len = num2str_dec(gpio_isrs, buf, sizeof(buf));
			buf[len] = 0x0a;
			buf[len+1] = 0x0d;
			uart_write(0, buf, len+2);
			p_isrs = gpio_isrs;
		}
		if (memchr(mesg, clen, '\r') < clen) {
			uart_read_stop(iport);
			uart_write(iport, "\n", 1);
			if (clen > 5 && memcmp(mesg, RESET, 5) == 0) {
				uart_trans_wait(iport);
				tm4c_reset();
			}
			uart_wait_txdma(iport);
			uart_read_start(iport, mesg, sizeof(mesg), usedma);
			plen = 0;
		}
	}
}
