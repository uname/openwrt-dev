#ifndef SERIAL_H
#define SERIAL_H

int serial_init(const char* port, int baud);
int serial_flush(int fd);
int serial_close(int fd);

#endif