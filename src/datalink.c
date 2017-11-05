/** DATA LINK LAYER **/
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

#include "tools.h"
#include "datalink.h"

/* GLOBAL VAR */
int readNumber = 0;  //Numero da trama a ler
int sendNumber = 0;     //Numero da trama a escrever

//cool function to read stuff from port
int getFrame(int port, unsigned char *frame, int MODE) {

	int done = 0, res = 0, b = 0;
	unsigned char get;

	timer_seconds = WAIT_TIME;

	fd_set readfds;
	struct timeval tv;

	//make sure fd set is cleared
	FD_ZERO(&readfds);
	//add port to set
	FD_SET(port, &readfds);
	//set timeout value
	tv.tv_sec = timer_seconds; //s
	tv.tv_usec = 0;  //us

	memset(frame, 0, TAM_FRAME);  //cleans frame before a read


	while(!done) {
		/*** get byte ***/
		if(read(port, &get, 1) == 0) {
			res = select(port + 1, &readfds, NULL, NULL, &tv);
			if(res == -1) {
				perror("select()"); //some error occured in select
				return -2;
			} else if(res == 0) {
				memset(frame, 0, TAM_FRAME);  //cleans buffer if timeout
				tcflush(port, TCIOFLUSH);
				return ERR_READ_TIMEOUT;
			} else if(FD_ISSET(port, &readfds)) {
				//port has data
				read(port, &get, 1);
			}
		}

		/*** by this stage function has returned err or get has 1 byte ***/
		if(get == FR_F) {
			if(b == 0) {
				frame[b++] = get;
			} else {
				if(frame[b-1] == FR_F) {
					memset(frame, 0, TAM_FRAME);
					b = 0;
					frame[b++] = FR_F;
				} else {
					frame[b++] = get;
					done = 1;
				}
			}
		} else {
			if(b > 0) {
				frame[b++] = get;
			}
		}
	}

	return b;  //returns frame length

} //getFrame()

int llopen(int port, int MODE) {

	int state = 0, res = 0, done = 0, bad = 0, got = 0;
	unsigned char SET[5], UA[5];
	unsigned char frame_got[TAM_FRAME];

	//SET
	supervisionFrame(SET, FR_A_TX, FR_C_SET);

	//UA
	supervisionFrame(UA, FR_A_TX, FR_C_UA);

	if(MODE == TX) {
		while (!done) {
			switch (state) {
				case 0: //Envia o SET
					// fprintf(stderr, "\nSending SET...\n");
					tcflush(port, TCIOFLUSH);  //clear port
					if((res = write(port, SET, 5)) < 0) {   //0 ou 5?
						perror("write()");
						return -1;
					}
					state = 1;
					break;

				case 1:  //get UA
					got = getFrame(port, frame_got, TX);
					if(got == ERR_READ_TIMEOUT) {
						if(bad < TRIES) {
							fprintf(stderr, "\n\nTime-out: nothing from port after %d seconds...\n\n\n", timer_seconds);
							bad++;
							state = 0;
						} else {
							fprintf(stderr, "\n\nNothing from RX after %d tries. Giving up...\n", TRIES);
							return -1;
						}
					} else {
						// count_bytes_S+=got; //calc eficiencia
						if(memcmp(frame_got, UA, 5) == 0) {
							fprintf(stderr, "\nConnection successfully established.\n");
							done = 1;
						} else {
							state = 0;
						}
					}
					break;

				default:
					break;
			}
		}

	} else if(MODE == RX) {
		while(!done) {
			switch(state) {
				case 0: //get SET
					got = getFrame(port, frame_got, TX);
					if(got == ERR_READ_TIMEOUT) {
						if(bad < TRIES) {
							fprintf(stderr, "\n\nTime-out: nothing from port after %d seconds...\n\n\n", timer_seconds);
							bad++;
							state = 0;
						} else {
							fprintf(stderr, "\n\nNothing from TX after %d tries. Giving up...\n", TRIES);
							return -1;
						}
					} else {
						// count_bytes_S+=got; //calc eficiencia
						if(memcmp(frame_got, SET, 5) == 0) {
							state = 1;
						}
					}
					break;

				case 1:  //send UA
					// fprintf(stderr, "\nSending UA...\n");
					tcflush(port, TCIOFLUSH);  //clear port
					if((res = write(port, UA, 5)) < 0) {   //0 ou 5?
						perror("write()");
						return -1;
					}
					fprintf(stderr, "\nConnection successfully established.\n");
					done = 1;
					break;

				default:
					break;
			}
		}
	}

	return 0;

} //llopen()

