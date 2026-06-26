IMG = os.img
BOOT = boot_sect_simple.bin
KERNEL_SRC = kernel.asm
KERNEL_BIN = kernel.bin

all: $(IMG)

$(IMG): $(BOOT) $(KERNEL_BIN)
	cat $(BOOT) $(KERNEL_BIN) > $(IMG)

$(KERNEL_BIN): $(KERNEL_SRC)
	nasm -f bin $(KERNEL_SRC) -o $(KERNEL_BIN)

run: $(IMG)
	qemu-system-x86_64 -drive format=raw,file=$(IMG)

clean:
	rm -f $(KERNEL_BIN) $(IMG)

.PHONY: all run clean

//terminal make file