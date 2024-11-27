obj-m += benchmark_module.o
CROSS_COMPILE = riscv64-linux-gnu-
KDIR=../linux

all:
	make ARCH=riscv CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean
test:
	make -C $(KDIR) M=$(PWD) modules
board:
	make -C /lib/modules/5.15.0/build M=$(PWD) modules
