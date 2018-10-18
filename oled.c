/*
 *  OLED driver for SD1331 chip, 96x64
 *  Dashi Cao, dscao999@hotmail.com, caods1@lenovo.com
 */
#include "oled.h"

static inline int reset_start_addr(char *buf, int idx)
{
	buf[idx++] = 0x15;
	buf[idx++] = 0x00;
	buf[idx++] = 0x5f;
	buf[idx++] = 0x75;
	buf[idx++] = 0x00;
	buf[idx++] = 0x3f;
	return idx;
}

static inline void ssi_wait(int port)
{
	while (tm4c_ssi_rwst(port))
		tm4c_waitint();
}

static inline void oled_set_color(struct oled_ctrl *od, int color)
{
	uint16_t *cdx;
	int i;

	switch(color % 3) {
	case 0:
		color = 0x00f8;
		break;
	case 1:
		color = 0xe007;
		break;
	case 2:
		color = 0x1f00;
		break;
	default:
		color = 0xffff;
		break;
	}
		
	cdx = (uint16_t *)od->pbuf;
	for (i = 0; i < 512; i++)
		*cdx++ = color;
}

void oled_fill_display(struct oled_ctrl *od, int color)
{
	int i, pos;

	pos = reset_start_addr(od->pbuf, 0);
	ssi_wait(od->ssi);
	tm4c_ssi_rwstart(od->ssi, od->pbuf, od->tbuf, pos);

	oled_set_color(od, color);
	tm4c_gpio_write(od->cp, od->cmdpin, 1);
	for (i = 0; i < 12; i++) {
		ssi_wait(od->ssi);
		tm4c_ssi_rwstart(od->ssi, od->pbuf, od->tbuf, 1024);
	}
	ssi_wait(od->ssi);
	tm4c_gpio_write(od->cp, od->cmdpin, 0);
}

void oled_reset(struct oled_ctrl *od)
{
	tm4c_ledlit(RED, 1);
	tm4c_delay(200);
	tm4c_gpio_write(od->cp, od->rstpin, 0);
	tm4c_delay(10);
	tm4c_gpio_write(od->cp, od->rstpin, 1);
	tm4c_delay(200);

	od->pbuf[0] = 0xA0;
	od->pbuf[1] = 0x40;
	tm4c_ssi_rwstart(od->ssi, od->pbuf, od->tbuf, 2);

	oled_fill_display(od, 0);
	oled_display_onoff(od, 1);
	tm4c_ledlit(RED, 0);
}

void oled_init(struct oled_ctrl *od, int ssiport)
{
	od->ssi = ssiport;
	tm4c_ssi_setup(od->ssi);
	tm4c_gpio_write(od->cp, od->cmdpin, 0);
	tm4c_gpio_write(od->cp, od->rstpin, 1);

	oled_reset(od);
}
