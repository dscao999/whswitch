.section .rodata.picture, "a", %progbits
.global png_begin
.type png_begin, %object
.align 2
png_begin:
.incbin "png/cane-corso.png"
.global png_end
.type png_end, %object
png_end:
