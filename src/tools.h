#ifndef _TOOLS_
#define _TOOLS_

#define BAUDRATE B115200  //38400
#define MODEMDEVICE "/dev/ttyS0"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define TX 1
#define RX 0

#define TAM_FRAME 	  1024	  			//size of frames
#define TAM_BUF       512    			  //size of buffer
#define DUPLICATE     -2      			//quando recebe uma trama duplicada, descarta
#define CALL_CLOSE  	-1 						//quando recebe DISC em llread
#define FER				   	 0			  		//Frame Error Rate em percentagem
#define NUM_TLV		  	 3
#define DIRECTORY      "/output/"
#define WAIT_TIME_RX   3
#define WAIT_TIME_TX   3

#define FR_F       		0x7E   	//0111 1110
#define FR_F_AUX 	  	0x5E  	//0111 1101 0101 1110

#define ESC           0x7D  	//0111 1101
#define ESC_AUX       0x5D  	//0111 1101 0101 1101

#define FR_A_TX       0x03    //0000 0011
#define FR_A_RX       0x01    //0000 0001

#define FR_C_SET      0x03    //0000 0011
#define FR_C_DISC     0x0B    //0000 1011
#define FR_C_UA       0x07    //0000 0111
#define FR_C_RR0      0x05    //0000 0101
#define FR_C_RR1      0x85    //1000 0101
#define FR_C_REJ0     0x01    //0000 0001
#define FR_C_REJ1     0x81    //1000 0001

#define FR_C_SEND0    0x00    //0000 0000
#define FR_C_SEND1    0x40    //0100 0000

#define FR_START	  	0x02		//0000 0010
#define FR_MID		  	0x01		//0000 0001
#define FR_END		  	0x03		//0000 0011

#define ERR_READ_TIMEOUT  -1
#define TRIES							 3

typedef struct {
	//time stamp variable (ver utime())
	unsigned char* file_name;
	int file_length;
	int file_flags;
	mode_t file_mode;
} details;

typedef struct {
	unsigned char T;
	unsigned char L;
	unsigned char *V;
} tlv;

typedef struct {
	unsigned char N;
	unsigned char L1;
	unsigned char L2;
	unsigned char *file_data;
	int size;
} data;

int fer_count;        //counts REJs
int count_frames;      //counts frames sent
int flag_alarm;
int timer_seconds;

void alarmHandler();
void randomError (unsigned char *buffer, int buffer_size);

void supervisionFrame(unsigned char* frame, unsigned char A, unsigned char C);
int constructFrame(unsigned char* frame, unsigned char* buffer, int length, int sendNumber);

int buildTLVPackage(unsigned char C, unsigned char* package, tlv* properties);
int buildDataPackage(unsigned char* buffer, unsigned char* package, int pack_size, int* seq_num);
void tlvPackage(unsigned char *packet, tlv *properties);
void dataPackage(unsigned char *packet, data *packet_data);

int setPort(char *port, struct termios *oldtio);
int changePort(int fd, int readMode);
int resetPort(int fd, struct termios *oldtio);

int getFileLength(int fd);
void progressBar(int done, int total);

#endif