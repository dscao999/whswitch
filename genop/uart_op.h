#ifndef UART_OP_DSCAO__
#define UART_OP_DSCAO__

#define UART_BUFLEN	80
struct uart_param {
	uint8_t port;
	uint8_t len;
	char buf[UART_BUFLEN];
};
static inline char *uart_buf(struct uart_param *up)
{
	return up->buf;
}

int uart_op(struct uart_param *p);

#endif /* UART_OP_DSCAO__ */
