CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra -std=c99
all: rgbgen rgbx
rgbgen: rgbgen.c
    $(CC) $(CFLAGS) -o $@ $< -lm
rgbx: rgbx.c
    $(CC) $(CFLAGS) -o $@ $< -lX11
clean:
    rm -f rgbgen rgbx
.PHONY: all clean
