#
# Include the common make definitions.
#

.PHONY: all clean clean_all
all: nswitch

include makedefs

SBCDIR := tm4c

INCLS := -isystem syslib -I$(SBCDIR)
CFLAGS += -DTARGET_IS_TM4C123_RB2 -DPART_TM4C123GH6PM $(INCLS)
LDSCRIPT := tm4c/tm4c.ld

#
# The default rule, which causes the driver library to be built.
#

nswitch: switch_main.o
	$(LD) $(LDFLAGS) $^ -ltm4c123 -o $@
#
# The rule to clean out all the build products.
#
clean:
	rm -rf nswitch nswitch.bin *.o

clean_all: clean
	rm -f *.d
