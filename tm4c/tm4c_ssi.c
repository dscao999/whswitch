#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ssi.h"
#include "inc/hw_ints.h"
#include "inc/hw_gpio.h"
#include "inc/hw_udma.h"
#include "driverlib/ssi.h"
#include "driverlib/udma.h"
#include "miscutils.h"
#include "tm4c_setup.h"
#include "tm4c_dma.h"
#include "tm4c_ssi.h"

static uint32_t ssi0_isr_nums = 0;

static struct ssi_port ssims[] = {
	{
		.base = SSI0_BASE,
		.tx_dmach = UDMA_CHANNEL_SSI0TX,
		.rx_dmach = UDMA_CHANNEL_SSI0RX,
		.buf = NULL,
	}
};

void tm4c_ssi_setup(int port)
{
	struct ssi_port *ssi;
	uint32_t sysperip, imask;
	int intr;

	ssi = ssims+port;
	switch (port) {
	case 0:
		ROM_GPIOPinTypeSSI(GPIO_PORTA_BASE,
			GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5);
		ROM_GPIOPinConfigure(GPIO_PA2_SSI0CLK);
		ROM_GPIOPinConfigure(GPIO_PA3_SSI0FSS);
		ROM_GPIOPinConfigure(GPIO_PA4_SSI0RX);
		ROM_GPIOPinConfigure(GPIO_PA5_SSI0TX);
		sysperip = SYSCTL_PERIPH_SSI0;
		intr = INT_SSI0_TM4C123;
		break;
	default:
		while(1)
			;
	}

	ROM_SysCtlPeripheralEnable(sysperip);
	while(!ROM_SysCtlPeripheralReady(sysperip))
			;
	ROM_SSIClockSourceSet(ssi->base, SSI_CLOCK_SYSTEM);
	ROM_SSIConfigSetExpClk(ssi->base, HZ, SSI_FRF_MOTO_MODE_0,
		SSI_MODE_MASTER, 4000000, 8);

	ROM_uDMAChannelAttributeDisable(ssi->tx_dmach, UDMA_ATTR_ALL);
	ROM_uDMAChannelControlSet(ssi->tx_dmach|UDMA_PRI_SELECT,
		UDMA_SIZE_8|UDMA_SRC_INC_8|UDMA_DST_INC_NONE|UDMA_ARB_8);
	ROM_uDMAChannelControlSet(ssi->tx_dmach|UDMA_ALT_SELECT,
		UDMA_SIZE_8|UDMA_SRC_INC_8|UDMA_DST_INC_NONE|UDMA_ARB_8);
	ROM_uDMAChannelAttributeDisable(ssi->rx_dmach, UDMA_ATTR_ALL);
	ROM_uDMAChannelControlSet(ssi->rx_dmach|UDMA_PRI_SELECT,
		UDMA_SIZE_8|UDMA_SRC_INC_NONE|UDMA_DST_INC_8|UDMA_ARB_8);
	ROM_uDMAChannelControlSet(ssi->rx_dmach|UDMA_ALT_SELECT,
		UDMA_SIZE_8|UDMA_SRC_INC_NONE|UDMA_DST_INC_8|UDMA_ARB_8);

	imask = SSI_IM_RXIM|SSI_IM_RTIM|SSI_IM_RORIM;
	HWREG(ssi->base+SSI_O_IM) |= imask;
	ROM_IntPrioritySet(intr, 0xc0);
	ROM_IntEnable(intr);
	ROM_SSIEnable(ssi->base);
}

static int ssi_write_direct(struct ssi_port *ssi, const char *buf, int len)
{
	const uint8_t *ubuf;
	int i;

	for (i = 0, ubuf = (const uint8_t *)buf; i < len; ubuf++, i++) {
		while((HWREG(ssi->base+SSI_O_SR) & SSI_SR_TNF) == 0)
			;
		HWREG(ssi->base+SSI_O_DR) = *ubuf;
	}
	return len;
}

