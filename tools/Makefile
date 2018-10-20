
.PHONY: all clean

CFLAGS += -g
LDFLAGS += -g

all: ssdrgb png2nkk
ssdrgb: ssd_rgb.o
	$(LINK.o) $^ $(OUTPUT_OPTION)

png2nkk: png2nkk.o
	$(LINK.o) $^ -lpng -lz $(OUTPUT_OPTION)

clean:
	rm -f *.o ssdrgb png2nkk
