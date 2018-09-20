/*
 *  OLED driver for SD1331 chip, 96x64
 *  Dashi Cao, dscao999@hotmail.com, caods1@lenovo.com
 */
#include "oled.h"

static inline int oled_reset_addr(struct oled_ctrl *od, int idx)
{
	od->pbuf[idx+0] = 0x15;
	od->pbuf[idx+1] = 0x00;
	od->pbuf[idx+2] = 0x5f;
	od->pbuf[idx+3] = 0x75;
	od->pbuf[idx+4] = 0x00;
	od->pbuf[idx+5] = 0x3f;
	return idx + 6;
}

static inline void oled_set_color(struct oled_ctrl *od, int color)
{
	uint16_t *cdx;
	int i;

	switch(color % 3) {
	case 0:
		color = 0x001f;
		break;
	case 1:
		color = 0x07e0;
		break;
	case 2:
		color = 0xf800;
		break;
	default:
		color = 0xffff;
		break;
	}
		
	cdx = (uint16_t *)od->pbuf;
	for (i = 0; i < 512; i++)
		*cdx++ = color;
}

void oled_picset(struct oled_ctrl *od, int color)
{
	int i, pos;

	pos = oled_reset_addr(od, 0);
	tm4c_ssi_rwstart(0, od->pbuf, od->tbuf, pos);
	
	oled_set_color(od, color);

	tm4c_gpio_write(od->cp, od->cmdpin, 1);
	for (i = 0; i < 12; i++) {
		while (tm4c_ssi_rwst(0))
			tm4c_waitint();
		tm4c_ssi_rwstart(0, od->pbuf, od->tbuf, 1024);
	}
	while (tm4c_ssi_rwst(0))
		tm4c_waitint();
	tm4c_gpio_write(od->cp, od->cmdpin, 0);
}

void oled_reset(struct oled_ctrl *oled)
{
	tm4c_gpio_write(oled->cp, oled->cmdpin, 0);
	tm4c_gpio_write(oled->cp, oled->rstpin, 1);
	tm4c_ledlit(RED, 1);
	tm4c_delay(500);
	tm4c_ledlit(RED, 0);

	tm4c_gpio_write(oled->cp, oled->rstpin, 0);
	tm4c_delay(10);
	tm4c_gpio_write(oled->cp, oled->rstpin, 1);
	tm4c_ssi_setup(0);

	oled->pbuf[0] = 0xA0;
	oled->pbuf[1] = 0x40;
	tm4c_ssi_rwstart(0, oled->pbuf, oled->tbuf, 2);

	oled_picset(oled, 0);

	oled_display_onoff(oled, 1);
}
