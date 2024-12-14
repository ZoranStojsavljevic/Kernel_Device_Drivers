### How to Compile and Run:
### Compile: Use a Makefile to compile the kernel module.
### Create a Makefile like this:

obj-m += i218_lm.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

