IMG = os.img
BOOT = boot_sect_simple.bin
KERNEL_ASM = kernel.asm
KERNEL_C = kernel.c
KERNEL_BIN = kernel.bin

all: $(IMG)

$(BOOT): boot_sect_simple.asm
	nasm -f bin $< -o $@

$(IMG): $(BOOT) $(KERNEL_BIN)
	cat $(BOOT) $(KERNEL_BIN) > $(IMG)
	truncate -s 1440k $(IMG)

$(KERNEL_BIN): $(KERNEL_ASM) $(KERNEL_C)
	nasm -f elf $(KERNEL_ASM) -o kernel.o
	gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -c $(KERNEL_C) -o kernel_c.o
	ld -m elf_i386 -T link.ld --oformat binary -o $(KERNEL_BIN) kernel.o kernel_c.o

run: $(IMG)
	qemu-system-x86_64 -drive format=raw,file=$(IMG)

clean:
	rm -f $(KERNEL_BIN) $(IMG) kernel.o kernel_c.o

.PHONY: all run clean