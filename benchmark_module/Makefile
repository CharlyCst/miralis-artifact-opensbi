# This Makefile builds the kernel module used to communicate between the linux kernel and the miralis security monitor

obj-m += benchmark_module.o
CROSS_COMPILE = riscv64-linux-gnu-
KDIR=../linux

all:
	make ARCH=riscv CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean
test:
	make -C $(KDIR) M=$(PWD) modules
# This commands should be used inside visionfive2 board and is used to generate the kernel module specifically for this board only
# This commands works (and is tested) on the v3.6.1 branch
visionfive2-board:
	make -C /lib/modules/5.15.0/build M=$(PWD) modules
