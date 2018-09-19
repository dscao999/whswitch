/*
 *  OLED driver for SD1331 chip, 96x64
 *  Dashi Cao, dscao999@hotmail.com, caods1@lenovo.com
 */
#include "oled.h"

static void oled_picset(struct oled_ctrl *od)
{
	uint16_t color, *cdx;
	int i;

	color = 0x001f;
	cdx = (uint16_t *)od->pbuf;
	for (i = 0; i < 512; i++, cdx++)
		*cdx = color;
	tm4c_gpio_write(od->cp, od->cmdpin, 1);
	for (i = 0; i < 6; i++)
		tm4c_ssi_rwstart(0, od->pbuf, od->tbuf, 1024);
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
	oled_picset(oled);
	oled_display_onoff(oled, 1);
}
