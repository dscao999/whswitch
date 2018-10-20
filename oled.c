/*
 *  OLED driver for SD1331 chip, 96x64
 *  Dashi Cao, dscao999@hotmail.com, caods1@lenovo.com
 */
#include "miscutils.h"
#include "oled.h"

static inline void reset_start_addr(struct oled_ctrl *od)
{
	char buf[8];

	buf[0] = 0x15;
	buf[1] = 0x00;
	buf[2] = 0x5f;
	buf[3] = 0x75;
	buf[4] = 0x00;
	buf[5] = 0x3f;
	tm4c_ssi_wdma(od->ssi);
	tm4c_ssi_rwstart(od->ssi, buf, od->tbuf, 6, NULL);
	tm4c_ssi_wdma(od->ssi);
}

static inline void ssi_wait(int port)
{
	while (tm4c_ssi_rwst(port))
		tm4c_waitint();
}

void oled_fill_display(struct oled_ctrl *od, int color)
{
	char *pix, *dmabuf[2], *rgb[4];
	extern char png_red, png_green, png_blue, png_dog;
	volatile uint8_t pipo;
	uint8_t opipo, idx;
	uint16_t len;

	reset_start_addr(od);
	rgb[0] = &png_red;
	rgb[1] = &png_green;
	rgb[2] = &png_blue;
	rgb[3] = &png_dog;

	len = od->width * 2;
	pix = rgb[color % 4];
	opipo = 0;
	dmabuf[0] = od->pbuf;
	dmabuf[1] = od->abuf;

	ssi_wait(od->ssi);
	tm4c_gpio_write(od->cp, od->cmdpin, 1);

	memcpy4(dmabuf[0], pix, len);
	pix += len;
	tm4c_ssi_rwstart(od->ssi, dmabuf[0], od->tbuf, len, &pipo);

	do {
		idx = ((opipo + 2) >> 1) & 1;
		memcpy4(dmabuf[idx], pix, len);
		pix += len;
		tm4c_ssi_rwresch(od->ssi, dmabuf[idx], od->tbuf, len, idx);
		while ((opipo + 2) != pipo)
			tm4c_waitint();
		opipo = pipo;
	} while (((opipo + 2) >> 1) < od->height);
	while ((opipo + 2) != pipo)
		tm4c_waitint();
	tm4c_ssi_rwstop(od->ssi);
	ssi_wait(od->ssi);
	tm4c_delay_cycles(8);
	tm4c_gpio_write(od->cp, od->cmdpin, 0);
}

void oled_reset(struct oled_ctrl *od, int color)
{
	tm4c_ledlit(RED, 1);
	tm4c_delay(200);
	tm4c_gpio_write(od->cp, od->rstpin, 0);
	tm4c_delay(10);
	tm4c_gpio_write(od->cp, od->rstpin, 1);
	tm4c_delay(200);

	od->pbuf[0] = 0xA0;
	od->pbuf[1] = 0x40;
	tm4c_ssi_rwstart(od->ssi, od->pbuf, od->tbuf, 2, NULL);

	oled_fill_display(od, color);
	oled_display_onoff(od, 1);
	tm4c_ledlit(RED, 0);
}

void oled_init(struct oled_ctrl *od, int ssiport, int color)
{
	od->ssi = ssiport;
	tm4c_ssi_setup(od->ssi);
	tm4c_gpio_write(od->cp, od->cmdpin, 0);
	tm4c_gpio_write(od->cp, od->rstpin, 1);

	oled_reset(od, color);
}
