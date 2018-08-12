#include <stdint.h>
#include "tm4c_uart.h"
#include "uart_op.h"

int uart_op(struct uart_param *p)
{
	int count, i, echo;
	char *buf;

	echo = 0;
	buf = uart_buf(p);
	p->len = uart_read(p->port, buf, UART_BUFLEN, 0);
	for (i = 0; i < p->len; i++)
		if (*(buf+i) == 0)
			*(buf+i) = '\\';
	if (p->len && (*(buf+p->len-1) == 0x0d || *(buf+p->len-1) == 0x0a))
		echo = 1;
	return echo;
}
