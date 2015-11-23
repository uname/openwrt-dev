#include <stdio.h> 
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <string.h>
#include <sys/ioctl.h>

#include "serial.h"

int serial_init(const char* serialport, int baud)
{
    struct termios toptions;
    int fd;
    
    if(serialport == NULL) {
        perror("serialport is null");
        return -1;
    }
    
    fd = open(serialport, O_RDWR | O_NONBLOCK );
    
    if (fd == -1)  {
        perror("serial_init: Unable to open port ");
        return -1;
    }
    
    if (tcgetattr(fd, &toptions) < 0) {
        perror("serial_init: Couldn't get term attributes");
        return -1;
    }

    speed_t brate = baud;

    switch(baud) {
    case 4800:   brate = B4800;   break;
    case 9600:   brate = B9600;   break;
#ifdef B14400
    case 14400:  brate = B14400;  break;
#endif
    case 19200:  brate = B19200;  break;
#ifdef B28800
    case 28800:  brate = B28800;  break;
#endif
    case 38400:  brate = B38400;  break;
    case 57600:  brate = B57600;  break;
    case 115200: brate = B115200; break;
    
    default:
        perror("baud not supported");
        return -1;
    }
    
    cfsetispeed(&toptions, brate);
    cfsetospeed(&toptions, brate);

    // 8N1
    toptions.c_cflag &= ~PARENB;
    toptions.c_cflag &= ~CSTOPB;
    toptions.c_cflag &= ~CSIZE;
    toptions.c_cflag |= CS8;

    toptions.c_cflag &= ~CRTSCTS; // no flow control

    toptions.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
    toptions.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl

    toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
    toptions.c_oflag &= ~OPOST; // make raw

    toptions.c_cc[VMIN]  = 0;
    toptions.c_cc[VTIME] = 0;
    
    tcsetattr(fd, TCSANOW, &toptions);
    if( tcsetattr(fd, TCSAFLUSH, &toptions) < 0) {
        perror("init_serial: Couldn't set term attributes");
        return -1;
    }

    return fd;
}

int serial_flush(int fd)
{
    int ret;
    usleep(10000); //required to make flush work, for some reason
    ret = tcflush(fd, TCIOFLUSH);
    usleep(1000000); //to make it work
    return ret;
}

int serial_close(int fd)
{
    return close(fd);
}
