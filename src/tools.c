/** TOOLS **/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <math.h>
#include <signal.h>
#include <sys/time.h>
#include <inttypes.h>

#include "tools.h"

uint64_t nanos(struct timespec* ts) {

    return ts->tv_sec * (uint64_t)1000000000L + ts->tv_nsec;

}

void alarmHandler() {

	printf ("\nTimed out: %d seconds passed.\n\n", timer_seconds);
	flag_alarm = 1;

}

void randomError (unsigned char *buffer, int buffer_size) {

  int indice = 0;
	// struct timeval micros;
	int err = 0;
	// gettimeofday(&micros, NULL);
	// srand(micros.tv_usec);
	err = rand() % 101; //n de 0 a 100 que corresponde a percentagem de erro

	if (err < FER) {
		// gettimeofday(&micros, NULL);
		// srand(micros.tv_usec);
		indice = rand() % (buffer_size - 3) + 1;

		buffer[indice] = 0x00;
	}

	// float divi = 0;
	// fprintf(stderr, "\nerr: %d\n\n", err);
	// divi = (float) 100/FER;
	// if((count_frames % divi) == 0) {
	// 	indice = rand() % (buffer_size - 2) + 1; //de 1 a 8
	// 	//se p.e. buffer_size = 10 (pode mudar apenas de 1~8, todos menos as duas flags);
	//
	// 	fprintf(stderr, "\tbuffer[%d] a/e: %x\t - ", indice, buffer[indice]);
	// 	buffer[indice] = 0x00;
	// 	fprintf(stderr, "buffer[%d] d/e: %x\t - ", indice, buffer[indice]);
	// }

	// if(count_frames % 10 < FER/10) {
	// 		indice = rand() % (buffer_size - 2) + 1; //de 1 a 8
	// 		//se p.e. buffer_size = 10 (pode mudar apenas de 1~8, todos menos as duas flags);
	//
	// 		fprintf(stderr, "\tbuffer[%d] a/e: %x\t - ", indice, buffer[indice]);
	// 		buffer[indice] = 0x00;
	// 		fprintf(stderr, "buffer[%d] d/e: %x\t - ", indice, buffer[indice]);
	// }

} //randomError()

void supervisionFrame(unsigned char *sframe, unsigned char A, unsigned char C) {

	sframe[0] = FR_F;
	sframe[1] = A;
	sframe[2] = C;
	sframe[3] = sframe[1]^sframe[2];
	sframe[4] = FR_F;

} //supervisionFrame()

int constructFrame(unsigned char* frame, unsigned char* buffer, int length, int sendNumber) {

	int b = 0, i = 0;
	unsigned char Bcc2 = 0x00;

	frame[b++] = FR_F;  						//Flag inicial
	frame[b++] = FR_A_TX;  					//A

	if(sendNumber == 0) {   				//C
		frame[b++] = FR_C_SEND0;
	} else if(sendNumber == 1) {
		frame[b++] = FR_C_SEND1;
	}

	frame[b++] = frame[1]^frame[2]; //Bcc1
	for(i = 0; i < length; i++){    		//Byte stuffing dos dados
		Bcc2 = Bcc2 ^ buffer[i];   			//Bcc2 feito com XOR dos dados originais
		if(buffer[i] == FR_F) {
			frame[b++] = ESC;
			frame[b++] = FR_F_AUX;
		} else if(buffer[i] == ESC) {
			frame[b++] = ESC;
			frame[b++] = ESC_AUX;
		} else {
			frame[b++] = buffer[i];
		}
	}

	if(Bcc2 == FR_F) {  					 //Byte stuffing do Bcc2
		frame[b++] = ESC;
		frame[b++] = FR_F_AUX;
	} else if(Bcc2 == ESC) {
		frame[b++] = ESC;
		frame[b++] = ESC_AUX;
	} else {
		frame[b++] = Bcc2;  			  //Bcc2
	}

	frame[b] = FR_F;  						//Flag final

	return b+1;
}

int buildTLVPackage(unsigned char C, unsigned char* package, tlv* properties) {

	int i = 0, j = 0, b = 0;

	package[b++] = C;
	for(i = 0; i < NUM_TLV; i++) {
		package[b++] = properties[i].T;
		package[b++] = properties[i].L;
		for(j = 0; j < (int) properties[i].L; j++) {
			package[b++] = properties[i].V[j];
		}
	}

	return b;

} //buildTLVPackage()

int buildDataPackage(unsigned char* buffer, unsigned char* package, int pack_size, int* seq_num) {

	int i = 0, tmp = 0;
	package[0] = FR_MID;                         //C
	package[1] = (char) (*seq_num)++;            //N
	if((*seq_num) == 256) {
		*seq_num = 0;
	}

	tmp = pack_size % 256;
	package[2] = (pack_size - tmp) / 256;       //L2 - MSB
	package[3] = tmp;                           //L1 - LSB

	for(i = 0; i < pack_size; i++) {
		package[i+4] = buffer[i];         //Dados do ficheiro
	}

	return i+4;

} //buildDataPackage()

