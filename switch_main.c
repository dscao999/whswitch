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
	uint16_t plen = 0, clen;
	char mesg[80];

	tm4c_gpio_setup(GPIOA, 0, 0, 0);
	tm4c_gpio_setup(GPIOB, 0, 0, 0);
	tm4c_gpio_setup(GPIOF, 0, GPIO_PIN_3|GPIO_PIN_2|GPIO_PIN_1, 0); 
	
	tm4c_setup();
	tm4c_dma_enable();
	tm4c_delay(1000);
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
	uart_read_start(0, mesg, sizeof(mesg));
	uart_close(1);

	while(1) {
		clen = uart_read_bytes(0);
		if ((clen - plen) == 0)
			continue;
		uart_write(0, mesg+plen, clen-plen);
		plen = clen;
		
		if (memchr(mesg, clen, '\r') < clen) {
			uart_read_stop(0);
			uart_write(0, "\n", 1);
			uart_write_wait(0);
			if (clen > 5 && memcmp(mesg, RESET, 5) == 0)
				tm4c_reset();
			uart_read_start(0, mesg, sizeof(mesg));
			plen = 0;
		}
	}
}
