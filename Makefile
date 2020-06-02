obj-m += Mymod.o

Mymod-objs := circular_buffer.o soft_uart.o module.o
RELEASE = $(shell uname -r)
LINUX_HEADERS = /usr/src/linux-headers-$(RELEASE)

all:
	$(MAKE) -C $(LINUX_HEADERS) M=$(PWD) modules
clean:
	$(MAKE) -C $(LINUX_HEADERS) M=$(PWD) clean


