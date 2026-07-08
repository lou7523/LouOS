IMG = os.img
BOOT = boot_sect_simple.bin
KERNEL_ASM = kernel.asm
KERNEL_C = kernel.c
KERNEL_BIN = kernel.bin
PROGRAMA = programa.elf

all: $(IMG)

$(BOOT): boot_sect_simple.asm
	nasm -f bin $< -o $@

$(PROGRAMA): programa.c programa.ld
	gcc -m32 -ffreestanding -nostdlib -nostartfiles -fno-pie -static -T programa.ld -o $@ programa.c

$(KERNEL_BIN): $(KERNEL_ASM) $(KERNEL_C)
	nasm -f elf $(KERNEL_ASM) -o kernel.o
	gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -c $(KERNEL_C) -o kernel_c.o
	ld -m elf_i386 -T link.ld --oformat binary -o $(KERNEL_BIN) kernel.o kernel_c.o

$(IMG): $(BOOT) $(KERNEL_BIN) $(PROGRAMA)
	dd if=/dev/zero of=$(IMG) bs=1M count=512
	mkfs.fat -F 32 $(IMG)
	dd if=$(BOOT) of=$(IMG) bs=1 count=3 conv=notrunc
	dd if=$(BOOT) of=$(IMG) bs=1 skip=90 seek=90 count=422 conv=notrunc
	mcopy -i $(IMG) $(KERNEL_BIN) ::KERNEL.BIN
	mcopy -i $(IMG) $(PROGRAMA) ::PROGRAMA.ELF

run: $(IMG)
	qemu-system-i386 -drive format=raw,file=$(IMG),if=ide,index=0 -boot order=c -machine pc-i440fx-3.1 -s

clean:
	rm -f $(KERNEL_BIN) $(IMG) kernel.o kernel_c.o $(PROGRAMA)

.PHONY: all run clean