/** APPLICATION LAYER **/
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
#include "datalink.h"

int transmitter(int fd_port, char *source_path);
int receiver(int fd_port);

int transmitter(int fd_port, char *source_path) {

	unsigned char buffer[TAM_BUF], package[TAM_BUF];
	int source, res = 0, count_bytes = 0, count_bytes2 = 0, ler = 0;
	int clr = 0, state = 0, done = 0, seq_num = 0, i = 0, sum = 0;
	long sum_long = 0, res_long = 0;
	float divi = 0;
	uint64_t total_nanos_elapsed_inDataLink = 0;
	struct timespec starttime, stoptime;
	struct timespec init_api, finit_api;

	fer_count = 0;
	count_frames = 0;

	tlv properties[NUM_TLV];
	struct stat fileStat;
	details detalhes;

	if(clock_gettime(CLOCK_MONOTONIC, &init_api) < 0) {   //CLOCKING
		perror("clock_gettime()");                        //CLOCKING
		return -1;                                        //CLOCKING
	}                                                     //CLOCKING
	while(!done) {
		switch(state) {
			case 0:
				if(stat(source_path, &fileStat) < 0) {
					perror("stat()");
					return -1;
				}

				if((detalhes.file_name = (unsigned char *) malloc(strlen(source_path)+1)) == NULL) {
					perror("malloc");
					return -1;
				}
				strcpy((char *) detalhes.file_name, source_path);
				detalhes.file_flags = O_CREAT | O_APPEND | O_TRUNC | O_WRONLY;
				detalhes.file_mode = fileStat.st_mode;
				detalhes.file_time_a = fileStat.st_atim;
				detalhes.file_time_m = fileStat.st_mtim;

				source = open(source_path, O_RDONLY);

				fprintf(stderr, "TESTING llopen()...\n");
        if(clock_gettime(CLOCK_MONOTONIC, &starttime) < 0) {  //CLOCKING
          perror("clock_gettime()");                          //CLOCKING
          return -1;                                          //CLOCKING
        }                                                     //CLOCKING
        res = llopen(fd_port, TX);
        if(clock_gettime(CLOCK_MONOTONIC, &stoptime) < 0) {   //CLOCKING
          perror("clock_gettime()");                          //CLOCKING
          return -1;                                          //CLOCKING
        }                                                     //CLOCKING
        total_nanos_elapsed_inDataLink += nanos(&stoptime) - nanos(&starttime);  //CLOCKING
				if(res < 0) {
					perror("llopen()");
					return -1;
				}
				if(res == 0) {
					fprintf(stderr, "\n\t llopen() success.\n\n TESTING llwrite()...\n");
				}

				if((detalhes.file_length = getFileLength(source)) < 0) {
					perror("getFileLength()");
					return -1;
				}

			case 1: //start package
				properties[0].T = 0x00; //Tamanho
				properties[0].L = sizeof(detalhes.file_length);
				properties[0].V = (unsigned char *) malloc(properties[0].L);
				sum = 0;
				for(i = 0; i < properties[0].L; i++) {
					res = (detalhes.file_length - sum) % (int)pow(256,3-i);
					properties[0].V[i] = (detalhes.file_length - res) / pow(256,3-i) - sum;
					sum += properties[0].V[i] * pow(256,3-i);
				}

				properties[1].T = 0x01; //Nome
				properties[1].L = strlen((char *) detalhes.file_name) + 1;
				properties[1].V = (unsigned char *) malloc((int) properties[1].L);
				memcpy(properties[1].V, detalhes.file_name, (int) properties[1].L);

				properties[2].T = 0x02; //Flags
				properties[2].L = sizeof(detalhes.file_flags);
				properties[2].V = (unsigned char *) malloc((int) properties[2].L);
				sum = 0;
				for(i = 0; i < ((int) properties[2].L); i++) {
					res = (detalhes.file_flags - sum) % (int)pow(256,3-i);
					properties[2].V[i] = (detalhes.file_flags - res) / pow(256,3-i) - sum;
					sum += properties[2].V[i] * pow(256,3-i);
				}

				properties[3].T = 0x03; //Mode - Permissoes
				properties[3].L = sizeof(detalhes.file_mode);
				properties[3].V = (unsigned char *) malloc(properties[3].L);
				sum = 0;
				for(i = 0; i < properties[3].L; i++) {
					res = (detalhes.file_mode - sum) % (int)pow(256,3-i);
					properties[3].V[i] = (detalhes.file_mode - res) / pow(256,3-i) - sum;
					sum += properties[3].V[i] * pow(256,3-i);
				}

				properties[4].T = 0x04; //Data de ultimo acesso
				properties[4].L = sizeof(detalhes.file_time_a.tv_sec) + sizeof(detalhes.file_time_a.tv_nsec);
				properties[4].V = (unsigned char *) malloc(properties[4].L);
				sum_long = 0;
				for(i = 0; i < sizeof(detalhes.file_time_a.tv_sec); i++) {
				  res_long = (detalhes.file_time_a.tv_sec - sum_long) % (long)pow(256,sizeof(detalhes.file_time_a.tv_sec)-1-i);
				  properties[4].V[i] = (detalhes.file_time_a.tv_sec - res_long) / pow(256,sizeof(detalhes.file_time_a.tv_sec)-1-i) - sum_long;
				  sum_long += properties[4].V[i] * pow(256,sizeof(detalhes.file_time_a.tv_sec)-1-i);
				}
				sum_long = 0;
				for(i = sizeof(detalhes.file_time_a.tv_sec); i < properties[4].L; i++) {
				  res_long = (detalhes.file_time_a.tv_nsec - sum_long) % (long)pow(256,properties[4].L-1-i);
				  properties[4].V[i] = (detalhes.file_time_a.tv_nsec - res_long) / pow(256,properties[4].L-1-i) - sum_long;
				  sum_long += properties[4].V[i] * pow(256,properties[4].L-1-i);
				}

				properties[5].T = 0x05; //Data de modificacao
				properties[5].L = sizeof(detalhes.file_time_m.tv_sec) + sizeof(detalhes.file_time_m.tv_nsec);
				properties[5].V = (unsigned char *) malloc(properties[5].L);
				sum_long = 0;
				for(i = 0; i < sizeof(detalhes.file_time_m.tv_sec); i++) {
				  res_long = (detalhes.file_time_m.tv_sec - sum_long) % (long)pow(256,sizeof(detalhes.file_time_m.tv_sec)-1-i);
				  properties[5].V[i] = (detalhes.file_time_m.tv_sec - res_long) / pow(256,sizeof(detalhes.file_time_m.tv_sec)-1-i) - sum_long;
				  sum_long += properties[5].V[i] * pow(256,sizeof(detalhes.file_time_m.tv_sec)-1-i);
				}
				sum_long = 0;
				for(i = sizeof(detalhes.file_time_m.tv_sec); i < properties[5].L; i++) {
				  res_long = (detalhes.file_time_m.tv_nsec - sum_long) % (long)pow(256,properties[5].L-1-i);
				  properties[5].V[i] = (detalhes.file_time_m.tv_nsec - res_long) / pow(256,properties[5].L-1-i) - sum_long;
				  sum_long += properties[5].V[i] * pow(256,properties[5].L-1-i);
				}

				res = buildTLVPackage(FR_START, package, properties);
				if(res < 1){
					perror("buildTLVPackage");
					return -1;
				}

				if(clock_gettime(CLOCK_MONOTONIC, &starttime) < 0) {  //CLOCKING
				  perror("clock_gettime()");                          //CLOCKING
				  return -1;                                          //CLOCKING
				}                                                     //CLOCKING
				res = llwrite(fd_port, package, res);
				if(clock_gettime(CLOCK_MONOTONIC, &stoptime) < 0) {   //CLOCKING
				  perror("clock_gettime()");                          //CLOCKING
				  return -1;                                          //CLOCKING
				}                                                     //CLOCKING
				total_nanos_elapsed_inDataLink += nanos(&stoptime) - nanos(&starttime);  //CLOCKING
				if(res < 0) {
					perror("llwrite");
					return -1;
				}
				// count_frames++;
				state = 2;
				break;

			case 2: //data packages
				while(count_bytes2 < detalhes.file_length) {

					if(count_bytes2 + TAM_BUF-4 < detalhes.file_length) {
						ler = TAM_BUF-4;
						count_bytes2 += ler;
					} else {
						ler = detalhes.file_length - count_bytes2;
						count_bytes2 += ler;
					}

					if((res = read(source, buffer, ler)) < 0) {
						perror("read()");
						return -1;
					}

					if((res = buildDataPackage(buffer, package, res, &seq_num)) < 0) {
						perror("buildDataPackage");
						return -1;
					}

					if(clock_gettime(CLOCK_MONOTONIC, &starttime) < 0) {  //CLOCKING
						perror("clock_gettime()");                        //CLOCKING
						return -1;                                        //CLOCKING
					}                                                     //CLOCKING
					res = llwrite(fd_port, package, ler+4);
					if(clock_gettime(CLOCK_MONOTONIC, &stoptime) < 0) {   //CLOCKING
						perror("clock_gettime()");                        //CLOCKING
						return -1;                                        //CLOCKING
					}                                                     //CLOCKING
					total_nanos_elapsed_inDataLink += nanos(&stoptime) - nanos(&starttime);  //CLOCKING
					if(res < 0) {
						perror("llwrite()");
						return -1;
					} else {
						count_bytes += res;
					}

					for(clr = 0; clr < TAM_BUF; clr++) {
						buffer[clr] = 0;
					}
					// count_frames++;
					progressBar(count_bytes2, detalhes.file_length);
				}
				state = 3;
				break;

			case 3: //end package
				res = buildTLVPackage(FR_END, package, properties);
				if(res < 1){
					perror("buildTLVPackage");
					return -1;
				}

				if(clock_gettime(CLOCK_MONOTONIC, &starttime) < 0) {  //CLOCKING
				  perror("clock_gettime()");                          //CLOCKING
				  return -1;                                          //CLOCKING
				}                                                     //CLOCKING
				res = llwrite(fd_port, package, res);
				if(clock_gettime(CLOCK_MONOTONIC, &stoptime) < 0) {   //CLOCKING
				  perror("clock_gettime()");                          //CLOCKING
				  return -1;                                          //CLOCKING
				}                                                     //CLOCKING
				total_nanos_elapsed_inDataLink += nanos(&stoptime) - nanos(&starttime);  //CLOCKING
				if(res < 0) {
					perror("llwrite");
					return -1;
				}
				// count_frames++;
				state = 4;
				break;

			case 4:
				fprintf(stderr, "\n\n\t llwrite() success: bytes as frames: %d bytes; bytes as data: %d bytes",count_bytes, count_bytes2);

				//FRAME ERROR RATE
				divi = (float) fer_count/count_frames;
				fprintf(stderr, "\nFER: %2.4f%% - fer_count %d - count_frames %d\n\n", (float) divi*100, fer_count, count_frames);

				fprintf(stderr, "\n\n TESTING llclose()...\n");

				if(clock_gettime(CLOCK_MONOTONIC, &starttime) < 0) {  //CLOCKING
				  perror("clock_gettime()");                          //CLOCKING
				  return -1;                                          //CLOCKING
				}                                                     //CLOCKING
				res = llclose(fd_port, TX);
				if(clock_gettime(CLOCK_MONOTONIC, &stoptime) < 0) {   //CLOCKING
				  perror("clock_gettime()");                          //CLOCKING
				  return -1;                                          //CLOCKING
				}                                                     //CLOCKING
				total_nanos_elapsed_inDataLink += nanos(&stoptime) - nanos(&starttime);  //CLOCKING
				if(res < 0) {
					perror("llclose()");
					return -1;
				}
				if(res == 0) {
					fprintf(stderr, "\n\t llclose() success.\n\n");
				}

				fprintf(stderr, "\tTime spent in Data-Link Layer:  %" PRId64 "ns\n", total_nanos_elapsed_inDataLink); //CLOCKING

				if((close(source) < 0)) {
					perror("close()");
					return -1;
				}
				done = 1;
				break;

			default:
				break;
		}
	}
	if(clock_gettime(CLOCK_MONOTONIC, &finit_api) < 0) {  //CLOCKING
		perror("clock_gettime()");                        //CLOCKING
		return -1;                                        //CLOCKING
	}                                                     //CLOCKING

	fprintf(stderr, "\tTime spent (including API): \t%" PRId64 "ns\n", nanos(&finit_api) - nanos(&init_api)); //CLOCKING
	fprintf(stderr, "\tDifference: \t\t\t%" PRId64 "ns\n\n", (nanos(&finit_api) - nanos(&init_api)) - total_nanos_elapsed_inDataLink);  //CLOCKING

	fprintf(stderr, "\n\tcount_bytes_S = %d\n", count_bytes_S);

	return 0;

} //transmitter()

int receiver(int fd_port) {

	unsigned char buffer[TAM_BUF];
	unsigned char packet[TAM_BUF-1];   //nao contem o campo C
	int output = 0, res = 0, done = 0, state = 0, i = 0, j = 0, change = 0, count_bytes = 0, seq_N = 255; //valor maximo do N no mod 256
	tlv properties_start[NUM_TLV], properties_end[NUM_TLV];
	data packet_data;
	details start, end;
	struct timespec file_time_obtido[2];
	uint64_t total_nanos_elapsed_inDataLink = 0;
	struct timespec starttime, stoptime;
	struct timespec init_api, finit_api;

	if(clock_gettime(CLOCK_MONOTONIC, &init_api) < 0) {   //CLOCKING
		perror("clock_gettime()");                        //CLOCKING
		return -1;                                        //CLOCKING
	}                                                     //CLOCKING

	fprintf(stderr, "TESTING llopen()...\n");

	if(clock_gettime(CLOCK_MONOTONIC, &starttime) < 0) {  //CLOCKING
		perror("clock_gettime()");                        //CLOCKING
		return -1;                                        //CLOCKING
	}                                                     //CLOCKING
	res = llopen(fd_port, RX);
	if(clock_gettime(CLOCK_MONOTONIC, &stoptime) < 0) {   //CLOCKING
		perror("clock_gettime()");                        //CLOCKING
		return -1;                                        //CLOCKING
	}                                                     //CLOCKING
	total_nanos_elapsed_inDataLink += nanos(&stoptime) - nanos(&starttime);  //CLOCKING
	if(res < 0) {
		perror("llopen()");
		return -1;
	}
	if(res == 0) {
		fprintf(stderr, "\n\t llopen() success.\n\n TESTING llread()...\n");
	}

	while (!done) {
		switch (state) {
			case 0: //start package
				if (!change) {
					if(clock_gettime(CLOCK_MONOTONIC, &starttime) < 0) {  //CLOCKING
						perror("clock_gettime()");                        //CLOCKING
						return -1;                                        //CLOCKING
					}                                                     //CLOCKING
					res = llread(fd_port, buffer);
					if(clock_gettime(CLOCK_MONOTONIC, &stoptime) < 0) {   //CLOCKING
						perror("clock_gettime()");                        //CLOCKING
						return -1;                                        //CLOCKING
					}                                                     //CLOCKING
					total_nanos_elapsed_inDataLink += nanos(&stoptime) - nanos(&starttime);  //CLOCKING
					if (res == CALL_CLOSE) { //Leu o DISC - Fim da leitura
						done = 1;
						break;
					} else if (res < 0) {
						perror("llread()");
						return -1;
					}
					change = 1;
				}

				if (change) {
					//Leitura dos pacotes do tipo START, DATA e END
					for (i = 0; i < res-1; i++) {
						packet[i] = buffer[i+1];
					}
					if (buffer[0] == FR_START){
						tlvPackage(packet, properties_start);
						for (i = 0; i < NUM_TLV; i++){
							switch (properties_start[i].T) {
								case 0x00: //Tamanho
									start.file_length = pow(256,3) * (int)(properties_start[i].V[0]) +
																			pow(256,2) * (int)(properties_start[i].V[1]) +
																			pow(256,1) * (int)(properties_start[i].V[2]) +
																			pow(256,0) * (int)(properties_start[i].V[3]);
									fprintf(stderr, "\nstart.file_length: %d bytes\n", start.file_length);
									break;

								case 0x01: //Nome
									start.file_name = (unsigned char *) malloc((int)properties_start[i].L);
									strcpy((char *) start.file_name, (char *)properties_start[i].V);
									fprintf(stderr, "start.file_name: %s\n", start.file_name);
									break;

								case 0x02: //Flags
									start.file_flags = pow(256,3) * (int)(properties_start[i].V[0]) +
																	 	 pow(256,2) * (int)(properties_start[i].V[1]) +
																	 	 pow(256,1) * (int)(properties_start[i].V[2]) +
																	 	 pow(256,0) * (int)(properties_start[i].V[3]);
									fprintf(stderr, "start.file_flags: %d\n", start.file_flags);
									break;

								case 0x03: //Mode - Permissoes
									start.file_mode = pow(256,3) * (int)(properties_start[i].V[0]) +
													pow(256,2) * (int)(properties_start[i].V[1]) +
													pow(256,1) * (int)(properties_start[i].V[2]) +
													pow(256,0) * (int)(properties_start[i].V[3]);
									fprintf(stderr, "start.file_mode: ");
									fprintf(stderr, (S_ISDIR(start.file_mode)) ? "d" : "-");
									fprintf(stderr, (start.file_mode & S_IRUSR) ? "r" : "-");
									fprintf(stderr, (start.file_mode & S_IWUSR) ? "w" : "-");
									fprintf(stderr, (start.file_mode & S_IXUSR) ? "x" : "-");
									fprintf(stderr, (start.file_mode & S_IRGRP) ? "r" : "-");
									fprintf(stderr, (start.file_mode & S_IWGRP) ? "w" : "-");
									fprintf(stderr, (start.file_mode & S_IXGRP) ? "x" : "-");
									fprintf(stderr, (start.file_mode & S_IROTH) ? "r" : "-");
									fprintf(stderr, (start.file_mode & S_IWOTH) ? "w" : "-");
									fprintf(stderr, (start.file_mode & S_IXOTH) ? "x" : "-");
									fprintf(stderr,"\n\n");
									break;

								case 0x04: //Time - Last access date
									start.file_time_a.tv_sec = 0;
									start.file_time_a.tv_nsec = 0;
									for(j = 0; j < 8; j++) {
										start.file_time_a.tv_sec += pow(256,7-j) * (long)properties_start[i].V[j];
										start.file_time_a.tv_nsec += pow(256,7-j) * (long)properties_start[i].V[j+8];
									}
									break;

								case 0x05: //Time - Last Modification date
									start.file_time_m.tv_sec = 0;
									start.file_time_m.tv_nsec = 0;
									for(j = 0; j < 8; j++) {
										start.file_time_m.tv_sec += pow(256,7-j) * (long)properties_start[i].V[j];
										start.file_time_m.tv_nsec += pow(256,7-j) * (long)properties_start[i].V[j+8];
									}
									break;

								default:
									break;

							}
						}
					} else if (buffer[0] == FR_MID) {
						change = 1;
						state = 1;
						break;
					} else {
						fprintf(stderr, "\n\nERROR: Wrong C from package\n");
						return -1;
					}
					change = 0;
				}

				if((output = open((char *)start.file_name, start.file_flags, start.file_mode)) < 0) {
					perror("open");
					return -1;
				}
				//0 - Last access date
				file_time_obtido[0] = start.file_time_a;
				//1 - Last modification date
				file_time_obtido[1] = start.file_time_m;
				break;

			case 1: //data package
				if (!change) {
					if(clock_gettime(CLOCK_MONOTONIC, &starttime) < 0) {  //CLOCKING
						perror("clock_gettime()");                        //CLOCKING
						return -1;                                        //CLOCKING
					}                                                     //CLOCKING
					res = llread(fd_port, buffer);
					if(clock_gettime(CLOCK_MONOTONIC, &stoptime) < 0) {   //CLOCKING
						perror("clock_gettime()");                        //CLOCKING
						return -1;                                        //CLOCKING
					}                                                     //CLOCKING
					total_nanos_elapsed_inDataLink += nanos(&stoptime) - nanos(&starttime);  //CLOCKING
					if (res == CALL_CLOSE) { //Leu o DISC - Fim da leitura
						done = 1;
						break;
					} else if (res == DUPLICATE) {
						break; //se receber frame repetida no llread retorna DUPLICATE; ignora a leitura do duplicado; volta a ler a proxima
					} else if (res < 0) {
						perror("llread()");
						return -1;
					}
					change = 1;
				}

				if (change) {
					//Leitura dos pacotes do tipo START, DATA e END
					for (i = 0; i < res-1; i++) {
						packet[i] = buffer[i+1];
					}
					if (buffer[0] == FR_MID) {
						/** Verifica se o tamanho do buffer excede
						o tamanho suportado pelo pacote
						(L2 e L1 - 2 bytes - 65535 bytes de dados) **/
						if ((res-4) != (256*(int)packet[1] + (int)packet[2])) {
							fprintf(stderr,"ERRO: Described packet size and actual "
                                    "packet size differ. Writing max data packet size "
                                    "(65535 bytes) from port to output.");
						}
						dataPackage(packet, &packet_data);
					} else if (buffer[0] == FR_END) {
						change = 1;
						state = 2;
						break;
					}

					if (((int)packet_data.N - seq_N == 1) || (seq_N - (int)packet_data.N == 255)) {
						seq_N = (int)packet_data.N;
						if (seq_N == 256) {
							seq_N = 0;
						}
					} else {
						fprintf(stderr, "\n\nERRO: seq_N = %d  |  packet.N = %d\n", seq_N, (int)packet_data.N);
                        return -1;
					}

					if ((res = write(output, packet_data.file_data, 256*(int)packet_data.L2 + (int)packet_data.L1)) < 0) {
						perror("write()");
						return -1;
					}

					count_bytes += res;

					//Cleaning the buffer
					for (i = 0; i < TAM_BUF; i++) {
						buffer[i] = 0;
					}
					change = 0;

					progressBar(count_bytes, start.file_length);
				}
				break;


			case 2: //end package
				if (!change) {
					if(clock_gettime(CLOCK_MONOTONIC, &starttime) < 0) {  //CLOCKING
						perror("clock_gettime()");                        //CLOCKING
						return -1;                                        //CLOCKING
					}                                                     //CLOCKING
					res = llread(fd_port, buffer);
					if(clock_gettime(CLOCK_MONOTONIC, &stoptime) < 0) {   //CLOCKING
						perror("clock_gettime()");                        //CLOCKING
						return -1;                                        //CLOCKING
					}                                                     //CLOCKING
					total_nanos_elapsed_inDataLink += nanos(&stoptime) - nanos(&starttime);  //CLOCKING
					if (res == CALL_CLOSE) { //Leu o DISC - Fim da leitura
						done = 1;
						break;
					} else if (res < 0) {
						perror("llread()");
						return -1;
					}
					change = 1;
				}

				if (change) {
					//Leitura dos pacotes do tipo START, DATA e END
					for (i = 0; i < res-1; i++) {
						packet[i] = buffer[i+1];
					}
					if (buffer[0] == FR_END){
						tlvPackage(packet, properties_end);
					}
					for (i = 0; i < NUM_TLV; i++) {
						switch (properties_end[i].T) {
							case 0x00: //Tamanho
								end.file_length = pow(256,3) * (int)(properties_end[i].V[0]) +
																	pow(256,2) * (int)(properties_end[i].V[1]) +
																	pow(256,1) * (int)(properties_end[i].V[2]) +
																	pow(256,0) * (int)(properties_end[i].V[3]);
								fprintf(stderr, "\n\nend.file_length: %d\n", end.file_length);
								break;

							case 0x01: //Nome
								end.file_name = (unsigned char *) malloc((int)properties_end[i].L);
								strcpy((char *) end.file_name, (char *)properties_end[i].V);
								fprintf(stderr, "end.file_name: %s\n", end.file_name);
								break;

							case 0x02: //Flags
								end.file_flags = pow(256,3) * (int)(properties_end[i].V[0]) +
																 pow(256,2) * (int)(properties_end[i].V[1]) +
																 pow(256,1) * (int)(properties_end[i].V[2]) +
																 pow(256,0) * (int)(properties_end[i].V[3]);
								fprintf(stderr, "end.file_flags: %d\n", end.file_flags);
								break;

							case 0x03: //Mode - Permissoes
								end.file_mode = pow(256,3) * (int)(properties_end[i].V[0]) +
											  pow(256,2) * (int)(properties_end[i].V[1]) +
											  pow(256,1) * (int)(properties_end[i].V[2]) +
											  pow(256,0) * (int)(properties_end[i].V[3]);
								fprintf(stderr, "end.file_mode: ");
								fprintf(stderr, (S_ISDIR(end.file_mode)) ? "d" : "-");
								fprintf(stderr, (end.file_mode & S_IRUSR) ? "r" : "-");
								fprintf(stderr, (end.file_mode & S_IWUSR) ? "w" : "-");
								fprintf(stderr, (end.file_mode & S_IXUSR) ? "x" : "-");
								fprintf(stderr, (end.file_mode & S_IRGRP) ? "r" : "-");
								fprintf(stderr, (end.file_mode & S_IWGRP) ? "w" : "-");
								fprintf(stderr, (end.file_mode & S_IXGRP) ? "x" : "-");
								fprintf(stderr, (end.file_mode & S_IROTH) ? "r" : "-");
								fprintf(stderr, (end.file_mode & S_IWOTH) ? "w" : "-");
								fprintf(stderr, (end.file_mode & S_IXOTH) ? "x" : "-");
								fprintf(stderr, "\n");
							break;

							case 0x04: //Time - Last access date
								end.file_time_a.tv_sec = 0;
								end.file_time_a.tv_nsec = 0;
								for(j = 0; j < 8; j++) {
									end.file_time_a.tv_sec += pow(256,7-j) * (long)properties_end[i].V[j];
									end.file_time_a.tv_nsec += pow(256,7-j) * (long)properties_end[i].V[j+8];
								}
							break;

							case 0x05: //Time - Last Modification date
								end.file_time_m.tv_sec = 0;
								end.file_time_m.tv_nsec = 0;
								for(j = 0; j < 8; j++) {
									end.file_time_m.tv_sec += pow(256,7-j) * (long)properties_end[i].V[j];
									end.file_time_m.tv_nsec += pow(256,7-j) * (long)properties_end[i].V[j+8];
								}
								break;

							default:
								break;
						}
					}
					change = 0;
				}
				break;
		}
	}

	fprintf(stderr, "\n\n\t llread() success: bytes written to output: %d bytes;", count_bytes);

	if (count_bytes != start.file_length && count_bytes != end.file_length) {
		fprintf(stderr, "\n\nERROR: The size of the file in START_PACKAGE (%d) and in END_PACKAGE (%d) is different\n\n", start.file_length, end.file_length);
	}

	if((end.file_time_a.tv_sec != file_time_obtido[0].tv_sec)
      || (end.file_time_a.tv_nsec != file_time_obtido[0].tv_nsec)
      || (end.file_time_m.tv_sec != file_time_obtido[1].tv_sec)
      || (end.file_time_m.tv_nsec != file_time_obtido[1].tv_nsec)) {
		fprintf(stderr, "\n\nERROR: Dates in initial package don't match the ones in end package\n\n");
	}
	if(futimens(output, file_time_obtido) < 0) {
		perror("futimens");
		return -1;
	}

	fprintf(stderr, "\n\n TESTING llclose()...\n");

	if(clock_gettime(CLOCK_MONOTONIC, &starttime) < 0) {  //CLOCKING
		perror("clock_gettime()");                        //CLOCKING
		return -1;                                        //CLOCKING
	}                                                     //CLOCKING
	res = llclose(fd_port, RX);
	if(clock_gettime(CLOCK_MONOTONIC, &stoptime) < 0) {   //CLOCKING
		perror("clock_gettime()");                        //CLOCKING
		return -1;                                        //CLOCKING
	}                                                     //CLOCKING
	total_nanos_elapsed_inDataLink += nanos(&stoptime) - nanos(&starttime);  //CLOCKING
	if(res < 0) {
		perror("llclose()");
		return -1;
	}
	if(res == 0) {
		fprintf(stderr, "\n\t llclose() success.\n\n");
	}

	fprintf(stderr, "\tTime spent in Data-Link Layer:  %" PRId64 "ns\n", total_nanos_elapsed_inDataLink); //CLOCKING

	if(close(output) < 0) {
		perror("close()");
		return -1;
	}

	if(clock_gettime(CLOCK_MONOTONIC, &finit_api) < 0) {  //CLOCKING
		perror("clock_gettime()");                        //CLOCKING
		return -1;                                        //CLOCKING
	}                                                     //CLOCKING

	fprintf(stderr, "\tTime spent (including API): \t%" PRId64 "ns\n", nanos(&finit_api) - nanos(&init_api)); //CLOCKING
	fprintf(stderr, "\tDifference: \t\t\t%" PRId64 "ns\n\n", (nanos(&finit_api) - nanos(&init_api)) - total_nanos_elapsed_inDataLink);  //CLOCKING

	fprintf(stderr, "\n\tcount_bytes_S = %d\n", count_bytes_S);

	return 0;

} //receiver()

int main(int argc, char** argv) {

	int fd_port;
	struct termios oldtio;
	char buf[100];

	if (argc < 2) {
		printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
		exit(-1);
	}

	if((fd_port = setPort(argv[1], &oldtio)) < 0) {
		perror("setPort()");
		exit(-1);
	}

	fprintf(stderr, "SHALL WE BEGIN?... RX / TX?\n\n");
	if(fgets(buf, sizeof(buf), stdin) == 0){
		perror("fgets");
		exit(-1);
	}
	if((strncmp(buf, "TX", 2) == 0) || (strncmp(buf, "tx", 2) == 0)){
		if(transmitter(fd_port, argv[2]) < 0) {
			perror("transmitter()");
			exit(-1);
		}
	} else if((strncmp(buf, "RX", 2) == 0) || (strncmp(buf, "rx", 2) == 0)) {
		if(receiver(fd_port) < 0) {
			perror("receiver()");
			exit(-1);
		}
	} else {
		perror("Bad input: Use 'RX' or 'TX' as an argument");
		exit(-1);
	}

	sleep(1); //para o set de default nao alterar durante a escrita

	//set original port configurations
	if(resetPort(fd_port, &oldtio) < 0) {
		perror("resetPort()");
		exit(-1);
	}

	return 0;

} //main()
