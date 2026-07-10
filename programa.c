void _start() {
    __asm__ volatile ("mov $1, %eax; int $0x80");
    while(1);   
}