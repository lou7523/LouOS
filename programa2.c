void _start() {
    char* video = (char*) 0xB8000;
    while(1) {
        video[78 * 2] = 'D';
        video[78 * 2 + 1] = 0x0F;
    } 
}