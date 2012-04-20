/*
 * receiver.c
 *
 * callback function to receive packets
 *
 * used by: plugin.c
 * uses: net.c, packets.c
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
#include "structs.h"
#include "packets.h"
#include "net.h"
#include "receiver.h"


float receiveCallback(
                   float	inElapsedSinceLastCall,
                   float	inElapsedTimeSinceLastFlightLoop,
                   int		inCounter,
                   void *	inRefcon) {

	int packet_size;
	int packet_type_id = 0;
	char debug_message[256];
	//char packet_type[4];
	
	char data[1024];
	
	//XPLMDebugString("XData: receive callback called\n");

	if (xdata_plugin_enabled && xdata_send_enabled && xdata_socket_open) {
	
		//XPLMDebugString("XData: receive polling\n");

        if ( pollReceive() ) {
			//packet_size = recv(sockfd, (char *)&packet_type, 4, 0);
			packet_size = recv(sockfd, (char *)&data, 1024, 0);
			
			if( packet_size >= 0 ) {
				// TODO: tidy this up, use standard C functions?
				packet_type_id = (packet_type_id << 8) + data[3];
				packet_type_id = (packet_type_id << 8) + data[2];
				packet_type_id = (packet_type_id << 8) + data[1];
				packet_type_id = (packet_type_id << 8) + data[0];				

				packet_type_id = (int) custom_htonl((long)packet_type_id);
				
				sprintf(debug_message, "XData: read packet_type_id %d with packet_size %d\n", packet_type_id, packet_size);
				XPLMDebugString(debug_message);	

				if( packet_type_id == (int) PACKET_TYPE_REQUEST ) {
					memcpy(&req_packet, data+4, sizeof(struct RequestPacket));
					XPLMDebugString("XData: read RequestPacket\n");	
					decodeRequestPacket();
				} else
				if( packet_type_id == (int) PACKET_TYPE_COMMAND ) {
					memcpy(&efis_packet, data+4, sizeof(struct CommandPacket));
					XPLMDebugString("XData: read CommandPacket\n");	
					decodeCommandPacket();
				} else {
					sprintf(debug_message, "XData: received unknown packet type: %d\n", packet_type_id);
					XPLMDebugString(debug_message);	
				}
			}

/*			
            //addr_len = sizeof(orig_sockaddr);
            //packet_size = recvfrom(sockfd, (char *)&efis_packet, sizeof(struct CommandPacket), 0, (struct sockaddr *) &orig_sockaddr, &addr_len);
            packet_size = recv(sockfd, (char *)&efis_packet, sizeof(struct CommandPacket), 0);
            if ( packet_size < 0) {
                XPLMDebugString("XData: caught error while receiving packet! ");
                sprintf(debug_message, "packet_size=%d  errno: %d (", packet_size, GET_ERRNO);
                XPLMDebugString(debug_message);
                XPLMDebugString((char * const) strerror(GET_ERRNO));
                XPLMDebugString(")\n");
                return 1.0f;
            } else {
                XPLMDebugString("XData: packet received **************** \n");
                decodeCommandPacket();
                return recv_delay;
            }
*/
        }

    } else {
	
 		// print something to allow us to work out what happened.
		XPLMDebugString("XData: plugin enabled:");
		if( xdata_plugin_enabled ) {
			XPLMDebugString("true\n");
		} else {
			XPLMDebugString("false\n");
		}

		XPLMDebugString("XData: send enabled:");
		if( xdata_send_enabled ) {
			XPLMDebugString("true\n");
		} else {
			XPLMDebugString("false\n");
		}

		XPLMDebugString("XData: socket open:");
		if( xdata_socket_open ) {
			XPLMDebugString("true\n");
		} else {
			XPLMDebugString("false\n");
		}
	
		return 1.0f;
	}
	
	return recv_delay;
}

