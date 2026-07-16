extern void desenharTexto(int x, int y, char* texto, unsigned char r, unsigned char g, unsigned char b, int escala);
extern void desenharJanelas(int x, int y, int largura, int altura, unsigned char r, unsigned char g, unsigned char b);
extern void preencherEcra(unsigned char r, unsigned char g, unsigned char b);
extern void desenharPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b);
extern void desenharChar(int x, int y, char c, unsigned char r, unsigned char g, unsigned char b, int escala);
extern void intParaString(int numero, char* buffer);
extern void desenharMenu();
extern void lerSector(unsigned int lba, unsigned char* destino);
extern int contadorTicks;
extern int estadoSistema;
extern unsigned int bitmap[];

struct registos {
    unsigned int edi;
    unsigned int esi;
    unsigned int ebp;
    unsigned int esp;
    unsigned int ebx;
    unsigned int edx;
    unsigned int ecx;
    unsigned int eax;
    unsigned int eip;
    unsigned int cs;
    unsigned int eflags;
} __attribute__ ((packed));
struct processos {
        unsigned int estado; //0 = bloqueia  1 = permite
        unsigned int enderecoProcesso; //Para saber onde esta cada endereco na stack
        unsigned int cr3; //0x07
        struct registos registos; //adiciona a struct registos dentro de processos
    };

extern struct processos processos[4];

struct FAT32BootSector {
        unsigned char jump[3];              //Os primeiros 3 bytes do sector sao uma instrucao x86 (JMP + NOP), 
                                            //este salta por cima dos dados do bootsector para o codigo de arranque
                                            //Como ja tenho bootloader nao vou usar isto (em principio)
        unsigned char oemName[8];           //Nome do sistema que formatou o disco
        unsigned short bytesPerSector;      //Quase sempre 512. Este define o tamanho de cada sector fisico do disco
                                            //usa-se este valor para calcular onde estao as estruturas do filesystems
        unsigned char sectorsPerCluster;    //Normalmente este e 8 (4KB p/ cluster) Um ficheiro de 1 byte ocupa um cluster inteiro
        unsigned short reservedSectors;     //Quantos sectores antes do primeiro ficheiro FAT
        unsigned char numFAT;               //(2) Há sempre um ficheiro FAT principal e a sua copia de seguranca
                                            //se o principal estiver corrompido o OS pode usar a copia de seguranca
        unsigned short rootEntryCount;      //Em FAT32 e sempre 0. Em FAT12 e 16 estes tem um valor fixo
        unsigned short totalSectors16;      //Em FAT32 este é sempre 0. Era usado em FAT12/FAT16 para discos pequenos (16-bits)
        unsigned char mediaType;            //0xF8 para o disco rigido fixo, 0xF0 para disquetes. Nao vai ser usado
        unsigned short sectoresPerFAT16;    //Em FAT32 e sempre 0, mas este era usado em FAT12/16, nao vamos usar aqui
        unsigned short sectoresPerTrack;    //Sectores por trilha, geometria fisica do disco (CHS), nao e importante pois estamos a usar LBA
        unsigned short numHeads;            //Tambem nao é relevante pois usamos LBA
        unsigned int hiddenSectors;         //Sectores ocultos quantos sectores existem antes da particao do disco
        unsigned int totalSectors32;        //Numero total de sectores do volume. Usa-se para saber o tamanho total do disco
        unsigned int sectorsPerFAT32;      //Quantos sectores ocupa cada copia de FAT 
                                            //inicio da area de dados = reservedSectors + (numFATs * sectorsPerFAT32)   
        unsigned short extFlags;            //Controla qual FAT esta ativa (main / copy)
        unsigned short fsVersion;           //Versao do Filesystem (0x0000) normalmente 0.0
        unsigned int rootCluster;           //Cluster do root directory, este e normalmente 2, e aqui que comeca a lista de ficheiros e pastas da raiz
        unsigned short fsInfo;              //Normalmente e 1. O fsInfo é uma estrutura que guarda o numero de clusters livres para nao ter de contar sempre 0
        unsigned short backupBootSector;    //Sector de backup do bootsector, normalmente 6, copia do boot sector para a recuperacao em caso de corrupcao
        unsigned char reserved[12];         //Bytes a 0 para o futuro
        unsigned char driveNumber;          //0x80 para hard drive, 0x00 para disquetes. Legado da BIOS (ignorar)
        unsigned char reserved2;            //Sempre 0 (ignorar)
        unsigned char bootSignature;        //0x29 indica o volumeID, volumeLabel, fsType
        unsigned int volumeID;              //Numero aleatorio gerado quando o disco foi formatado. Usado para identificar o disco.
        unsigned char volumeLabel[11];      //O nome que se da ao disco
        unsigned char fsType[8];            //Usado para detectar campos anteriores
        /*
            Disto tudo só se vai usar:
                -bytesPerSector 
                -sectorPerCluster
                -reservedSectors
                -numFATs
                -sectorsPerFAT32
                -rootCluster

            Tudo o resto e legado, cosmetico ou para compatibilidade
        */
    } __attribute__((packed));

struct FAT32DirEntry {
        unsigned char nome[8];          //Nome do ficheiro
        unsigned char ext[3];           //extensao (.txt .pdf .jpg)
        unsigned char atributos;        //tipo de entrada (ficheiro, pasta)
        unsigned char ignorados1[8];    //campos nao usados
        unsigned short clusterAlto;     //16 bits superiores do cluster
        unsigned char ignorado2[4];     //campos nao usados
        unsigned short clusterBaixo;    //16 bits inferiores do cluster
        unsigned int tamanho;           //tamanho do ficheiro em bytes
        //Cluster - É um grupo de entradas de sectores consecutivos no disco, é a unidade minima de alocacao de ficheiros FAT32

    } __attribute__ ((packed));

void iniciarTerminal();
void terminalHandleChar(char c);
void clear();
int compararStrings();
int comecaCom();
int chegouAoFimDaJanela();
int igualA(char* comando);
void version();
void uptime();
void mostrarMemory();
void quit();
void echo();
void ps();
void reboot();
void ls();
void errorMessage();