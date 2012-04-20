
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if IBM
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif


#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"
#include "XPLMNavigation.h"
#include "XPLMDisplay.h"
#include "XPLMMenus.h"
#include "XPWidgets.h"
#include "XPStandardWidgets.h"


#include "globals.h"
#include "xdata.h"
#include "settings.h"
//#include "structs.h"
//#include "packets.h"


#if IBM
#define GET_ERRNO WSAGetLastError()
#else
#define GET_ERRNO errno
#endif



// define global vars
#if IBM
SOCKET sockfd;
#else
int sockfd;
#endif
struct sockaddr_in recv_sockaddr, dest_sockaddr[NUM_DEST];
//struct sockaddr    orig_sockaddr;

// define local vars
int xdata_socket_open;


#if IBM
int startWinsock() {
	WSADATA wsa;
	WORD version;
	int rc;
	char debug_message[256];

	//version = (2<<8)+1;
	version = MAKEWORD(2, 2);

	rc = WSAStartup(version, &wsa);

	if ( rc ) {
		sprintf(debug_message, "XData failed: could not start winsock! errno: %d\n", WSAGetLastError());
		XPLMDebugString(debug_message);
		return 0;
	} else {
		XPLMDebugString("XData: winsock started\n");
		return 1;
	}
}
#endif


int openSocket() {

	char debug_message[256];

	#if IBM
	if ( startWinsock() == 0 ) {
	    XPLMDebugString("XData: start winsock failed\n");
		return 0;
	}
	#endif

	// create socket
	//sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	sockfd = socket(PF_INET, SOCK_DGRAM, 0);

    #if IBM
	if ( sockfd == INVALID_SOCKET ) {
    #else
	if ( sockfd < 0 ) {
    #endif
		sprintf(debug_message, "XData: failed - could not open socket! errno: %d\n", GET_ERRNO);
		XPLMDebugString(debug_message);
		return 0;
	} else {
		XPLMDebugString("XData: socket created\n");
	}

    return 1;

}


void setAddresses() {

	char addr_cleartext[200]; // 1.0 Beta 8 : [100] was too small...
	int i;

	XPLMDebugString("XData: setting addresses\n");

    // receiver address
	recv_sockaddr.sin_family = AF_INET;
	recv_sockaddr.sin_port = htons(recv_port);
	recv_sockaddr.sin_addr.s_addr = INADDR_ANY;
	memset(&(recv_sockaddr.sin_zero), '\0', 8);

	sprintf(addr_cleartext, "XData: recv : Port=%d\n", recv_port);
	XPLMDebugString(addr_cleartext);
	
	char tmp[200];
	sprintf(tmp, "Setting %d destination addresses.\n", NUM_DEST);
	XPLMDebugString(tmp);

    // destination address
	for ( i=0; i<NUM_DEST; i++ ) {
		if (dest_enable[i]) {
			sprintf(tmp, "Configuring destination %d.\n", i);
			XPLMDebugString(tmp);
			
			dest_sockaddr[i].sin_family = AF_INET;
			dest_sockaddr[i].sin_port = htons(dest_port[i]);
			dest_sockaddr[i].sin_addr.s_addr = inet_addr(dest_ip[i]);
			memset(&(dest_sockaddr[i].sin_zero), '\0', 8);

			sprintf(addr_cleartext, "XData: dest[%d] : IP=%s  Port=%d\n",
					i, dest_ip[i], dest_port[i]);
			XPLMDebugString(addr_cleartext);
		}
	}
	
	XPLMDebugString("Finished binding network addresses\n");
}


void closeSocket() {

	if ( xdata_socket_open ) {
        #if IBM
		if ( closesocket(sockfd) == -1 ) {
        #else
	    if ( close(sockfd) == -1 ) {
        #endif
			XPLMDebugString("XData: failed - caught error while closing socket! (");
			XPLMDebugString((char * const) strerror(GET_ERRNO));
			XPLMDebugString(")\n");
		} else {
			XPLMDebugString("XData: socket closed\n");
		}
        #if IBM
        WSACleanup();
        #endif
		xdata_socket_open = 0;
		xdata_send_enabled = 0;
	}

}


int bindSocket() {

	if ( ! xdata_socket_open ) {

//        // doesn't seem to help...
//        #if IBM
//            // Set the re-use address option
//            int optval = 1;
//            int optresult = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
//                         (char *) &optval, sizeof (optval));
//            if (optresult == SOCKET_ERROR) {
//                XPLMDebugString("XData: setsockopt for SO_REUSEADDR failed!\n");
//                //WSAGetLastError();
//            }
//        #endif

		if (bind(sockfd, (struct sockaddr*)&recv_sockaddr, sizeof(struct sockaddr_in)) == -1) {
			char tmp[200];
			sprintf(tmp, "Error number %d.\n", GET_ERRNO);
			XPLMDebugString(tmp);
	
			XPLMDebugString("XData: caught error while binding socket (");
			XPLMDebugString((char * const) strerror(GET_ERRNO));
			XPLMDebugString(")\n");
			xdata_socket_open = 0;
			return -1;
		} else {
			XPLMDebugString("XData: socket bound!\n");
			xdata_socket_open = 1;
			return 1;
		}

	} else {
		return 1;
	}

}


int pollReceive() {

    int             res;
    fd_set          sready;
    struct timeval  nowait;

    FD_ZERO(&sready);
    FD_SET((unsigned int)sockfd, &sready);
    nowait.tv_sec = 0;    // specify how many seconds you would like to wait for timeout
    nowait.tv_usec = 0;   // how many microseconds? If both is zero, select will return immediately

    res = select(sockfd+1, &sready, NULL, NULL, &nowait);
    if( FD_ISSET(sockfd, &sready) )
        return 1;
    else
        return 0;
}


void resetSocket() {
    closeSocket();
    openSocket();
    setAddresses();
    bindSocket();
}