void tlvPackage(unsigned char *packet, tlv *properties) {

	int i = 0, j = 0, tam_V = 0, k = 0;
	while (k < NUM_TLV) {
		properties[k].T = packet[i];
		properties[k].L = packet[++i];
		tam_V = (int)(properties[k].L);
		properties[k].V = (unsigned char *) malloc(tam_V);
		for (j = 0; j < tam_V; j++){
			properties[k].V[j] = packet[++i];
		}
		i++;
		k++;
	}

}

void dataPackage(unsigned char *packet, data *packet_data) {

	int i = 0, j = 0;

	(*packet_data).N = packet[0]; //N
	(*packet_data).L2 = packet[1]; //L2
	(*packet_data).L1 = packet[2]; //L1
	(*packet_data).file_data = (unsigned char*) malloc(256*(int)packet[1] + (int)packet[2]);
	//P1, P2, ..., Pk
	for (j = 0, i = 3; j < 256*(int)(*packet_data).L2 + (int)(*packet_data).L1; j++, i++) {
		(*packet_data).file_data[j] = packet[i];
	}

}

int setPort(char *port, struct termios *oldtio) {

	if((strcmp("/dev/ttyS0", port) != 0) && (strcmp("/dev/ttyS1", port) != 0)) {
		perror("changePort(): wrong argument for port");
		return -1;
	}

	/*
		Open serial port device for reading and writing and not as controlling tty
		because we don't want to get killed if linenoise sends CTRL-C.
	*/
	struct termios newtio;

	int fd;
	if ((fd = open(port, O_RDWR | O_NOCTTY )) < 0) {
		perror(port);
		return -1;
	}

	if ( tcgetattr(fd, oldtio) == -1) { /* save current port settings */
		perror("tcgetattr");
		return -1;
	}

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	/* set input mode (non-canonical, no echo,...) */
	newtio.c_lflag = 0;

	newtio.c_cc[VTIME]    = 1;   /* inter-character timer unused (estava a 0)*/
	newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received (estava a 5)*/

	/*
		VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
		leitura do(s) proximo(s) caracter(es)
	*/

	tcflush(fd, TCIOFLUSH);

	if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
		perror("tcsetattr");
		return -1;
	}

	printf("\nNew termios structure set\n");

	return fd;

} //setPort()

int changePort(int fd, int readMode) {

 	if(readMode != 0 && readMode != 1) {
		perror("changePort(): wrong argument for readMode");
		return -1;
	}

	struct termios newtio;

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	/* set input mode (non-canonical, no echo,...) */
	newtio.c_lflag = 0;

	if(readMode == 0) {
		newtio.c_cc[VTIME]    = 1;   /* inter-character timer unused (estava a 0)*/
		newtio.c_cc[VMIN]     = 0;   /* read non-blocking */
	} else if(readMode == 1) {
		newtio.c_cc[VTIME]    = 1;   /* 3s max between chars */
		newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 chars received */
	}

	/*
		VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
		leitura do(s) proximo(s) caracter(es)
	*/

	tcflush(fd, TCIOFLUSH);

	if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
		perror("tcsetattr");
		return -1;
	}

	printf("\n\nStructure termios changed; readMode: %d\n",readMode);

	return 0;

} //changePort()

int resetPort(int fd, struct termios *oldtio) {

	if ( tcsetattr(fd, TCSANOW, oldtio) == -1) {  //volta a por a configuracao original
		perror("tcsetattr");
		return -1;
	}

	close(fd);

	return 0;

} //resetPort()

int getFileLength(int fd) {

	int length = 0;

	if((length = lseek(fd, 0, SEEK_END)) < 0) {
		perror("lseek()");
		return -1;
	}
	if(lseek(fd, 0, SEEK_SET) < 0) {
		perror("lseek()");
		return -1;
	}

	return length;

} //getFileLength()

void progressBar(int done, int total) {

	float fraction = 0.0;
	int j = 0;

	fraction = (float) done/total;
	fprintf(stderr, "\r\33[2KProgress: %2.2f%% - |", fraction*100);
	// fprintf(stderr, "\nProgress: %2.2f%% - |", fraction*100);
	for(j = 0; j < fraction * 28; j++) {
		fprintf(stderr, "|");
	}
	for(j = 0; j < 28 - (fraction*28); j++) {
		fprintf(stderr, " ");
	}
	fprintf(stderr, "| - sent/total: %d/%d", done, total);

	// fprintf(stderr, "\n\n");

} //progressBar()
