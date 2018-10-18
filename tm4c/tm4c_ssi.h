#ifndef TM4C_SSI_DSCAO__
#define TM4C_SSI_DSCAO__
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/ssi.h"
#include "driverlib/rom.h"

struct ssi_port {
	uint32_t base;
	char *buf;
	uint16_t buflen;
	volatile uint16_t len;
	uint8_t tx_dmach;
	uint8_t rx_dmach;
	volatile uint8_t dma;
	uint8_t overrun;
};

void tm4c_ssi_setup(int port);

void ssi0_isr(void);

int tm4c_ssi_rwstart(int port, const char *txbuf, char *rvbuf, int len);
int tm4c_ssi_rwlen(int port);
int tm4c_ssi_rwstop(int port);
int tm4c_ssi_rwst(int port);

#endif /* TM4C_SSI_DSCAO__ */
