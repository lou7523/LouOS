# LouOS

LouOS is a 32-bit operating system built from scratch in C and x86 Assembly, developed as a PAP project. It runs in protected mode, supports graphical output via a VESA framebuffer, includes a PS/2 keyboard driver, a memory manager, multitasking, and a terminal with built-in commands.

---

## Requirements

Install the following dependencies (Ubuntu/Debian):

```bash
sudo apt install nasm gcc-multilib binutils qemu-system-x86 mtools
```

---

## How to Run

```bash
make run
```

This will compile the kernel, create a FAT32 disk image, and launch it in QEMU.

---

## Terminal Commands

| Command       | Description                              |
|---------------|------------------------------------------|
| `clear`       | Clears the terminal screen               |
| `version`     | Shows the current version of LouOS       |
| `uptime`      | Shows how long the OS has been running   |
| `mem`         | Shows available memory in MB             |
| `quit`        | Exits the terminal and returns to menu   |
| `echo <msg>`  | Displays a message in a popup            |

---

## Project Structure

- `kernel.asm` — Bootloader, GDT, IDT handlers in Assembly
- `kernel.c` — Kernel core: memory, paging, processes, drivers
- `terminal.c` — Terminal logic and commands
- `terminal.h` — Terminal header file
- `fonte.h` — 8x8 bitmap font for graphical text rendering
- `link.ld` — Linker script

---

## Features

- Protected mode (32-bit x86)
- VESA framebuffer graphics (800x600)
- PS/2 keyboard driver
- Physical memory manager (bitmap-based)
- Paging (virtual memory)
- Round-robin multitasking
- FAT32 filesystem (read)
- ELF program loader
- Ring 3 user mode
- Built-in terminal with commands