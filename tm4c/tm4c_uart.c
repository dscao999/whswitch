#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_uart.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/udma.h"
#include "driverlib/uart.h"
#include "tm4c_miscs.h"
#include "tm4c_dma.h"
#include "tm4c_uart.h"

static volatile uint32_t uart0_isr_nums = 0;
static volatile uint32_t uart1_isr_nums = 0;
static const int ubuf_mask = UART_BUFSIZ - 1;

static volatile int rxdma = 0;

static int idx_step(int *idx)
{
	int prev = *idx;
	*idx = (prev + 1) & ubuf_mask;
	return prev;
}

static struct uart_port uartms[] = {
	{
		.base = UART0_BASE,
		.rxhead = 0,
		.rxtail = 0,
		.tx_dmach = UDMA_CHANNEL_UART0TX
	},
	{
		.base = UART1_BASE,
		.rxhead = 0,
		.rxtail = 0,
		.tx_dmach = UDMA_CHANNEL_UART1TX,
		.rx_dmach = UDMA_CHANNEL_UART1RX 
	}
};

void uart_open(int port)
{
	uint32_t imask, periph, intr, baud, sflag;
	struct uart_port *uart = uartms + port;

	baud = 115200;
	switch(port) {
	case 0:
		ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0|GPIO_PIN_1);
		ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
		ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
		ROM_uDMAChannelAssign(UDMA_CH9_UART0TX);
		periph = SYSCTL_PERIPH_UART0;
		intr = INT_UART0;
		break;
	case 1:
		baud = 19200;
		ROM_GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0|GPIO_PIN_1);
		ROM_GPIOPinConfigure(GPIO_PB0_U1RX);
		ROM_GPIOPinConfigure(GPIO_PB1_U1TX);
		ROM_uDMAChannelAssign(UDMA_CH23_UART1TX);
		ROM_uDMAChannelAssign(UDMA_CH22_UART1RX);
		periph = SYSCTL_PERIPH_UART1;
		intr = INT_UART1;
		break;
	case 2:
		periph = SYSCTL_PERIPH_UART2;
		intr = INT_UART2;
		break;
	case 3:
		periph = SYSCTL_PERIPH_UART3;
		intr = INT_UART3;
		break;
	case 4:
		periph = SYSCTL_PERIPH_UART4;
		intr = INT_UART4;
		break;
	case 5:
		periph = SYSCTL_PERIPH_UART5;
		intr = INT_UART5;
		break;
	case 6:
		periph = SYSCTL_PERIPH_UART6;
		intr = INT_UART6;
		break;
	case 7:
		periph = SYSCTL_PERIPH_UART7;
		intr = INT_UART7;
		break;
	default:
		while (1)
			;
	}
	ROM_SysCtlPeripheralEnable(periph);
	while (!ROM_SysCtlPeripheralReady(periph))
			;
	ROM_UARTClockSourceSet(uart->base, UART_CLOCK_SYSTEM);
	ROM_UARTConfigSetExpClk(uart->base, HZ, baud,
		UART_CONFIG_WLEN_8|UART_CONFIG_STOP_ONE|UART_CONFIG_PAR_NONE);
	ROM_UARTFIFOLevelSet(uart->base, UART_FIFO_TX2_8, UART_FIFO_RX6_8);
	ROM_UARTFIFOEnable(uart->base);
	imask = UART_INT_OE|UART_INT_FE|UART_INT_TX|UART_INT_RX|UART_INT_RT;

	HWREG(uart->base+UART_O_DMACTL) &= ~(UART_DMA_ERR_RXSTOP|UART_DMA_TX|UART_DMA_RX);
	ROM_uDMAChannelAttributeDisable(uart->tx_dmach, UDMA_ATTR_ALL);
	ROM_uDMAChannelControlSet(uart->tx_dmach|UDMA_PRI_SELECT,
		UDMA_SIZE_8|UDMA_SRC_INC_8|UDMA_DST_INC_NONE|UDMA_ARB_8);
	if (uart->rx_dmach) {
		ROM_uDMAChannelAttributeDisable(uart->rx_dmach, UDMA_ATTR_ALL);
		ROM_uDMAChannelControlSet(uart->rx_dmach|UDMA_PRI_SELECT,
			UDMA_SIZE_8|UDMA_SRC_INC_NONE|UDMA_DST_INC_8|UDMA_ARB_8);
	}
	HWREG(uart->base+UART_O_IM) = imask;

	ROM_UARTEnable(uart->base);
	sflag = HWREG(uart->base+UART_O_FR);
	while((sflag & UART_FR_RXFE) == 0 || (sflag & UART_FR_BUSY) != 0) {
		if ((sflag & UART_FR_RXFE) == 0)
			HWREG(uart->base+UART_O_DR);
		sflag = HWREG(uart->base+UART_O_FR);
	}
	HWREG(uart->base+UART_O_ICR) = HWREG(uart->base+UART_O_RIS);
	ROM_IntPrioritySet(intr, 0xe0);
	ROM_IntEnable(intr);
}

