#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <math.h>


int main(int argc, char** argv) {

  fprintf(stderr, "\n\nsizeof(mode_t): %lu\n\n", sizeof(mode_t));

  int sum = 0, res = 0, i = 0;
  unsigned char L;
  unsigned char* V;

  struct stat fileStat;
  mode_t file_mode_original;
  mode_t file_mode_obtido;

  if(argc < 2) {
    perror("nr of arguments");
    exit(-1);
  }

  if(stat(argv[1], &fileStat) < 0) {
    perror("stat()");
    return -1;
  }

  file_mode_original = fileStat.st_mode;

  //code
  L = sizeof(file_mode_original);
  V = (unsigned char *) malloc(L);
  sum = 0;
  for(i = 0; i < L; i++) {
    res = (file_mode_original - sum) % (int)pow(256,3-i);
    V[i] = (file_mode_original - res) / pow(256,3-i) - sum;
    sum += V[i] * pow(256,3-i);
  }
  fprintf(stderr, "file_mode_original = %d\n\n", file_mode_original);


  //decode
  file_mode_obtido = pow(256,3) * (int)(V[0]) +
                     pow(256,2) * (int)(V[1]) +
                     pow(256,1) * (int)(V[2]) +
                     pow(256,0) * (int)(V[3]);
  fprintf(stderr, "file_mode_obtido = %d\n\n", file_mode_obtido);

  open("test_file", O_CREAT | O_APPEND | O_TRUNC | O_WRONLY, file_mode_obtido);

  return 0;

}
