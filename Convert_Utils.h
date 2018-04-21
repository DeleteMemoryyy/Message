#ifndef CONVERT_UTILS_H_

#if defined(WIN32)
#include <Winsock2.h>
typedef int socklen_t;
#define MSG_NOSIGNAL 0
#elif defined(__linux__)
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
typedef int SOCKET;
typedef unsigned char BYTE;
typedef unsigned long DWORD;
typedef sockaddr SOCKADDR;
#define SOCKADDR_IN sockaddr_in
#define FALSE 0
#define SOCKET_ERROR (-1)
#endif

#include <cstdio>
#include <cstring>
#include <iostream>

int init_socket();

#endif  // !CONVERT_UTILS_H_