static int uart_direct_write(struct uart_port *uart, const char *str, int len)
{
	const unsigned char *ustr;
	int i;

	for (i = 0, ustr = (const unsigned char *)str; i < len; ustr++, i++) {
		while(HWREG(uart->base+UART_O_FR) & UART_FR_TXFF)
			;
		HWREG(uart->base+UART_O_DR) = *ustr;
	}
	return len;
}

int uart_write_sync(int port, const char *str, int len)
{
	struct uart_port *uart = uartms + port;

	return uart_direct_write(uart, str, len);
}

int uart_write(int port, const char *str, int len, int wait)
{
	int dmalen;
	struct uart_port *uart = uartms + port;

	if (!wait && uart->txdma)
		return 0;
	while (wait && uart->txdma)
		tm4c_waitint();
	if (str < (char *)MEMADDR)
		return uart_direct_write(uart, str, len);

	dmalen = len > MAX_DMALEN? MAX_DMALEN : len;
	ROM_uDMAChannelTransferSet(uart->tx_dmach|UDMA_PRI_SELECT,
		UDMA_MODE_BASIC, (void *)str, (void *)(uart->base+UART_O_DR), dmalen);
	uart->txdma = 1;
	HWREG(uart->base+UART_O_DMACTL) |= UART_DMA_TX;
	HWREG(UDMA_ENASET) = 1 << uart->tx_dmach;
	while (wait && uart->txdma)
		tm4c_waitint();
	return dmalen;
}

void uart_write_cmd_expect(int port, const char cmd, int explen)
{
	int dmalen;
	struct uart_port *uart = uartms + port;

	if (uart->txdma || uart->rxdma)
		return;
	dmalen = explen > MAX_DMALEN? MAX_DMALEN : explen;
	ROM_uDMAChannelTransferSet(uart->rx_dmach|UDMA_PRI_SELECT,
		UDMA_MODE_BASIC, (void *)(uart->base+UART_O_DR), (void *)uart->rxbuf, dmalen);
	uart->rxdma = 1;
	rxdma = 1;
	uart->dmalen = dmalen;
	HWREG(uart->base+UART_O_DMACTL) |= UART_DMA_RX;
	while(HWREG(uart->base+UART_O_FR) & UART_FR_TXFF)
		;
	uart->rxhead = 0;
	uart->rxtail = 0;
	HWREG(UDMA_ENASET) = 1 << uart->rx_dmach;
	HWREG(uart->base+UART_O_DR) = cmd;
	uart->at_tick = tm4c_tick_after(0);
}

int uart_read_expect(int port, char *buf, int len)
{
	uint8_t *uchar;
	int count, tail;
	struct uart_port *uart = uartms + port;

	if (uart->rxdma == 1)
		return 0;

	tail = uart->rxtail;
	count = 0;
	uchar = (uint8_t *)buf;
	while (tail != uart->rxhead && count < len) {
		*uchar++ = uart->rxbuf[idx_step(&tail)];
		count++;
	}
	uart->rxtail = tail;
	return count;
}

