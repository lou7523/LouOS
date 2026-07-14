IMG = os.img
BOOT = boot_sect_simple.bin
KERNEL_ASM = kernel.asm
KERNEL_C = kernel.c
KERNEL_BIN = kernel.bin
PROGRAMA = programa.elf
PROGRAMA1 = programa1.elf
PROGRAMA2 = programa2.elf
PROGRAMA3 = programa3.elf
TERMINAL = terminal.o

all: $(IMG)

$(BOOT): boot_sect_simple.asm
	nasm -f bin $< -o $@

$(PROGRAMA): programa.c programa.ld
	gcc -m32 -ffreestanding -nostdlib -nostartfiles -fno-pie -static -T programa.ld -o $@ programa.c

$(PROGRAMA1): programa1.c programa1.ld
	gcc -m32 -ffreestanding -nostdlib -nostartfiles -fno-pie -static -T	programa1.ld -o $@ programa1.c

$(PROGRAMA2): programa2.c programa2.ld
	gcc -m32 -ffreestanding -nostdlib -nostartfiles -fno-pie -static -T	programa2.ld -o $@ programa2.c

$(PROGRAMA3): programa3.c programa3.ld
	gcc -m32 -ffreestanding -nostdlib -nostartfiles -fno-pie -static -T	programa3.ld -o $@ programa3.c

$(KERNEL_BIN): $(KERNEL_ASM) $(KERNEL_C) terminal.c
	nasm -f elf $(KERNEL_ASM) -o kernel.o
	gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -c $(KERNEL_C) -o kernel_c.o
	gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -c terminal.c -o terminal.o
	ld -m elf_i386 -T link.ld --oformat binary -o $(KERNEL_BIN) kernel.o kernel_c.o terminal.o

$(IMG): $(BOOT) $(KERNEL_BIN) $(PROGRAMA) $(PROGRAMA1) $(PROGRAMA2) $(PROGRAMA3)
	dd if=/dev/zero of=$(IMG) bs=1M count=512
	mkfs.fat -F 32 $(IMG)
	dd if=$(BOOT) of=$(IMG) bs=1 count=3 conv=notrunc
	dd if=$(BOOT) of=$(IMG) bs=1 skip=90 seek=90 count=422 conv=notrunc
	mcopy -i $(IMG) $(KERNEL_BIN) ::KERNEL.BIN
	mcopy -i $(IMG) $(PROGRAMA) ::PROGRAMA.ELF
	mcopy -i $(IMG) $(PROGRAMA1) ::PROGRAM1.ELF
	mcopy -i $(IMG) $(PROGRAMA2) ::PROGRAM2.ELF
	mcopy -i $(IMG) $(PROGRAMA3) ::PROGRAM3.ELF

run: $(IMG)
	qemu-system-i386 -drive format=raw,file=$(IMG),if=ide,index=0 -boot order=c -machine pc-i440fx-3.1 -s

clean:
	rm -f $(KERNEL_BIN) $(IMG) kernel.o kernel_c.o terminal.o $(PROGRAMA) $(PROGRAMA1) $(PROGRAMA2) $(PROGRAMA3)

.PHONY: all run clean