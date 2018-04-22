#ifndef DEF_H_

#if defined(WIN32)
#include <Winsock2.h>
typedef int socklen_t;
#define MSG_NOSIGNAL 0
#define MSG_DONTWAIT 0
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
#include <cstdlib>
#include <iostream>

#define CONNECT_BUF_SIZE (1000)

#define I_REQUEST 0
#define I_RESPONSE 1
#define I_DISCONNECT 2
#define I_RESPONSE_DISCONNECT 3

const char PRIMITIVE[][100] = {"REQUEST", "RESPONSE", "DISCONNECT", "RESPONSE_DISCONNECT"};

int init_socket();
int process_request(char*);
int process_response(char*);
int process_disconnect(char*);


#endif  // !DEF_H_
