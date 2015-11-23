TARGET=sock2serial
CFLGAS=-O2
CC=mips-openwrt-linux-gcc
SRCS := $(wildcard *.c)
INCS := $(wildcard *.h)

.PHONY: clean

$(TARGET): $(SRCS) $(INCS)
	$(CC) $(SRCS) -o $@

clean:
	rm -rf *.o $(TARGET)
