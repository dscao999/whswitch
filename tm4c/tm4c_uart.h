#ifndef TM4C_UART_DSCAO__
#define TM4C_UART_DSCAO__
#include <stdint.h>

#define UART_BUFSIZ	128
struct uart_port {
	uint32_t base;
	uint32_t at_tick;
	uint16_t oerr;
	uint16_t ferr;
	uint16_t dmalen;
	uint8_t tx_dmach;
	uint8_t rx_dmach;
	volatile uint8_t txdma, rxdma;
	volatile uint8_t rxhead;
	uint8_t rxtail;
	uint8_t rxbuf[UART_BUFSIZ];
};

void uart_open(int port);
void uart_close(int port);

int uart_write(int port, const char *str, int len, int wait);
int uart_write_sync(int port, const char *str, int len);
void uart_write_cmd_expect(int port, const char cmd, int explen);

int uart_read(int port, char *buf, int len, int wait);
int uart_read_expect(int port, char *buf, int len);
int uart_in_dma(int port);
void uart_wait_dma(int port);
void uart_wait(int port);

void uart0_isr(void);
void uart1_isr(void);

#endif /* TM4C_UART_DSCAO__ */
