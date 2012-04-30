#include <stdio.h>
#include <string.h>
#include <sys/timeb.h>

#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"
#include "XPLMPlugin.h"
#include "XPLMProcessing.h"

#include "globals.h"
#include "net.h"
#include "structs.h"
#include "packets.h"
#include "settings.h"
#include "datarefs.h"
#include "xdata.h"
#include "receiver.h"


XPLMDataRef dataref_acf_desc;
XPLMDataRef dataref_acf_tailnum;

float situation_update_period = 1.0f;  // every second




void sendAircraftPacket() {
	XPLMDebugString("XData: sendAircraftPacket called.\n");

	int i;
	int packet_size;
	int res;
#if IBM
	char msg[80];
#endif
	
	if (xdata_plugin_enabled && xdata_send_enabled && xdata_socket_open) {

		packet_size = createAircraftPacket();

		for (i=0; i<NUM_DEST; i++) {
			if (dest_enable[i]) {
				res = sendto(sockfd, (const char*)&acf_packet, packet_size, 0, (struct sockaddr *)&dest_sockaddr[i], sizeof(struct sockaddr));
#if IBM
				if ( res == SOCKET_ERROR ) {
					XPLMDebugString("XData: caught error while sending ACFT packet! (");
                    sprintf(msg, "%d", WSAGetLastError());
					XPLMDebugString(msg);
					XPLMDebugString(")\n");
				}
#else
				if ( res < 0 ) {
					XPLMDebugString("XData: caught error while sending ACFT packet! (");
					XPLMDebugString((char * const) strerror(GET_ERRNO));
					XPLMDebugString(")\n");
				}
#endif
			}
		}
	}
}

// attempt to record start time
struct timeb t_start;

float sendRequestedDataCallback(
                                   float	inElapsedSinceLastCall,
                                   float	inElapsedTimeSinceLastFlightLoop,
                                   int		inCounter,
                                   void *	inRefcon) {

	int i;
	int packet_size;
	int res;
#if IBM
	char msg[80];
#endif
	struct RequestRecord rr;

	XPLMDebugString("XData: sendRequestedDataCallback called.\n");

	if (xdata_plugin_enabled && xdata_send_enabled && xdata_socket_open) {
	
		// Tried a number of things, but absolute current time in millis just overflows and isn't printable via sprintf, it seems, as a long long.
		// So: initializing a start time, then evaluating time since that, which fits into an int.
		if( t_start.time == 0 ) {
			ftime(&t_start);
			XPLMDebugString("Initialising t_start.\n");
		}


		struct timeb t_current;
		ftime(&t_current);
		int t_diff = (int) (1000.0 * (t_current.time - t_start.time) + (t_current.millitm - t_start.millitm));
		
//		sprintf(msg, "Time check: milliseconds since start = %d\n", t_diff);
//		XPLMDebugString(msg);
		
		
		// work out which requests to send
		for( i=0; i<=max_requested_index; i++ ) {
			rr = request_records[i];
			if( rr.enabled ) {
				//sprintf(msg, "%d - Checking %s which is scheduled for %d\n", t_diff, rr.dataref_name, rr.time_next_send);
				//XPLMDebugString(msg);
				if( rr.time_next_send == 0 || t_diff > rr.time_next_send ) {
					// send it

					// schedule next time
					request_records[i].time_next_send = t_diff + rr.every_millis;
					sprintf(msg, "%d - Would send %s and rescheduled (every %d millis) for %d\n", t_diff, rr.dataref_name, rr.every_millis, rr.time_next_send);
					XPLMDebugString(msg);
				}
			}
		}
/*
	long		time_last_sent;
	long 		every_millis;
	long		time_next_send;
	
	
every 100 ms, create a buffer (well, reuse it for performance) and start populating it with eligible data.  Keep going until the next ref would overflow the buffer.
 send the packet, then resume from the current ref.  until there are no more eligible refs.  send that last packet, if there's something in it.
 upon adding the data to the packet, update the struct with the calculated next send time.
 
REQD|COUNT|ID|DATATYPE|LENGTH|DATA.....................................|(repeated ID, TYPE, LEN, DATA)..

	
*/	
	
/*
		packet_size = createSituationPacket();

		for (i=0; i<NUM_DEST; i++) {
			if (dest_enable[i]) {
				res = sendto(sockfd, (const char*)&sim_packet, packet_size, 0, (struct sockaddr *)&dest_sockaddr[i], sizeof(struct sockaddr));
#if IBM
				if ( res == SOCKET_ERROR ) {
					XPLMDebugString("XData: caught error while sending REQD packet! (");
                    sprintf(msg, "%d", WSAGetLastError());
					XPLMDebugString(msg);
					XPLMDebugString(")\n");
				}
#else
				if ( res < 0 ) {
					XPLMDebugString("XData: caught error while sending REQD packet! (");
					XPLMDebugString((char * const) strerror(GET_ERRNO));
					XPLMDebugString(")\n");
				}
#endif
			}
		}
*/
		return situation_update_period;

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

}



void registerDataRefs() 
{
	XPLMDebugString("Registering datarefs\n");
	dataref_acf_desc = XPLMFindDataRef("sim/aircraft/view/acf_descrip");
	dataref_acf_tailnum = XPLMFindDataRef("sim/aircraft/view/acf_tailnum");
}

void unregisterDataRefs()
{
}

void registerCallbacks()
{
	XPLMDebugString("Registering callbacks\n");
	XPLMRegisterFlightLoopCallback(
							sendRequestedDataCallback,
							situation_update_period,
							NULL);
							
	
	XPLMRegisterFlightLoopCallback(
							receiveCallback,
							recv_delay,
							NULL);							
}

void unregisterCallbacks()
{
	XPLMDebugString("Unregistering callbacks\n");
	XPLMUnregisterFlightLoopCallback(sendRequestedDataCallback, NULL);
	XPLMUnregisterFlightLoopCallback(receiveCallback, NULL);
}

