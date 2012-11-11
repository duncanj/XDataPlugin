#include <stdio.h>
#include <string.h>
#include <sys/timeb.h>

#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"
#include "XPLMPlugin.h"
#include "XPLMProcessing.h"
#include "XPLMNavigation.h"

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

float situation_update_period = 0.1f;  // 10x per second

int acf_packet_requested = 0; 



void sendRepositionedAtAirport() {
	XPLMDebugString("XData: sendRepositionedAtAirport called.\n");
	
	// where are we?
	float inLat;
	float inLon;
	
	XPLMDataRef dataref_latitude = XPLMFindDataRef("sim/flightmodel/position/latitude");
	inLat = XPLMGetDataf(dataref_latitude);
	
	XPLMDataRef dataref_longitude = XPLMFindDataRef("sim/flightmodel/position/longitude");
	inLon = XPLMGetDataf(dataref_longitude);
	
	
	XPLMNavRef navref = XPLMFindNavAid(
                                   NULL,
                                   NULL,
                                   &inLat,
                                   &inLon,
                                   NULL,
                                   xplm_Nav_Airport);
	
	float apt_lat;
	float apt_lon;
	float apt_height;
	char apt_id[32];
	char apt_name[256];
	
	XPLMGetNavAidInfo( navref,
					   NULL,
					   &apt_lat,
					   &apt_lon,
					   &apt_height,
					   NULL,
					   NULL,
					   &apt_id,
					   &apt_name,
					   NULL);
					   
	int i;
	int res;	
	char msg[256];
	
	sprintf(msg, "Repositioned at airport: %s (name: %s) height=%f lat=%f lon=%f\n", apt_id, apt_name, apt_height, apt_lat, apt_lon);
	XPLMDebugString(msg);
	
	// make endian corrections
	// COMMENTED OUT - THIS CRASHES X-PLANE.  WHY?
//	apt_lat = custom_htonf(apt_lat);
//	apt_lon = custom_htonf(apt_lon);
//	apt_height = custom_htonf(apt_height);
	
	strncpy(airport_packet.apt_id, apt_id, 32);
	strncpy(airport_packet.apt_name, apt_name, 256);
	airport_packet.apt_height = apt_height;
	airport_packet.apt_lat = apt_lat;
	airport_packet.apt_lon = apt_lon;
	

	
	if (xdata_plugin_enabled && xdata_send_enabled && xdata_socket_open) {
		strncpy(airport_packet.packet_id, "RAPT", 4);
		for (i=0; i<NUM_DEST; i++) {
			if (dest_enable[i]) {
				res = sendto(sockfd, (const char*)&airport_packet, sizeof(airport_packet), 0, (struct sockaddr *)&dest_sockaddr[i], sizeof(struct sockaddr));
#if IBM
				if ( res == SOCKET_ERROR ) {
					XPLMDebugString("XData: caught error while sending RAPT packet! (");
                    sprintf(msg, "%d", WSAGetLastError());
					XPLMDebugString(msg);
					XPLMDebugString(")\n");
				}
#else
				if ( res < 0 ) {
					XPLMDebugString("XData: caught error while sending RAPT packet! (");
					XPLMDebugString((char * const) strerror(GET_ERRNO));
					XPLMDebugString(")\n");
				}
#endif
			}
		}	
	}
	
}


void sendCrashedPacket() {
	XPLMDebugString("XData: sendCrashedPacket called.\n");

	int i;
	int res;
#if IBM
	char msg[80];
#endif
	
	if (xdata_plugin_enabled && xdata_send_enabled && xdata_socket_open) {
		char data[4];
		strncpy(data, "CRSH", 4);

		for (i=0; i<NUM_DEST; i++) {
			if (dest_enable[i]) {
				res = sendto(sockfd, (const char*)&data, 4, 0, (struct sockaddr *)&dest_sockaddr[i], sizeof(struct sockaddr));
#if IBM
				if ( res == SOCKET_ERROR ) {
					XPLMDebugString("XData: caught error while sending CRSH packet! (");
                    sprintf(msg, "%d", WSAGetLastError());
					XPLMDebugString(msg);
					XPLMDebugString(")\n");
				}
#else
				if ( res < 0 ) {
					XPLMDebugString("XData: caught error while sending CRSH packet! (");
					XPLMDebugString((char * const) strerror(GET_ERRNO));
					XPLMDebugString(")\n");
				}
#endif
			}
		}
	}
}



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

enum {response_max_size=1000};  // http://compgroups.net/comp.lang.c/explanation-needed-for-const-int-error-variably-m/284354

