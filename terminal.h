extern void desenharTexto(int x, int y, char* texto, unsigned char r, unsigned char g, unsigned char b, int escala);
extern void desenharJanelas(int x, int y, int largura, int altura, unsigned char r, unsigned char g, unsigned char b);
extern void preencherEcra(unsigned char r, unsigned char g, unsigned char b);
extern void desenharPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b);
extern void desenharChar(int x, int y, char c, unsigned char r, unsigned char g, unsigned char b, int escala);
extern void intParaString(int numero, char* buffer);
extern int contadorTicks;
extern unsigned int bitmap[];

void iniciarTerminal();
void terminalHandleChar(char c);
void clear();
int compararStrings();
int chegouAoFimDaJanela();
int igualA(char* comando);
void version();
void uptime();
void mostrarMemory();
void errorMessage();