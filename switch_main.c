//*****************************************************************************
// This is part of revision 2.1.4.178 of the EK-TM4C123GXL Firmware Package.
//
//*****************************************************************************

#include "miscutils.h"
#include "tm4c_miscs.h"
#include "tm4c_gpio.h"
#include "tm4c_uart.h"
#include "tm4c_dma.h"
#include "uart_op.h"

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
static const char hello[] = "Initialization Completed!\n";

void __attribute__((noreturn)) main(void)
{
	uint32_t isr_count;
	uint16_t len = 0;

	tm4c_gpio_setup(GPIOA, 0, 0, 0);
	tm4c_gpio_setup(GPIOB, 0, 0, 0);
/*	tm4c_gpio_setup(GPIOC, GPIO_PIN_4, GPIO_PIN_7, GPIO_PIN_4);
	tm4c_gpio_setup(GPIOD, 0, 0, 0); */
	tm4c_gpio_setup(GPIOF, 0, GPIO_PIN_3|GPIO_PIN_2|GPIO_PIN_1, 0);
	tm4c_setup();
	tm4c_dma_enable();
	tm4c_ledlit(RED, 1);
	tm4c_delay(1000);
	tm4c_ledlit(RED, 0);

	uart_open(0);
	uart_write(0, hello, strlen(hello));

	while(1) {
		if (len == 0 && uart_op(&dbg_uart)) {
			g_ctrl.dir ^= 1;
			if (memcmp(dbg_uart.buf, RESET, 5) == 0)
				tm4c_reset();
			else if (memcmp(dbg_uart.buf, "BTNxInfo", 8) == 0) {
				memcpy(dbg_uart.buf, "Button Press: ", 14);
				isr_count = tm4c_gpio_isrtimes(GPIOC);
				len = num2str_dec(isr_count, dbg_uart.buf+14, 8);
				dbg_uart.buf[len+14] = 0x0d;
				len += 15;
			} else if (memcmp(dbg_uart.buf, "PWMxInfo", 8) == 0) {
				memcpy(dbg_uart.buf, "PWM Intrs: ", 11);
				isr_count = tm4c_pwm_get_intrs(g_ctrl.pwm, 0);
				len = num2str_dec(isr_count, dbg_uart.buf+11, 8);
				dbg_uart.buf[len+11] = 0x0d;
				len += 12;
			} else {
				memcpy(dbg_uart.buf, "Motor Direction: ", 17);
				len = num2str_dec(g_ctrl.dir, dbg_uart.buf+17, 8);
				dbg_uart.buf[len+17] = 0x0d;
				len += 18;
			}
			dbg_uart.pos = 0;
		}
		if (len != 0 && !uart_in_dma(0)) {
			uart_write(dbg_uart.port, dbg_uart.buf, len, 0);
			len = 0;
		}
		global_task(&g_ctrl);
	}
}