char response_data[response_max_size];
int response_index;
int number_of_points;

float sendRequestedDataCallback(
                                   float	inElapsedSinceLastCall,
                                   float	inElapsedTimeSinceLastFlightLoop,
                                   int		inCounter,
                                   void *	inRefcon) {

	int i;
	int res;
#if IBM
	char msg[80];
#endif
	struct RequestRecord rr;

	if( DEBUG ) {
		XPLMDebugString("XData: sendRequestedDataCallback called.\n");
	}

	if (xdata_plugin_enabled && xdata_send_enabled && xdata_socket_open) {
	
		if( acf_packet_requested == 1 ) {			
			acf_packet_requested = 0;
			sendAircraftPacket();
		}
	
		// Tried a number of things, but absolute current time in millis just overflows and isn't printable via sprintf, it seems, as a long long.
		// So: initializing a start time, then evaluating time since that, which fits into an int.
		if( t_start.time == 0 ) {
			ftime(&t_start);
			XPLMDebugString("Initialising t_start.\n");
		}


		struct timeb t_current;
		ftime(&t_current);
		int t_diff = (int) (1000.0 * (t_current.time - t_start.time) + (t_current.millitm - t_start.millitm));
		
		// clear the buffer
		memset(response_data, 0, response_max_size);
		
//		sprintf(msg, "Time check: milliseconds since start = %d\n", t_diff);
//		XPLMDebugString(msg);
				
		strncpy(response_data, "REQD", 4);		// first 4 bytes indicate packet type
		response_index = 8;
		number_of_points = 0;
		
		// work out which requests to send
		for( i=0; i<=max_requested_index; i++ ) {
			rr = request_records[i];
			if( rr.enabled && request_records[i].dataref != NULL ) {
				//sprintf(msg, "%d - Checking %s which is scheduled for %d\n", t_diff, rr.dataref_name, rr.time_next_send);
				//XPLMDebugString(msg);
				if( rr.time_next_send == 0 || t_diff > rr.time_next_send ) {
					
					// let's work out the size of the data
					int size = 0;
					
					int nbrArrayValues = 0;
					
					int intValue = 0;
					float floatValue = 0.0f;
					double doubleValue = 0.0;
					int intArrayValues[20];
					float floatArrayValues[20];
					char data[900];
					
					if( request_records[i].datatype == xplmType_Int ) {
						intValue = XPLMGetDatai(request_records[i].dataref);
						size = 4;
					} else
					if( request_records[i].datatype == xplmType_Float ) {
						floatValue = XPLMGetDataf(request_records[i].dataref);
						size = 4;
					} else
					if( request_records[i].datatype == xplmType_Double || request_records[i].datatype == 6 ) {  // hack based on observed values
						doubleValue = XPLMGetDatad(request_records[i].dataref);
						size = 8;
					} else			
					if( request_records[i].datatype == xplmType_FloatArray ) {
						nbrArrayValues = XPLMGetDatavf(request_records[i].dataref, floatArrayValues, 0, 20);
						size = (4 * nbrArrayValues);
					} else
					if( request_records[i].datatype == xplmType_IntArray ) {
						nbrArrayValues = XPLMGetDatavi(request_records[i].dataref, intArrayValues, 0, 20);
						size = (4 * nbrArrayValues);
					} else
					if( request_records[i].datatype == xplmType_Data ) {
						nbrArrayValues = XPLMGetDatab(request_records[i].dataref, data, 0, 900);
						size = nbrArrayValues;
					}
				
					
					// will it fit in the buffer
					int index_after_add = response_index + 4 + 4 + 4 + size;
					if( index_after_add >= response_max_size ) {
						// if not, send it
						// first update response with number of points
						int convertedNum = custom_htonl(number_of_points);
						memcpy(response_data+4, &convertedNum, 4);
						
						if( DEBUG ) {
							sprintf(msg, "Packet ready to go: number_of_points=%d response_index=%d\n", number_of_points, response_index);
							XPLMDebugString(msg);
							XPLMDebugString("Packet data: ");
							int p=0;
							for( p=0; p<response_index; p++ ) {
								sprintf(msg, "%d:%d ", p, (int)response_data[p]);
								XPLMDebugString(msg);
							}
							XPLMDebugString("\n");
						}
						
						// ready to go	
						for (i=0; i<NUM_DEST; i++) {
							if (dest_enable[i]) {
								res = sendto(sockfd, response_data, response_index, 0, (struct sockaddr *)&dest_sockaddr[i], sizeof(struct sockaddr));
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
						
						XPLMDebugString("Sent packet\n");						
						response_index = 8;
						number_of_points = 0;
						
					} else {			
						// otherwise add it to the buffer
						
						// TODO figure out how to either convert each of the above datatypes into char[] data, or
						// simply memcpy from the original dataref into the buffer.
						
						// ID|DATATYPE|LENGTH|DATA....
						int convertedI = custom_htonl(i);
						memcpy(response_data+response_index, &convertedI, 4);
						response_index += 4;
						
						int convertedDatatype = custom_htonl(request_records[i].datatype);
						memcpy(response_data+response_index, &convertedDatatype, 4);
						response_index += 4;
						
						int convertedSize = custom_htonl(size);
						memcpy(response_data+response_index, &convertedSize, 4);
						response_index += 4;

						if( request_records[i].datatype == xplmType_Int ) {
							int convertedInt = custom_htonl(intValue);
							memcpy(response_data+response_index, &convertedInt, 4);
							response_index += 4;
						} else
						if( request_records[i].datatype == xplmType_Float ) {
							// skipping endianness conversion of float too, similar problem to double?
							//float convertedFloat = custom_htonf(floatValue);
							memcpy(response_data+response_index, &floatValue, 4);
							response_index += 4;
							if( DEBUG ) {
								sprintf(msg, "Converted a float: %f\n", floatValue);
								XPLMDebugString(msg);
								sprintf(msg, "%d %d %d %d\n", (int)response_data[response_index-4], (int)response_data[response_index-3], (int)response_data[response_index-2], (int)response_data[response_index-1]);
								XPLMDebugString(msg);								
							}							
						} else
						if( request_records[i].datatype == xplmType_Double || request_records[i].datatype == 6 ) {  // hack based on observed values
						
							// Now this is weird.  Converting the double breaks it somehow.  Even reversing the byte order at the receiving
							// end doesn't work..
							//double convertedDouble = custom_htond(doubleValue);
							memcpy(response_data+response_index, &doubleValue, 8);
							response_index += 8;
						} else			
						if( request_records[i].datatype == xplmType_FloatArray ) {
							int d = 0;
							for( d = 0; d<nbrArrayValues; d++ ) {
								float convertedFloat = custom_htonf(floatArrayValues[d]);
								memcpy(response_data+response_index, &convertedFloat, 4);
								response_index += 4;
							}
						} else
						if( request_records[i].datatype == xplmType_IntArray ) {
							int d = 0;
							for( d = 0; d<nbrArrayValues; d++ ) {
								int convertedInt = custom_htonl(intArrayValues[d]);
								memcpy(response_data+response_index, &convertedInt, 4);
								response_index += 4;
							}
						} else
						if( request_records[i].datatype == xplmType_Data ) {
							memcpy(response_data+response_index, &data, size);
							response_index += size;
						}
						
						
						
						
						
						response_index = index_after_add;
						number_of_points++;	
						
						if( DEBUG ) {
							sprintf(msg, "Packet accumulating: number_of_points=%d response_index=%d\n", number_of_points, response_index);
							XPLMDebugString(msg);						
						}
						
					}

					// schedule next time
					request_records[i].time_next_send = t_diff + rr.every_millis;
//					sprintf(msg, "%d - Would send %s and rescheduled (every %d millis) for %d\n", t_diff, rr.dataref_name, rr.every_millis, rr.time_next_send);
//					XPLMDebugString(msg);
				}
			}
		}
		
		// does the buffer contain any data?
		if( number_of_points > 0 ) {
			// send it
			
			// first update response with number of points
			int convertedNum = custom_htonl(number_of_points);
			memcpy(response_data+4, &convertedNum, 4);
			
			if( DEBUG ) {
				sprintf(msg, "Packet ready to go: number_of_points=%d response_index=%d\n", number_of_points, response_index);
				XPLMDebugString(msg);
				XPLMDebugString("Packet data: ");
				int p=0;
				for( p=0; p<response_index; p++ ) {
					sprintf(msg, "%d:%d ", p, (int)response_data[p]);
					XPLMDebugString(msg);
				}
				XPLMDebugString("\n");
			}
			
			// ready to go		
			for (i=0; i<NUM_DEST; i++) {
				if (dest_enable[i]) {
					res = sendto(sockfd, response_data, response_index, 0, (struct sockaddr *)&dest_sockaddr[i], sizeof(struct sockaddr));
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
			
			if( DEBUG ) {
				XPLMDebugString("Sent packet.\n");				
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
		return 0.1;

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
	
		return 0.1f;
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