int uart_read(int port, char *buf, int len, int wait)
{
	uint8_t *uchar, cret;
	int count, tail;
	struct uart_port *uart = uartms + port;

	while (wait && uart->rxtail == uart->rxhead)
		tm4c_waitint();

	tail = uart->rxtail;
	count = 0;
	uchar = (uint8_t *)buf;
	cret = 0;
	while (tail != uart->rxhead && count < len && cret != 0x0d && cret != 0x0a) {
		cret = uart->rxbuf[idx_step(&tail)];
		*uchar++ = cret;
		count++;
	}
	uart->rxtail = tail;
	return count;
}

void uart_close(int port)
{
	struct uart_port *uart = uartms + port;
	uint32_t intr, periph;

        while (uart->txdma)
                ;
	switch(port) {
	case 0:
		intr = INT_UART0;
		periph = SYSCTL_PERIPH_UART0;
		break;
	case 1:
		intr = INT_UART1;
		periph = SYSCTL_PERIPH_UART1;
		break;
	case 2:
		intr = INT_UART2;
		periph = SYSCTL_PERIPH_UART2;
		break;
	case 3:
		intr = INT_UART3;
		periph = SYSCTL_PERIPH_UART3;
		break;
	case 4:
		intr = INT_UART4;
		periph = SYSCTL_PERIPH_UART4;
		break;
	case 5:
		intr = INT_UART5;
		periph = SYSCTL_PERIPH_UART5;
		break;
	case 6:
		intr = INT_UART6;
		periph = SYSCTL_PERIPH_UART6;
		break;
	case 7:
		intr = INT_UART7;
		periph = SYSCTL_PERIPH_UART7;
		break;
	default:
		while(1)
			;
	}
	ROM_IntDisable(intr);
        ROM_UARTDisable(uart->base);
	ROM_SysCtlPeripheralDisable(periph);
}

static void uart_recv(struct uart_port *uart)
{
	int head;

	while ((HWREG(uart->base+UART_O_FR) & UART_FR_RXFE) == 0) {
		head = uart->rxhead;
		uart->rxbuf[idx_step(&head)] = HWREG(uart->base+UART_O_DR);
		while ((HWREG(uart->base+UART_O_FR) & UART_FR_RXFE) == 0)
			uart->rxbuf[idx_step(&head)] = HWREG(uart->base+UART_O_DR);
		uart->rxhead = head;
	}
}

static void uart_isr(struct uart_port *uart)
{
	uint32_t mis, err, udma_int;

	err = 0;
	mis = HWREG(uart->base+UART_O_MIS);
	if (mis)
		HWREG(uart->base+UART_O_ICR) = mis;
	udma_int = HWREG(UDMA_CHIS);
	if (udma_int & (1 << uart->tx_dmach)) {
		HWREG(uart->base+UART_O_DMACTL) &= ~UART_DMA_TX;
		HWREG(UDMA_CHIS) = (1 << uart->tx_dmach);
		uart->txdma = 0;
	}
	if (uart->rx_dmach != 0 && (udma_int & (1 << uart->rx_dmach))) {
		HWREG(uart->base+UART_O_DMACTL) &= ~UART_DMA_RX;
		HWREG(UDMA_CHIS) = (1 << uart->rx_dmach);
		uart->rxhead = uart->dmalen;
		uart->rxdma = 0;
	}

	if (mis & UART_INT_OE) {
		uart->oerr++;
		err = 1;
	}
	if (mis & UART_INT_FE) {
		uart->ferr++;
		err = 1;
	}
	if (err)
		HWREG(uart->base+UART_O_ECR) = 0x0f;
	if ((mis & UART_INT_RX) || (mis & UART_INT_RT))
		uart_recv(uart);
}

void uart0_isr(void)
{
	uart_isr(uartms);
	uart0_isr_nums++;
}

void uart1_isr(void)
{
	uart_isr(uartms+1);
	uart1_isr_nums++;
}

void uart_wait_dma(int port)
{
	struct uart_port *uart = uartms + port;

	while (uart->txdma || uart->rxdma)
		tm4c_waitint();
}

int uart_in_dma(int port)
{
	struct uart_port *uart = uartms + port;
	return uart->txdma || uart->rxdma;
}

void uart_wait(int port)
{
	struct uart_port *uart = uartms + port;

	while (HWREG(uart->base+UART_O_FR) & UART_FR_BUSY)
		;
}
