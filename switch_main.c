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
static const char hello[] = "Initialization Completed!\n\r";

void __attribute__((noreturn)) main(void)
{
	uint16_t plen = 0, clen, len;
	char mesg[80], buf[80];

	tm4c_gpio_setup(GPIOA, 0, 0, 0);
	tm4c_gpio_setup(GPIOB, 0, 0, 0);
	tm4c_gpio_setup(GPIOF, 0, GPIO_PIN_3|GPIO_PIN_2|GPIO_PIN_1, 0); 
	
	tm4c_setup();
	tm4c_dma_enable();
	tm4c_delay(1000);
	tm4c_ledlit(GREEN, 1);
	tm4c_delay(3000);
	tm4c_ledlit(GREEN, 0);

	uart_open(0);
	uart_write(0, hello, strlen(hello));
	uart_open(1);
	uart_write(1, hello, strlen(hello));
	uart_read_start(1, mesg, sizeof(mesg));

	while(1) {
		clen = uart_read_bytes(1);
		if ((clen - plen) == 0)
			continue;
		plen = clen;
		uart_write_dmawait(0);
		len = num2str_dec(clen, buf, sizeof(80));
		buf[len] = 0x0d;
		buf[len+1] = 0x0a;
		uart_write(0, buf, len+2);
		
		if (len > 5 && memcmp(mesg, RESET, 5) == 0)
			tm4c_reset();
/*		if (memchr(mesg, len, '\r') < len) {
			uart_write(0, echo, strlen(echo));
			uart_write(0, mesg, len);
			uart_read_stop(1);
			uart_read_start(1, mesg, sizeof(mesg));
		}*/
	}
}