int llread(int port, unsigned char *buffer) {

  int state = 0, bad = 0, i = 0, j = 0, done = 0, got = 0;
	unsigned char RR[5], REJ[5], RR_LOST[5], DISC[5];
	unsigned char frame_got[TAM_FRAME];
	unsigned char BCC2 = 0x00;

	//DISC
	supervisionFrame(DISC, FR_A_TX, FR_C_DISC);

	while(!done) {
		switch(state) {
			case 0:  //reads frame
				got = getFrame(port, frame_got, RX);
				if(got == ERR_READ_TIMEOUT) {
					if(bad < TRIES) { //daft punk - one more time
						fprintf(stderr, "\n\nTime-out: nothing from port after %d seconds...\n\n\n", timer_seconds);
						bad++;
						state = 6;
					} else {
						fprintf(stderr, "\n\nNothing from TX after %d tries. Giving up...\n", TRIES);
						return -3;
					}
				} else {
					if (frame_got[2] != FR_C_SET && frame_got[2] != FR_C_UA && frame_got[2] != FR_C_DISC) {
						randomError(frame_got, got);
					}
					count_bytes_S+=got; //calc eficiencia
					count_frames++; //calc eficiencia
					state = 1;
				}
				break;

			case 1: //check Bcc1 first
				if(frame_got[3] != (frame_got[1]^frame_got[2])) {  //Bcc1 != A^C
					state = 6; //REJ
					break;
				}
				state = 2;
				break;

			case 2: //check if llread() got a DISC
				if(memcmp(frame_got, DISC, 5) == 0) {
					return CALL_CLOSE;
				} else {
					state = 3;
				}
				break;

			case 3: //destuffing
				for (i = 4; i < got - 1; i++) {
					if((frame_got[i] == ESC) && ((frame_got[i+1] == FR_F_AUX) || (frame_got[i+1] == ESC_AUX))) {
						if(frame_got[i+1] == FR_F_AUX) {
							frame_got[i] = FR_F;
						} else if(frame_got[i+1] == ESC_AUX) {
							frame_got[i] = ESC;
						}
						for (j = i + 1; j < got - 1; j++){
							frame_got[j] = frame_got[j+1];
						}
						got--;
					}
				}
				state = 4;
				break;

			case 4: //check Bcc2
				//Comparacao de BCC2 com o XOR do pacote (D1^D2^...^Dn)
				BCC2 = frame_got[4];
				for (i = 5; i < got - 2; i++) {
					BCC2 = BCC2 ^ frame_got[i];
				}
				if (BCC2 != frame_got[got-2]) {
					state = 6; //REJ
					break;
				}
				state = 5;
				break;

			case 5:  //RR
				if(frame_got[2] == FR_C_SEND0 && readNumber == 0) {
					readNumber = 1;
					supervisionFrame(RR, FR_A_TX, FR_C_RR1);
				} else if (frame_got[2] == FR_C_SEND1 && readNumber == 1) {
					readNumber = 0;
					supervisionFrame(RR, FR_A_TX, FR_C_RR0);
				} else {
					state = 7;  //Entra no RR LOST
					break;
				}

				//Transferencia do pacote de dados da frame para o buffer da API
				for (i = 4, j = 0; i < got - 2; i++, j++) {
					buffer[j] = frame_got[i];
				}
				//SEND RR
				tcflush(port, TCIOFLUSH); //clear port
				if (write(port, RR, 5) < 0) {
					perror("RR write");
					return -3;
				}
				done = 1;
				break;

			case 6: //REJ
				if (got == ERR_READ_TIMEOUT) {
					if(readNumber == 0) {
						supervisionFrame(REJ, FR_A_TX, FR_C_REJ0);
					} else if (readNumber == 1) {
						supervisionFrame(REJ, FR_A_TX, FR_C_REJ1);
					}
				} else {
					if(frame_got[2] == FR_C_SEND0 && readNumber == 0) {
						supervisionFrame(REJ, FR_A_TX, FR_C_REJ0);
					} else if (frame_got[2] == FR_C_SEND1 && readNumber == 1) {
						supervisionFrame(REJ, FR_A_TX, FR_C_REJ1);
					}
				}

				//send REJ
				tcflush(port, TCIOFLUSH); //clear port
				if (write(port, REJ, 5) < 0) {
					perror("REJ write");
					return -3;
				}
				// memset(frame_got, 0, TAM_FRAME);  //cleans buffer if REJ sent.
				state = 0;  //go back and listen the port again
				break;

			case 7: //DUPLICATE
				if (frame_got[2] == FR_C_SEND0 && readNumber == 1) {
						supervisionFrame(RR_LOST, FR_A_TX, FR_C_RR1);
				} else if (frame_got[2] == FR_C_SEND1 && readNumber == 0) {
						supervisionFrame(RR_LOST, FR_A_TX, FR_C_RR0);
				}

				tcflush(port, TCIOFLUSH); //clear port
				if (write(port, RR_LOST, 5) < 0) {
					perror("llwrite(): RR_LOST");
					return -3;
				}
				memset(frame_got, 0, TAM_FRAME);  //cleans buffer if RR is lost.
				return DUPLICATE;

			default:
				break;
		}
	}

	return (got - 6);  //frame length without headers

} //llread()

