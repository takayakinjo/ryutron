#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <pthread.h>
//ADDED
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <sys/ioctl.h>

#define DEV_NAME "/dev/ttyACM0"
#define BAUD_RATE B9600
#define BUFF_SIZE 4096

int fd;

int ok = 0;

void *func_thread(void *p) {

  int len;
  unsigned char buffer[BUFF_SIZE];

  //printf("start %d\n", *(int*)p);

  while(1) {
#ifdef __linux__
    while (0 >= (len = read(fd, buffer, BUFF_SIZE))) {
      usleep(100);
    }; // waiting for inputs
#elif __APPLE__
    while (0 == (len = read(fd, buffer, BUFF_SIZE))) {
      usleep(100);
    }; // waiting for inputs
    if(len < 0) { // I/O error
      printf("I/O error\n");
      exit(2);
    }
#else
    printf("platform unknown. abort.\n");
    exit(1);
#endif
    // check ok
    if (!strncmp((const char*)buffer, "o", 1)) {
      ok = 1;
    }
    for(int i = 0; i < len; i++) {
      printf("%c", buffer[i]); // disp data
    }
  }
  return 0;
}

int kbhit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}

void serial_init(int fd) {
  struct termios tio;
  memset(&tio, 0, sizeof(tio));
  tio.c_cflag = CS8 | CLOCAL | CREAD;
  tio.c_cc[VTIME] = 10; //DELETED
  //tio.c_cc[VMIN] = 1;
  // set baud rate
  cfsetispeed(&tio, BAUD_RATE);
  cfsetospeed(&tio, BAUD_RATE);

  //cfmakeraw(&tio);                    // ADDED

  // device confuguration
  tcsetattr(fd, TCSANOW, &tio);

  //ioctl(fd, TCSETS, &tio);            // ADDED
}

void send_command( const char *com ) {

  ok = 0;
  int length = strlen(com);
  char cr = 0x0d;

  write(fd, com, length);
  write(fd, &cr, 1);

  usleep(2000);

  while (ok == 0) {
    usleep(200);
  }
  usleep(8000);
  //printf("OK received\n");
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

  send_command("R28");
  //  send_command("R1 TW-7");

#if 0
  while(1) {
    send_command("R1 YL0 XL-3 XR-3 t2000");
    send_command("R1 YR0 ZR50 t500");
    send_command("R1 YR0 ZR0 t500");
    send_command("R1 YR0 XL3 XR3 t2000");
    send_command("R1 YL0 ZL50 t500");
    send_command("R1 YL0 ZL0 t500");
  }
  
  while(1) {
    send_command("R1 XL2 XR2 YL-10 YR10 ZR0 t500");
    send_command("R1 XL4 XR4 YL0 YR0 ZL50 t400");
    send_command("R1 XL-2 XR-2 YL10 YR-10 ZL0 t500");
    send_command("R1 XL-4 XR-4 YL0 YR0 ZR50 t400");
  }
  while(1) {
    send_command("R1 YL-20 YR20 ZR0 t500");
    send_command("R1 XL3 XR3 YL0 YR0 ZL60 t500");
    send_command("R1 YL20 YR-20 ZL0 t500");
    send_command("R1 XL-3 XR-3 YL0 YR0 ZR60 t500");
  }
#endif
  while(1) {
    send_command("R1 XL2 XR2 t1000");
    send_command("R1 XL5 XR5 ZL40 t1000");
    send_command("R1 XL2 XR2 ZL0 YL0 YR0 t1000");
    //send_command("R1 YL0 YR-20 t1000");
    send_command("R1 XL-1 XR-1 t1000");
    send_command("R1 XL-3 XR-3 ZR40 t1000");
    send_command("R1 XL2 XR2 ZR0 YL0 YR0 t1000");
    //send_command("R1 YL-20 YR0 t1000");
  }
  while(1) {
    send_command("R1 YL0 XL-4 XR-4 t1500");
    send_command("R1 YR0 ZR60 t1500");
    send_command("R1 XL0 XR0 ZR0 YL-10 YR20 t1500");
    send_command("R1 XL2 XR2 t1500");
    //send_command("R1 YR0");
    send_command("R1 YR0 XL4 XR4 t1500");
    send_command("R1 YL0 ZL60 t1500");
    send_command("R1 XL0 XR0 ZL0 YR-10 YL20 t1500");
    send_command("R1 XL-1 XR-1 t1500");
    //send_command("R1 YL0");
  }

  // main loop
  while(1) {
    if (kbhit()) {
      char c = getchar();
      write(fd, &c, 1);
    }
  }
}
