#
# Include the common make definitions.
#

.PHONY: all clean clean_all

all: libtm4c.a

sbcsrcs := $(wildcard *.c)
sbcobjs := $(sbcsrcs:.c=.o)

include makedefs

INCLS := -isystem ../syslib
CFLAGS += -DTARGET_IS_TM4C123_RB2 -DPART_TM4C123GH6PM $(INCLS)

#
#

libtm4c.a: $(sbcobjs)
	$(AR) rv $@ $?
#
# The rule to clean out all the build products.
#
clean:
	rm -rf libtm4c.a *.o

clean_all: clean
	rm -f *.d

ifneq (${MAKECMDGOALS},clean)
-include $(wildcard *.d) __dummy__
endif
