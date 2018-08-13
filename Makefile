#
# Include the common make definitions.
#

.PHONY: all clean clean_all libtm4c

all: nswitch

include makedefs

SBCDIR := tm4c

INCLS := -isystem syslib -I$(SBCDIR)
CFLAGS += -DTARGET_IS_TM4C123_RB2 -DPART_TM4C123GH6PM $(INCLS)
LDSCRIPT := tm4c/tm4c.ld
LIBDIR := tm4c

#
# The default rule, which causes the driver library to be built.
#

libtm4c:
	$(MAKE) --directory=tm4c

nswitch: switch_main.o libtm4c
	$(LD) $(LDFLAGS) $< -L$(LIBDIR) -ltm4c -o $@
#
# The rule to clean out all the build products.
#
clean:
	$(MAKE) --directory=tm4c $@
	rm -rf nswitch nswitch.bin *.o

clean_all: clean
	$(MAKE) --directory=tm4c $@
	rm -f *.d
