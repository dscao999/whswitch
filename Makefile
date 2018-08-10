#
# Include the common make definitions.
#

.PHONY: all clean clean_inc
all: blinky

include makedefs

SBCDIR := tm4c
VPATH = $(SBCDIR)

sbcsrcs := $(wildcard $(SBCDIR)/*.c)
sbcobjs := $(notdir $(sbcsrcs:.c=.o))

INCLS := -isystem syslib -I$(SBCDIR)
CFLAGS += -DTARGET_IS_TM4C123_RB2 -DPART_TM4C123GH6PM $(INCLS)
LDSCRIPT := $(SBCDIR)/tm4c.ld

#
# The default rule, which causes the driver library to be built.
#

blinky: auto_raise.o ssi_display.o display_blink.o qei_position.o \
	 timer_task.o led_blink.o uart_op.o uart_laser.o $(sbcobjs)
	$(LD) $(LDFLAGS) $^ -o $@
#
# The rule to clean out all the build products.
#
clean:
	rm -rf blinky blinky.bin *.o

clean_inc: clean
	rm -f *.d