int tm4c_ssi_rwstop(int port)
{
	struct ssi_port *ssi = ssims + port;
	int len;

	if (!ssi->buf)
		return 0;
	len = ssi->len;
	if (ssi->dma) {
		HWREG(ssi->base+SSI_O_DMACTL) &= ~(SSI_DMA_TX|SSI_DMA_RX);
		HWREG(UDMA_ENACLR) = (1 << ssi->tx_dmach)|(1 << ssi->rx_dmach);
		len = ssi->buflen - ROM_uDMAChannelSizeGet(ssi->tx_dmach|UDMA_PRI_SELECT);
		ssi->dma = 0;
	}
	return len;
}

int tm4c_ssi_rwst(int port)
{
	struct ssi_port *ssi = ssims + port;

	if (!ssi->buf || ssi->dma == 0)
		return HWREG(ssi->base+SSI_O_SR) & SSI_SR_BSY;
	return 1;
}

int tm4c_ssi_rwlen(int port)
{
	struct ssi_port *ssi = ssims + port;
	int len;

	if (!ssi->buf)
		return 0;
	len =ssi->len;
	if (ssi->dma)
		len = ssi->len - ROM_uDMAChannelSizeGet(ssi->tx_dmach|UDMA_PRI_SELECT);
	return len;
}

int tm4c_ssi_rwstart(int port, const char *txbuf, char *rvbuf, int len)
{
	struct ssi_port *ssi = ssims + port;

	if (len <= 0 || len > MAX_DMALEN)
		return -1;
	if (ssi->dma)
		return -2;

	ssi->buf = rvbuf;
	ssi->buflen = len;
	ssi->len = 0;
	if (txbuf < (char *)MEMADDR || len < 16) {
		ssi_write_direct(ssi, txbuf, len);
		while (ssi->len != len)
			tm4c_waitint();
		ssi->buf = NULL;
		return len;
	}

	ROM_uDMAChannelTransferSet(ssi->tx_dmach|UDMA_PRI_SELECT,
		UDMA_MODE_BASIC, (void *)txbuf, (void *)(ssi->base+SSI_O_DR), len);
	ROM_uDMAChannelTransferSet(ssi->rx_dmach|UDMA_PRI_SELECT,
		UDMA_MODE_BASIC, (void *)(ssi->base+SSI_O_DR), rvbuf, len);
	HWREG(UDMA_ENASET) = (1 << ssi->rx_dmach)|(1 << ssi->tx_dmach);
	HWREG(ssi->base+SSI_O_DMACTL) |= (SSI_DMA_TX|SSI_DMA_RX);
	ssi->dma = 2;
	return 0;
}

static void tm4c_ssi_recv(struct ssi_port *ssi)
{
	uint8_t byte, count;

	count = 0;
	while ((HWREG(ssi->base+SSI_O_SR) & SSI_SR_RNE) && count < 8) {
		byte = HWREG(ssi->base+SSI_O_DR);
		if (ssi->buf)
			ssi->buf[ssi->len++] = byte;
		count++;
	}
}

static void ssi_isr(struct ssi_port *ssi)
{
	uint32_t mis, udma_int;

	mis = HWREG(ssi->base+SSI_O_MIS);
	if (mis & (SSI_MIS_RTMIS|SSI_MIS_RORMIS))
		HWREG(ssi->base+SSI_O_ICR) =(SSI_MIS_RTMIS|SSI_MIS_RORMIS);
	udma_int = HWREG(UDMA_CHIS);
	if (udma_int & ((1 << ssi->rx_dmach)|(1 << ssi->tx_dmach))) {
		if (udma_int & (1 << ssi->rx_dmach)) {
			HWREG(ssi->base+SSI_O_DMACTL) &= ~SSI_DMA_RX;
			HWREG(UDMA_CHIS) = (1 << ssi->rx_dmach);
		}
		if (udma_int & (1 << ssi->tx_dmach)) {
			HWREG(ssi->base+SSI_O_DMACTL) &= ~SSI_DMA_TX;
			HWREG(UDMA_CHIS) = (1 << ssi->tx_dmach);
		}
		ssi->dma--;
	}
	if (mis & (SSI_MIS_RXMIS|SSI_MIS_RTMIS))
		tm4c_ssi_recv(ssi);
	if (mis & SSI_MIS_RORMIS)
		ssi->overrun++;
}

void ssi0_isr(void)
{
	ssi_isr(ssims);
	ssi0_isr_nums++;
}
