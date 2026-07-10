void _start() {
    char* video = (char*) 0xB8000;
    while(1) {
        video[77 * 2] = 'C';
        video[77 * 2 + 1] = 0x0F;
    } 
}