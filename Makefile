#
# Include the common make definitions.
#

.PHONY: all clean clean_all

all: nswitch

include makedefs

LIBDIR := tm4c
LIBSRC = $(wildcard $(LIBDIR)/*.c)
LIBOBJ = $(subst .c,.o, $(LIBSRC))

INCLS := -isystem syslib -I$(LIBDIR)
CFLAGS += -DTARGET_IS_TM4C123_RB2 -DPART_TM4C123GH6PM $(INCLS)
LDSCRIPT := tm4c/tm4c.ld

#
# The default rule, which causes the driver library to be built.
#

#tm4c/libtm4c.a: $(LIBOBJ)
#	$(AR) rv $@ $?

nswitch: switch_main.o oled.o $(LIBOBJ)
	$(LD) $(LDFLAGS) $^ -o $@
#
# The rule to clean out all the build products.
#
clean:
	$(MAKE) --directory=tm4c $@
	rm -rf nswitch nswitch.bin *.o

clean_all: clean
	$(MAKE) --directory=tm4c $@
	rm -f *.d

ifneq (${MAKECMDGOALS},clean)
-include $(wildcard tm4c/*.d) __dummy__
-include $(wildcard *.d) __dummy__
endif
