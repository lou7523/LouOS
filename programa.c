void _start() {
    char* video = (char*) 0xB8000;

    char* msg = "Ola Mundo!";
    int i = 0;
    while (msg[i] != 0) {
        video[(80 + i) * 2] = msg[i];
        video[(80 + i) * 2 + 1] = 0x0A;
        i++;
    }

    while(1);
}