all: opensbi-linux-kernel.bin opensbi.bin

CROSS_COMPILE = riscv64-linux-gnu-
PATCHES = ../mirage_firmware.patch
INIT = shell

ifeq ($(shell uname -o), Darwin)
	CROSS_COMPILE = riscv64-elf-
	PATCHES += ../mirage_firmware_macos.patch
endif

opensbi:
	-git clone --depth 1 --branch v1.4 https://github.com/riscv-software-src/opensbi.git
	cd opensbi && git apply $(PATCHES)

.PHONY: initramfs
initramfs:
	sudo cp init_$(INIT) ramfs-riscv/init
	sudo chmod +x ramfs-riscv/init
	-cd ramfs-riscv/dev; \
	sudo mknod null c 1 3; \
	sudo mknod console c 5 1; \
	sudo mknod tty c 5 0;
	cd ramfs-riscv; \
	find . | cpio -o -H newc | gzip > ../initramfs_$(INIT).cpio.gz

.PHONY: linux
linux: initramfs
	-git clone --depth 1 https://github.com/torvalds/linux.git
	make -C linux ARCH=riscv CROSS_COMPILE=$(CROSS_COMPILE) defconfig
	make -C linux ARCH=riscv \
		CROSS_COMPILE=$(CROSS_COMPILE) \
		CONFIG_INITRAMFS_SOURCE=../initramfs_$(INIT).cpio.gz \
		-j`nproc` 

opensbi.bin: opensbi
	make -C opensbi PLATFORM=generic FW_PAYLOAD=y FW_DYNAMIC=n FW_JUMP=n CROSS_COMPILE=$(CROSS_COMPILE) -j`nproc`
	cp opensbi/build/platform/generic/firmware/fw_payload.bin opensbi.bin
	cp opensbi/build/platform/generic/firmware/fw_payload.elf opensbi.elf

opensbi-linux-kernel.bin: opensbi linux
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

.PHONY: clean
clean:
	-rm -rf opensbi linux *.bin *.elf *.cpio.gz
