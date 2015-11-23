/**
 *  Apache 2015/2/11
 *  Aersion 0.1
 *  Compile steps:
 *          1. Download Toolchain from http://downloads.openwrt.org/barrier_breaker/14.07/ar71xx/generic/OpenWrt-Toolchain-ar71xx-for-mips_34kc-gcc-4.8-linaro_uClibc-0.9.33.2.tar.bz2
 *          2. Extract it to your home
 *          3. Change OPENWRT_HOME in profile(at current dir) file if necessary
 *          4. Execute shell command: source ./profile
 *          5. make
 *  Usage:
 *          If compiled, you will get a sock2serial file, copy it your openwrt system.
 *          ./sock2serial  [OPTIONS] -s serial-port # maybe you will need to execute 'chmod +x ./sock2serial', enter "./sock2serial -h" for more information.
 *          e.g.
 *          ./sock2serial  -s /dev/ttyATH0 -b 115200 -p 2001
*/

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h> 
#include <getopt.h>
#include "serial.h"
#include "debug.h"

#define VERSION          "1.0beta"

#define TCP_PORT         2001
#define BACKLOG          1
#define BAUD             57600
#define BUFF_SIZE        512

#if defined(__unix__)
#define HL_PRINT(fmt, ...)    printf("\033[32m"fmt"\033[0m", ##__VA_ARGS__)
#else
#define HL_PRINT(fmt, ...)    printf(fmt, ##__VA_ARGS__)
#endif

typedef struct {
    int baud;
    int tcpport;
    char *serial;
}optargs_t;

static int intflag = 0;

static optargs_t optargs = { BAUD, TCP_PORT, NULL };
static char *optstr = "b:s:p:vh";
static struct option longopts[] = {
    {"baud",    required_argument, NULL, 'b'},
    {"serial",  required_argument, NULL, 's'},
    {"tcp-port",required_argument, NULL, 'p'},
    {"version", no_argument,       NULL, 'v'},
    {"help",    no_argument,       NULL, 'h'},
    {NULL,      no_argument,       NULL,  0 }
};

void show_version()
{
    printf("Sock2serial Version %s\n"
           "Copyright 1998-2015 Tencent Inc. All Rights Reserved.\n", VERSION );
    exit(1);
}

void show_usage()
{
    printf("Usage: sock2serial [OPTIONS] -s serial-port\n"
           "Sock2serial is a tool for serial and tcp data transmission\nIt's just like ser2net, but lighter.\n"
           "Misc:\n"
           "   -h, --help         print this usage message, then exit\n"
           "   -v, --version      print version number, then exit\n"
           "   -s, --serial       set serial port\n"
           "   -b, --baud         set serial baud(default as %d)\n"
           "   -p, --tcp-port     set tcp bind port(default as %d)\n\n", BAUD,  TCP_PORT);
           
    exit(1);
}

void on_sigint(int sig)
{
    intflag = 1;
}

int redirect_data(int srcfd, int dstfd, char *buff, size_t size)
{
    int n = read(srcfd, buff, size);
    if(n == -1 && errno == EAGAIN) {
        DEBUG_PRINT("read error");
        return 0;
    }
    return write(dstfd, buff, n);
}

void proc(int listenfd, int serialfd)
{
    char buff[BUFF_SIZE] = {0};
    int ret;
    int clientfd = -1;
    int maxfd = listenfd > serialfd ? listenfd : serialfd;
    struct timeval tv;
    fd_set fdset;
    
    serial_flush(serialfd);
    
    while(intflag == 0) {
        
        FD_ZERO(&fdset);
        
        if(clientfd > 0) {
            FD_SET(clientfd, &fdset);
            FD_SET(serialfd, &fdset);
        }
        FD_SET(listenfd, &fdset);
        
        tv.tv_sec = 10;
        tv.tv_usec = 0;
        
        ret = select(maxfd + 1, &fdset, NULL, NULL, &tv);
        if(ret < 0) {
            perror("select");
            break;
        } else if (ret == 0) {
            continue;
        }
        
        if(FD_ISSET(listenfd, &fdset)) {
            close(clientfd);
            clientfd = -1;
            serial_flush(serialfd);
            clientfd = accept(listenfd, NULL, NULL);
            DEBUG_PRINT("new client connected\n");
            fcntl(clientfd, F_SETFL, O_NONBLOCK);
            maxfd = clientfd;
            continue;
        }
        
        if(clientfd < 0) {
            continue;
        }
        
        if(FD_ISSET(clientfd, &fdset)) {
            ret = redirect_data(clientfd, serialfd, buff, BUFF_SIZE);
            if(ret < 1) {
                DEBUG_PRINT("socket closed\n");
                clientfd = -1;
                close(clientfd);
                continue;
            }
        }
        
        if(FD_ISSET(serialfd, &fdset)) {
            ret = redirect_data(serialfd, clientfd, buff, BUFF_SIZE);
            // TODO: test if ret > 0, mostly no check will be ok
        }
    }
}

int get_opts(int argc, char *argv[])
{
    int opt;
    
    while((opt = getopt_long(argc, argv, optstr, longopts, NULL)) != -1) {
    switch(opt) {
    case 'b':
        optargs.baud = atoi(optarg);
        break;
    case 'p':
        optargs.tcpport = atoi(optarg);
        break;
    case 's':
        optargs.serial = strdup(optarg);
        break;  
    case 'v':
        show_version();
        break;
    case 'h':
        show_usage();
        break;
    };
    }
    
    return optargs.serial != NULL ? 0 : -1;
}

int main(int argc, char **argv)
{
    int listenfd, clientfd;
    int serialfd;
    struct sockaddr_in servaddr;
    int reuse = 1;

    if(get_opts(argc, argv) != 0) {
        show_usage();
    }
    
    HL_PRINT("[use settings -> serial : %s, baud : %d, tcp-port : %d]\n",
           optargs.serial, optargs.baud, optargs.tcpport);
    
    signal(SIGHUP, SIG_IGN );
    if(signal(SIGINT, on_sigint) == SIG_ERR) {
        DEBUG_PRINT("fail to set signal handler");
        exit(1);
    }
    
    DEBUG_PRINT("opening serial...\n");
    serialfd = serial_init(optargs.serial, optargs.baud);
    if(serialfd < 0) {
        return -1;
    }
    fcntl(serialfd, F_SETFL, O_NONBLOCK);
    DEBUG_PRINT("serial open success!\n");
    
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(optargs.tcpport);
    
    if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        perror("bind");
        return 1;
    }
    
    if (listen(listenfd, BACKLOG) != 0) {
        perror("listen");
        return 1;
    }
    
    proc(listenfd, serialfd);
    
    close(listenfd);
    serial_close(serialfd);
    
    DEBUG_PRINT("program stopped\n");
    
    return 0;
}
