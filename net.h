
#if IBM
#include "winsock2.h"
#else
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#if IBM
#define GET_ERRNO WSAGetLastError()
#else
#define GET_ERRNO errno
#endif


#if IBM
extern SOCKET sockfd;
#else
extern int sockfd;
#endif
extern struct sockaddr_in recv_sockaddr, dest_sockaddr[];
//extern struct sockaddr    orig_sockaddr;

// socket stuff
void setAddresses();

int openSocket();

int bindSocket();

void closeSocket();

int pollReceive();

void resetSocket();
