void kernel_main() {
    char* video = (char*) 0xB8000;
    
    int i;
    for (i = 0; i < 80 * 25 * 2; i += 2) {
        video[i] = ' ';
        video[i + 1] = 0x0F;
    }

    char* msg = "LouOS";
    i = 0;
    while (msg[i] != 0) {
        video[i * 2] = msg[i];
        video[i * 2 + 1] = 0x0F;
        i++;
    } 

    while(1);
}