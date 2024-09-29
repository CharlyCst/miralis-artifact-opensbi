TARGETS_LINUX_BIN = opensbi-linux-exit.bin opensbi-linux-shell.bin opensbi-linux-driver.bin
INIT_RAMFS = initramfs_shell initramfs_exit initramfs_driver
LINUX = linux_shell linux_exit linux_driver
OPEN_SBI = opensbi.bin opensbi_jump.bin
UBOOT = u-boot u-boot-exit

all:  $(UBOOT) $(OPEN_SBI) $(TARGETS_LINUX_BIN) 

CROSS_COMPILE = riscv64-linux-gnu-
PATCHES = ../miralis_firmware.patch
INIT = shell
DRIVER_PATH = ../driver

.PHONY: opensbi.bin opensbi_jump.bin driver $(LINUX) $(TARGETS_LINUX_BIN) clean

ifeq ($(shell uname -o), Darwin)
	CROSS_COMPILE = riscv64-elf-
	PATCHES += ../miralis_firmware_macos.patch
endif

opensbi:
	-git clone --depth 1 --branch v1.4 https://github.com/riscv-software-src/opensbi.git
	cd opensbi && git apply $(PATCHES)

$(INIT_RAMFS):
	sudo cp init_$(INIT) ramfs-riscv/init
	sudo chmod +x ramfs-riscv/init
	mkdir -p ramfs-riscv/dev 
	-cd ramfs-riscv/dev; \
	sudo mknod null c 1 3; \
	sudo mknod console c 5 1; \
	sudo mknod tty c 5 0;
	cd ramfs-riscv; \
	find . | cpio -o -H newc | gzip > ../initramfs_$(INIT).cpio.gz

$(LINUX):
	-git clone --depth 1 --branch v6.10 https://github.com/torvalds/linux.git
	make -C linux ARCH=riscv CROSS_COMPILE=$(CROSS_COMPILE) defconfig
	make -C linux ARCH=riscv \
		CROSS_COMPILE=$(CROSS_COMPILE) \
		CONFIG_INITRAMFS_SOURCE=../initramfs_$(INIT).cpio.gz \
		-j`nproc` 

driver:
	make -C linux M=$(DRIVER_PATH) modules ARCH=riscv CROSS_COMPILE=$(CROSS_COMPILE)
	cp driver/driver.ko ramfs-riscv/driver.ko
	make -C linux M=$(DRIVER_PATH) clean

opensbi-linux-driver.bin: INIT=driver
opensbi-linux-driver.bin: driver linux_driver
linux_driver: initramfs_driver

opensbi-linux-exit.bin: INIT=exit
opensbi-linux-exit.bin: linux_exit
linux_exit: initramfs_exit

opensbi-linux-shell.bin: INIT=shell
opensbi-linux-shell.bin: linux_shell
linux_shell: initramfs_shell

$(TARGETS_LINUX_BIN): opensbi
	make -C opensbi PLATFORM=generic \
		O=build_$(INIT) \
		FW_PAYLOAD=y \
		FW_PAYLOAD_PATH=../linux/arch/riscv/boot/Image \
		FW_PAYLOAD_ALIGN=0x200000 \
		FW_DYNAMIC=n \
		FW_JUMP=n \
		CROSS_COMPILE=$(CROSS_COMPILE) \
		-j`nproc`
	cp opensbi/build_$(INIT)/platform/generic/firmware/fw_payload.bin opensbi-linux-kernel-$(INIT).bin
	cp opensbi/build_$(INIT)/platform/generic/firmware/fw_payload.elf opensbi-linux-kernel-$(INIT).elf

opensbi.bin: opensbi
	make -C opensbi PLATFORM=generic FW_PAYLOAD=y FW_DYNAMIC=n FW_JUMP=n CROSS_COMPILE=$(CROSS_COMPILE) -j`nproc`
	cp opensbi/build/platform/generic/firmware/fw_payload.bin opensbi.bin
	cp opensbi/build/platform/generic/firmware/fw_payload.elf opensbi.elf

opensbi_jump.bin: opensbi
	make -C opensbi PLATFORM=generic FW_JUMP=y FW_DYNAMIC=n FW_PAYLOAD=n FW_JUMP_ADDR=0x80400000 CROSS_COMPILE=$(CROSS_COMPILE) -j`nproc`
	cp opensbi/build/platform/generic/firmware/fw_jump.bin opensbi_jump.bin
	cp opensbi/build/platform/generic/firmware/fw_jump.elf opensbi_jump.elf


u-boot:
	git clone --depth 1 --branch v2024.10-rc5 https://github.com/u-boot/u-boot.git
	cd u-boot && git apply ../u-boot_patch.patch
	cd u-boot && make CROSS_COMPILE=riscv64-linux-gnu- qemu-riscv64_smode_defconfig
	cd u-boot && make CROSS_COMPILE=riscv64-linux-gnu-
	cp u-boot/u-boot.bin u-boot.bin
	cp u-boot/u-boot u-boot.elf
	rm -rf u-boot

u-boot-exit:
	git clone --depth 1 --branch v2024.10-rc5 https://github.com/u-boot/u-boot.git
	cd u-boot && git apply ../u-boot_patch_ci_cd.patch
	cd u-boot && make CROSS_COMPILE=riscv64-linux-gnu- qemu-riscv64_smode_defconfig
	cd u-boot && make CROSS_COMPILE=riscv64-linux-gnu-
	cp u-boot/u-boot.bin u-boot-exit.bin
	cp u-boot/u-boot u-boot-exit.elf
	rm -rf u-boot

test-u-boot:
	qemu-system-riscv64 \
	-machine virt -nographic -m 2048 -smp 4 \
	-bios opensbi_jump.bin \
	-device loader,file=u-boot/u-boot.bin,addr=0x80400000 

clean:
	-rm -rf u-boot opensbi linux *.bin *.elf *.cpio.gz
