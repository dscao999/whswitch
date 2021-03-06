#include <stdint.h>
#include <stdbool.h>
#include <driverlib/udma.h>
#include "tm4c_dma.h"

static struct dmactl dmacr[64] __attribute__((aligned(1024)));
uint32_t udmaerr = 0;

void tm4c_dma_enable(void)
{
	int i;
	struct dmactl *mc;

	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
	for (i = 0, mc = dmacr; i < 64; i++, mc++) {
		mc->src = 0;
		mc->dst = 0;
		mc->ctrl = 0;
		mc->reserv1 = 0;
	}
	while(!ROM_SysCtlPeripheralReady(SYSCTL_PERIPH_UDMA))
		;
	ROM_uDMAEnable();
	ROM_uDMAControlBaseSet(dmacr);
}

int tm4c_dma_rem(int ch)
{
	return (dmacr[ch].ctrl & UDMA_CHCTL_XFERSIZE_M) >> UDMA_CHCTL_XFERSIZE_S;
}

void udma_error_isr(void)
{
	uint32_t err;
	err = HWREG(UDMA_ERRCLR);
	if (err & 1) {
		HWREG(UDMA_ERRCLR) = 1;
		udmaerr++;
		err = HWREG(UDMA_ERRCLR);
	}
}
