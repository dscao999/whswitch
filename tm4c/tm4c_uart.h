#ifndef TM4C_UART_DSCAO__
#define TM4C_UART_DSCAO__
#include <stdint.h>

struct uart_port {
	uint32_t base;
	char *buf;
	uint8_t oerr;
	uint8_t ferr;
	uint8_t dma_ov;
	uint8_t tx_dmach;
	uint8_t rx_dmach;
	volatile uint8_t txdma:1, rxdma:1;
	volatile uint8_t len;
	uint8_t explen;
};

void uart_open(int port);
void uart_close(int port);

int uart_write(int port, const char *str, int len);
int uart_write_sync(int port, const char *str, int len);

int uart_read_start(int port, char *buf, int len);
int uart_read_bytes(int port);
int uart_read_stop(int port);

int uart_read_dma(int port);
int uart_write_dma(int port);
static inline void uart_write_dmawait(int port)
{
	while (uart_write_dma(port))
		tm4c_waitint();
}

static inline int uart_indma(int port)
{
	return uart_read_dma(port) || uart_write_dma(port);
}

void uart0_isr(void);
void uart1_isr(void);

#endif /* TM4C_UART_DSCAO__ */
