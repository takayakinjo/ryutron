#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <pthread.h>


#define DEV_NAME "/dev/tty.usbmodem14433"
#define BAUD_RATE B9600
#define BUFF_SIZE 4096

int fd;

void *func_thread(void *p) {

  int len;
  unsigned char buffer[BUFF_SIZE];

  //printf("start %d\n", *(int*)p);

  while(1) {
    while (0 == (len = read(fd, buffer, BUFF_SIZE))) {}; // waiting for inputs
    if(len < 0) { // I/O error
      printf("I/O error\n");
      exit(2);
    }
    for(int i = 0; i < len; i++) {
      printf("%c", buffer[i]); // disp data
    }
  }
  return 0;
}

void serial_init(int fd) {
  struct termios tio;
  memset(&tio, 0, sizeof(tio));
  tio.c_cflag = CS8 | CLOCAL | CREAD;
  //tio.c_cflag = CS8 | CREAD;
  tio.c_cc[VTIME] = 10;
  //tio.c_cc[VMIN] = 1;
  // set baud rate
  cfsetispeed(&tio, BAUD_RATE);
  cfsetospeed(&tio, BAUD_RATE);
  // device confuguration
  tcsetattr(fd, TCSANOW, &tio);
}

int main(int argc, char **argv) {

  char inbuff[256];
  
  printf("Ryutron v10\n");

  // open the port
  if (0 > (fd = open(DEV_NAME, O_RDWR | O_NONBLOCK ))) {
    printf("Devide open failed\n");
    exit(1);
  }

  serial_init(fd); // init the port

  int b = 42;
  pthread_t pthread;
  pthread_create( &pthread, NULL, &func_thread, &b);

    // main loop
  while(1) {
    char c = getchar();
    write(fd, &c, 1);
    //sleep(1);
  }

}
