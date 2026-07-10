void _start() {
    char* video = (char*) 0xB8000;
    while(1) {
        video[79 * 2] = 'B';
        video[79 * 2 + 1] = 0x0F;
    } 
}