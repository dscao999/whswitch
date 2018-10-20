.section .rodata.picture, "a", %progbits
.global png_red
.type png_red, %object
.align 2
png_red:
.incbin "tools/can.ssd"
.global png_green
.type png_green, %object
png_green:
.incbin "tools/tpic_g.ssd"
.global png_blue
.type png_blue, %object
png_blue:
.incbin "tools/tpic_b.ssd"
.global png_dog
.type png_dog, %object
png_dog:
.incbin "tools/dog.ssd"
