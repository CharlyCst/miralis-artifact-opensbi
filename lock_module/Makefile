obj-m += lock_module.o

CROSS_COMPILE = riscv64-linux-gnu-

KDIR=../linux

all:
	make ARCH=riscv CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean
