
INCLUDE tm4c/tm4c_mem.ld

SECTIONS
{
	INCLUDE tm4c/tm4c.ld

	.rodata.picture :
	{
		KEEP(*(.rodata.picture))
	} > FLASH
}
