obj-m += lock_module.o

KDIR=../linux

all:
	make ARCH=riscv CROSS_COMPILE=riscv64-unknown-linux-gnu- -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean
