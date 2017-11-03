#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <math.h>
#include <unistd.h>


int main(int argc, char** argv) {

  fprintf(stderr, "\n7D^0=%x", 0x7D^0);
  fprintf(stderr, "\n7D^5D=%x", 0x7D^0x5D);
  fprintf(stderr, "\n7E^0=%x", 0x7E^0);
  fprintf(stderr, "\n7E^5E=%x\n", 0x7E^0x5E);


  // fprintf(stderr, "\n\nsizeof(struct timespec): %lu\n\n", sizeof(struct timespec));
  // fprintf(stderr, "\n\nsizeof(time_t): %lu\n\n", sizeof(time_t));
  // fprintf(stderr, "\n\nsizeof(long): %lu\n\n", sizeof(long));
  //
  // long sum = 0, res = 0;
  // int i = 0;
  // unsigned char L0, L1;
  // unsigned char *V0, *V1;
  //
  // int fdteste;
  //
  // struct stat fileStat;
  // struct timespec file_time_original[2];
  // struct timespec file_time_obtido[2];
  //
  // if(argc < 2) {
  //   perror("nr of arguments");
  //   exit(-1);
  // }
  //
  // if(stat(argv[1], &fileStat) < 0) {
  //   perror("stat()");
  //   return -1;
  // }
  //
  // file_time_original[0] = fileStat.st_atim;
  // file_time_original[1] = fileStat.st_mtim;
  //
  //
  // L0 = sizeof(file_time_original[0].tv_sec) + sizeof(file_time_original[0].tv_nsec);
  // V0 = (unsigned char *) malloc(L0);
  // sum = 0;
  // for(i = 0; i < sizeof(file_time_original[0].tv_sec); i++) {
  //   res = (file_time_original[0].tv_sec - sum) % (long)pow(256,sizeof(file_time_original[0].tv_sec)-1-i);
  //   V0[i] = (file_time_original[0].tv_sec - res) / pow(256,sizeof(file_time_original[0].tv_sec)-1-i) - sum;
  //   sum += V0[i] * pow(256,sizeof(file_time_original[0].tv_sec)-1-i);
  //   // fprintf(stderr, "\nres: %ld\tV0[%d]: %x\tsum: %ld", res, i, V0[i], sum); //DEBUG
  // }
  // sum = 0;
  // for(i = sizeof(file_time_original[0].tv_sec); i < L0; i++) {
  //   res = (file_time_original[0].tv_nsec - sum) % (long)pow(256,L0-1-i);
  //   V0[i] = (file_time_original[0].tv_nsec - res) / pow(256,L0-1-i) - sum;
  //   sum += V0[i] * pow(256,L0-1-i);
  // }
  //
  //
  // L1 = sizeof(file_time_original[1].tv_sec) + sizeof(file_time_original[1].tv_nsec);
  // V1 = (unsigned char *) malloc(L1);
  // sum = 0;
  // for(i = 0; i < sizeof(file_time_original[1].tv_sec); i++) {
  //   res = (file_time_original[1].tv_sec - sum) % (long)pow(256,sizeof(file_time_original[1].tv_sec)-1-i);
  //   V1[i] = (file_time_original[1].tv_sec - res) / pow(256,sizeof(file_time_original[1].tv_sec)-1-i) - sum;
  //   sum += V1[i] * pow(256,sizeof(file_time_original[1].tv_sec)-1-i);
  // }
  // sum = 0;
  // for(i = sizeof(file_time_original[1].tv_sec); i < L1; i++) {
  //   res = (file_time_original[1].tv_nsec - sum) % (long)pow(256,L1-1-i);
  //   V1[i] = (file_time_original[1].tv_nsec - res) / pow(256,L1-1-i) - sum;
  //   sum += V1[i] * pow(256,L1-1-i);
  // }
  //
  //
  // file_time_obtido[0].tv_sec = 0;
  // file_time_obtido[0].tv_nsec = 0;
  // file_time_obtido[1].tv_sec = 0;
  // file_time_obtido[1].tv_nsec = 0;
  // for(i = 0; i < 8; i++) {
  //   file_time_obtido[0].tv_sec += pow(256,7-i) * (long)V0[i];
  //   file_time_obtido[0].tv_nsec += pow(256,7-i) * (long)V0[i+8];
  //   fprintf(stderr, "\n\n%d: pow(256,7-%d): %ld\t(long)V0[%d]: %x\t(long)V0[%d+8]: %x",
  //           i, i, (long)pow(256,7-i), i, V0[i], i, V0[i+8]); //DEBUG
  //   file_time_obtido[1].tv_sec += pow(256,7-i) * (long)V1[i];
  //   file_time_obtido[1].tv_nsec += pow(256,7-i) * (long)V1[i+8];
  //   fprintf(stderr, "\n%d: pow(256,7-%d): %ld\t(long)V1[%d]: %x\t(long)V1[%d+8]: %x",
  //           i, i, (long)pow(256,7-i), i, V1[i], i, V1[i+8]); //DEBUG
  //   fprintf(stderr, "\n\n%d: file_time_obtido[0].tv_sec: %ld", i, file_time_obtido[0].tv_sec);  //DEBUG
  //   fprintf(stderr, "\n%d: file_time_obtido[0].tv_nsec: %ld", i, file_time_obtido[0].tv_nsec);  //DEBUG
  //   fprintf(stderr, "\n%d: file_time_obtido[1].tv_sec: %ld", i, file_time_obtido[1].tv_sec);  //DEBUG
  //   fprintf(stderr, "\n%d: file_time_obtido[1].tv_nsec: %ld", i, file_time_obtido[1].tv_nsec);  //DEBUG
  // }
  //
  //
  // if((fdteste = open("test_file.jpg", O_CREAT | O_APPEND | O_TRUNC | O_WRONLY, fileStat.st_mode)) < 0) {
  //   perror("open");
  //   exit(-1);
  // }
  //
  // if(futimens(fdteste, file_time_obtido) < 0) {
  //   perror("futimens");
  //   exit(-1);
  // }
  // /*** DEBUG INIT ***/
  // fprintf(stderr, "\n\ndetalhes.file_time_a.tv_sec: %ld", file_time_obtido[0].tv_sec);
  // fprintf(stderr, "\ndetalhes.file_time_a.tv_nsec: %ld", file_time_obtido[0].tv_nsec);
  // fprintf(stderr, "\ndetalhes.file_time_m.tv_sec: %ld", file_time_obtido[1].tv_sec);
  // fprintf(stderr, "\ndetalhes.file_time_m.tv_nsec: %ld\n", file_time_obtido[1].tv_nsec);
  // /*** DEBUG FINIT ***/
  //
  // close(fdteste);

  return 0;

}