int llwrite(int port, unsigned char* buffer, int length) {

	int state = 0, res = 0, bad = 0, done = 0, sent = 0, frame_len = 0;
	unsigned char RR[5], REJ[5];
	unsigned char frame_sent[TAM_FRAME];
	unsigned char frame_got[TAM_FRAME];

	//construct RR & REJ
	if(sendNumber == 0) {
		supervisionFrame(RR, FR_A_TX, FR_C_RR1);
		supervisionFrame(REJ, FR_A_TX, FR_C_REJ0);
	} else if(sendNumber == 1) {
		supervisionFrame(RR, FR_A_TX, FR_C_RR0);
		supervisionFrame(REJ, FR_A_TX, FR_C_REJ1);
	} else {
		return -1;
	}

	frame_len = constructFrame(frame_sent, buffer, length, sendNumber);

	while(!done) {
		switch(state) {
			case 0: //writes frame on port
				sent = write(port, frame_sent, frame_len);
				state = 1;
				break;

			case 1: //listens port for ACK or NACK
				res = getFrame(port, frame_got, TX);
				if(res == ERR_READ_TIMEOUT) {
					if(bad < TRIES) {  //let's give another try
						fprintf(stderr, "\n\nTime-out: nothing from port after %d seconds...\n\n\n", timer_seconds);
						bad++;
						state = 0;
					} else {   //you're fresh out of luck, pal
						fprintf(stderr, "\n\nNo answer from RX after %d tries. Giving up...\n", TRIES);
						return -1;
					}
				} else {
					count_bytes_S+=res;  //calc eficiencia
					count_frames++;
					state = 2;
				}
				break;

			case 2: //checks if RR or REJ
				if(memcmp(RR, frame_got, 5) == 0) { //congrats! it's an RR
					sendNumber = 1 - sendNumber;
					done = 1;
				} else if(memcmp(REJ, frame_got, 5) == 0){   //you still have a shot at this, bud
					fer_count++;
					bad = 0;
					state = 0;
				} else {   //what is this, a frame for ants?
					bad++;
					state = 0;
				}
				break;

			default:
				break;
		}
	}

	return sent;

} //llwrite()

int llclose(int port, int MODE) {

	int state = 0, res = 0, bad = 0, done = 0, got = 0;
	unsigned char DISC[5], UA[5];
	unsigned char frame_got[TAM_FRAME];

	//DISC
	supervisionFrame(DISC, FR_A_TX, FR_C_DISC);

	//UA
	supervisionFrame(UA, FR_A_TX, FR_C_UA);

	if (MODE == RX) {
		while (!done) {
			switch (state) {
				case 0: //Envia o DISC
					// fprintf(stderr, "\nSending DISC...\n");
					tcflush(port, TCIOFLUSH);  //clear port
					if((res = write(port, DISC, 5)) < 0) {   //0 ou 5?
						perror("write()");
						return -1;
					}
					state = 1;
					break;

				case 1:  //get UA
					got = getFrame(port, frame_got, RX);
					if(got == ERR_READ_TIMEOUT) {
						if(bad < TRIES) {
							fprintf(stderr, "\n\nTime-out: nothing from port after %d seconds...\n\n\n", timer_seconds);
							bad++;
							state = 0;
						} else {
							fprintf(stderr, "\n\nNothing from RX after %d tries. Giving up...\n", TRIES);
							return -1;
						}
					} else {
						// count_bytes_S+=got;  //calc eficiencia
						if(memcmp(frame_got, UA, 5) == 0) {
							fprintf(stderr, "\nConnection successfully terminated.\n");
							done = 1;
						} else {
							state = 0;
						}
					}
					break;

				default:
					break;
			}
		}
		return 0;

	} else if (MODE == TX) {
		while (!done) {
			switch (state) {
				case 0: //Envia o DISC
					// fprintf(stderr, "\nSending DISC...\n");
					tcflush(port, TCIOFLUSH);  //clear port
					if((res = write(port, DISC, 5)) < 0) {   //0 ou 5?
						perror("write()");
						return -1;
					}
					state = 1;
					break;

				case 1:  //get DISC
					got = getFrame(port, frame_got, TX);
					if(got == ERR_READ_TIMEOUT) {
						if(bad < TRIES) {
							fprintf(stderr, "\n\nTime-out: nothing from port after %d seconds...\n\n\n", timer_seconds);
							bad++;
							state = 0;
						} else {
							fprintf(stderr, "\n\nNothing from RX after %d tries. Giving up...\n", TRIES);
							return -1;
						}
					} else {
						// count_bytes_S+=got; //calc eficiencia
						if(memcmp(frame_got, DISC, 5) == 0) {
							state = 2;
						} else {
							state = 0;
						}
					}
					break;

				case 2: //DISC received, send UA
					tcflush(port, TCIOFLUSH);  //clear port
					// fprintf(stderr, "\nSending UA\n");
					if((res = write(port, UA, 5)) < 0) {  //0 ou 5?
						perror("write()");
						return -1;
					}
					fprintf(stderr, "\nConnection successfully terminated.\n");
					done = 1;
					break;

				default:
					break;
			}
		}
		return 0;
	}

	return -1;

} //llclose()
