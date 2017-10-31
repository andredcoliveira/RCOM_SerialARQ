#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int main(int argc, char** argv) {

  fprintf(stderr, "\n\nsizeof(mode_t): %lu\n\n", sizeof(mode_t));

  //code
  T = 0x03; //Mode - Permissoes
  L = sizeof(detalhes.file_mode);
  V = (unsigned char *) malloc(L);
  sum = 0;
  for(i = 0; i < L; i++) {
    res = (detalhes.file_mode - sum) % (int)pow(256,3-i);
    V[i] = (detalhes.file_mode - res) / pow(256,3-i) - sum;
    sum += V[i] * pow(256,3-i);
  }


  //decode
  start.file_mode = pow(256,3) * (int)(properties_start[i].V[0]) +
                     pow(256,2) * (int)(properties_start[i].V[1]) +
                     pow(256,1) * (int)(properties_start[i].V[2]) +
                     pow(256,0) * (int)(properties_start[i].V[3]);
  fprintf(stderr, "start_file_flags = %d\n\n", start.file_mode);

  return 0;

}
