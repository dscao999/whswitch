#ifndef TM4C_DMA_DSCAO__
#define TM4C_DMA_DSCAO__
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_udma.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom.h"

struct dmactl {
	const void *src;
	void *dst;
	uint32_t ctrl;
	uint32_t reserv1;
};

#define MAX_DMALEN	1024

void tm4c_dma_enable(void);
int tm4c_dma_rem(int ch);

void udma_error_isr(void);
	
#endif /* TM4C_DMA_DSCAO__ */
