void kernel_main() {
    char* video = (char*) 0xB8000;

    video[0] = 'L';
    video[1] = 0x0F;
    video[2] = 'o';
    video[3] = 0x0F;
    video[4] = 'u';
    video[5] = 0x0F;
    video[6] = 'O';
    video[7] = 0x0F;
    video[8] = 'S';

    while(1);
}