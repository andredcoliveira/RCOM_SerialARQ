#ifndef _DATA_LINK_
#define _DATA_LINK_

int getFrame(int port, unsigned char *frame, int MODE);
int llopen(int port, int MODE);
int llwrite(int port, unsigned char* buffer, int length);
int llread(int port, unsigned char* buffer);
int llclose(int port, int MODE);

#endif
