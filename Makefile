#
# Include the common make definitions.
#

.PHONY: all clean clean_all tm4c_lib

all: nswitch

include makedefs

LIBDIR := tm4c
LIBSRC = $(wildcard tm4c/*.c)
LIBOBJ = $(subst .c,.o, $(LIBSRC))

SRC = $(wildcard *.c)
OBJ = $(subst .c,.o, $(SRC))
OBJ += piggy.o

INCLS := -isystem syslib -I$(LIBDIR)
CFLAGS += -DTARGET_IS_TM4C123_RB2 -DPART_TM4C123GH6PM $(INCLS)
LDSCRIPT := nswitch.ld

#
# The default rule, which causes the driver library to be built.
#

tm4c/libtm4c.a: $(LIBOBJ)
	$(MAKE) -C $(LIBDIR)

nswitch: $(OBJ) tm4c/libtm4c.a
	$(LD) $(LDFLAGS) $^ -L $(LIBDIR) -ltm4c -o $@
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
-include $(wildcard *.d) __dummy__
endif
