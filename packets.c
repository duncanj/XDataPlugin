
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"
#include "XPLMNavigation.h"
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMMenus.h"
#include "XPLMPlanes.h"
//#include "XPWidgets.h"
//#include "XPStandardWidgets.h"


#include "globals.h"
#include "ids.h"
#include "structs.h"
#include "datarefs.h"
#include "endianess.h"


// Define global vars
// The data packets =========================================

struct SimDataPacket     	sim_packet;
struct AircraftDataPacket	acf_packet;
struct CommandPacket    	efis_packet;
struct RequestPacket		req_packet;

int max_packet_size = 0;
char msg[200];




void decodeCommandPacket(void) {
	char debug_message[256];
    int i, nb;
    int id;
    float float_value;

    nb = (int) custom_ntohl(efis_packet.nb_of_command_points);
	
	sprintf(debug_message, "Decoding command packet, with %d data points...\n", nb);
	XPLMDebugString(debug_message);	

	for (i=0; i<nb; i++) {
	    id = custom_ntohl(efis_packet.command_points[i].id);
        float_value = custom_ntohf(efis_packet.command_points[i].value);
        //writeDataRef(id, float_value);
		
		sprintf(debug_message, "    index %d   id %d   value %f\n", i, id, float_value);
		XPLMDebugString(debug_message);	
	}

}


void decodeRequestPacket(void) {
	char debug_message[256];
    int i, nb;
    int id;
    float frequency;
	char dataref_name[80];

    nb = (int) custom_ntohl(req_packet.nb_of_requests);
	
	sprintf(debug_message, "Decoding request packet, with %d requests...\n", nb);
	XPLMDebugString(debug_message);	

	for (i=0; i<nb; i++) {
	    id = custom_ntohl(req_packet.requests[i].ref_id);
        frequency = custom_ntohf(req_packet.requests[i].frequency);
		strncpy(dataref_name, req_packet.requests[i].dataref_name, 80);
        //writeDataRef(id, float_value);
		
		sprintf(debug_message, "    index %d   id %d   frequency %f   dataref_name %s\n", i, id, frequency, dataref_name);
		XPLMDebugString(debug_message);	
		
		if( id > max_requested_index ) {
			max_requested_index = id;
		}
		strncpy(request_records[id].dataref_name, dataref_name, 80);
		request_records[id].dataref = XPLMFindDataRef(dataref_name);
		request_records[id].datatype = XPLMGetDataRefTypes(request_records[id].dataref);
		request_records[id].every_millis = (int)(1000.0f / frequency);
		request_records[id].time_last_sent = 0;
		request_records[id].time_next_send = 0;
		request_records[id].enabled = 1;
		
		if( request_records[id].dataref != NULL ) {
			if( request_records[id].datatype == xplmType_Int ) {
				int value = XPLMGetDatai(request_records[id].dataref);
				sprintf(debug_message, "   dataref_name %s   int value %d\n", request_records[id].dataref_name, value);
				XPLMDebugString(debug_message);	
			} else
			if( request_records[id].datatype == xplmType_Float ) {
				float value = XPLMGetDataf(request_records[id].dataref);
				sprintf(debug_message, "   dataref_name %s   float value %f\n", request_records[id].dataref_name, value);
				XPLMDebugString(debug_message);	
			} else
			if( request_records[id].datatype == xplmType_Double || request_records[id].datatype == 6 ) {  // hack based on observed values
				double value = XPLMGetDatad(request_records[id].dataref);
				sprintf(debug_message, "   dataref_name %s   double value %f\n", request_records[id].dataref_name, value);
				XPLMDebugString(debug_message);	
			} else			
			if( request_records[id].datatype == xplmType_FloatArray ) {
				float values[20];
				long size = XPLMGetDatavf(request_records[id].dataref, values, 0, 20);

				sprintf(debug_message, "   dataref_name %s   float values: ", request_records[id].dataref_name);
				XPLMDebugString(debug_message);	
				int x;
				for( x=0;x<size;x++ ) {
					sprintf(debug_message, "%f ", values[x]);
					XPLMDebugString(debug_message);	
				}
				XPLMDebugString("\n");	
			} else
			if( request_records[id].datatype == xplmType_IntArray ) {
				int values[20];
				long size = XPLMGetDatavi(request_records[id].dataref, values, 0, 20);

				sprintf(debug_message, "   dataref_name %s   int values: ", request_records[id].dataref_name);
				XPLMDebugString(debug_message);	
				int x;
				for( x=0;x<size;x++ ) {
					sprintf(debug_message, "%d ", values[x]);
					XPLMDebugString(debug_message);	
				}
				XPLMDebugString("\n");	
			} else
			if( request_records[id].datatype == xplmType_Data ) {
				char value[1024];
				long num_bytes = XPLMGetDatab(request_records[id].dataref, value, 0, 1024);
				sprintf(debug_message, "   dataref_name %s   bytes value %s\n", request_records[id].dataref_name, value);
				XPLMDebugString(debug_message);	
			}
		}
		
		
		
		sprintf(debug_message, "  ->  id %d   dataref_name %s   every_millis %d   datatype %d\n", id, request_records[id].dataref_name, request_records[id].every_millis, request_records[id].datatype);
		XPLMDebugString(debug_message);	
		

/*
	int			enabled;
	XPLMDataRef dataref;
	char		dataref_name[80];
	long		time_last_sent;
	long 		every_millis;
	long		time_next_send;
*/			
			
	}

}

/*
int createSituationPacket(void) {

	int i = 0;
	int packet_size;

	strncpy(sim_packet.packet_id, "SITU", 4);

	sim_packet.sim_data_points[i].id = custom_htonl((long) PLUGIN_VERSION_ID);
	sim_packet.sim_data_points[i].value = custom_htonf((float) PLUGIN_VERSION_NUMBER);
	i++;


	sim_packet.sim_data_points[i].id = custom_htonl((long) SIM_FLIGHTMODEL_POSITION_GROUNDSPEED);
	sim_packet.sim_data_points[i].value = custom_htonf(XPLMGetDataf(groundspeed));
	i++;
	sim_packet.sim_data_points[i].id = custom_htonl((long) SIM_FLIGHTMODEL_POSITION_TRUE_AIRSPEED);
	sim_packet.sim_data_points[i].value = custom_htonf(XPLMGetDataf(true_airspeed));
	i++;
	sim_packet.sim_data_points[i].id = custom_htonl((long) SIM_FLIGHTMODEL_POSITION_MAGPSI);
	sim_packet.sim_data_points[i].value = custom_htonf(XPLMGetDataf(magpsi));
	i++;
	sim_packet.sim_data_points[i].id = custom_htonl((long) SIM_FLIGHTMODEL_POSITION_HPATH);
	sim_packet.sim_data_points[i].value = custom_htonf(XPLMGetDataf(hpath));
	i++;

	sim_packet.sim_data_points[i].id = custom_htonl((long) SIM_FLIGHTMODEL_POSITION_LATITUDE);
	sim_packet.sim_data_points[i].value = custom_htonf( (float) XPLMGetDatad(latitude) );
	i++;

	sim_packet.sim_data_points[i].id = custom_htonl((long) SIM_FLIGHTMODEL_POSITION_LONGITUDE);
	sim_packet.sim_data_points[i].value = custom_htonf( (float) XPLMGetDatad(longitude) );
	i++;

	sim_packet.sim_data_points[i].id = custom_htonl((long) SIM_FLIGHTMODEL_POSITION_PHI);
	sim_packet.sim_data_points[i].value = custom_htonf(XPLMGetDataf(phi));
	i++;
	sim_packet.sim_data_points[i].id = custom_htonl((long) SIM_FLIGHTMODEL_POSITION_R);
	sim_packet.sim_data_points[i].value = custom_htonf(XPLMGetDataf(r));
	i++;
	sim_packet.sim_data_points[i].id = custom_htonl((long) SIM_FLIGHTMODEL_POSITION_MAGVAR);
	sim_packet.sim_data_points[i].value = custom_htonf(XPLMGetDataf(magvar));
	i++;
	sim_packet.sim_data_points[i].id = custom_htonl((long) SIM_FLIGHTMODEL_POSITION_ELEVATION);
	sim_packet.sim_data_points[i].value = custom_htonf( (float) XPLMGetDatad(msl) );
	i++;
	sim_packet.sim_data_points[i].id = custom_htonl((long) SIM_FLIGHTMODEL_POSITION_Y_AGL);
	sim_packet.sim_data_points[i].value = custom_htonf(XPLMGetDataf(agl));
	i++;
	sim_packet.sim_data_points[i].id = custom_htonl((long) SIM_FLIGHTMODEL_POSITION_THETA);
	sim_packet.sim_data_points[i].value = custom_htonf(XPLMGetDataf(theta));
	i++;
	sim_packet.sim_data_points[i].id = custom_htonl((long) SIM_FLIGHTMODEL_POSITION_VPATH);
	sim_packet.sim_data_points[i].value = custom_htonf(XPLMGetDataf(vpath));
	i++;
	sim_packet.sim_data_points[i].id = custom_htonl((long) SIM_FLIGHTMODEL_POSITION_ALPHA);
	sim_packet.sim_data_points[i].value = custom_htonf(XPLMGetDataf(alpha));
	i++;
	sim_packet.sim_data_points[i].id = custom_htonl((long) SIM_FLIGHTMODEL_POSITION_BETA);
	sim_packet.sim_data_points[i].value = custom_htonf(XPLMGetDataf(beta));
	i++;
	sim_packet.sim_data_points[i].id = custom_htonl((long) SIM_FLIGHTMODEL_FAILURES_ONGROUND_ANY);
	sim_packet.sim_data_points[i].value = custom_htonf((float) XPLMGetDatai(on_ground));
	i++;


	sim_packet.sim_data_points[i].id = custom_htonl((long) SIM_FLIGHTMODEL_WEIGHT_TOTAL );
	sim_packet.sim_data_points[i].value = custom_htonf(XPLMGetDataf(weight_total));
	i++;
	sim_packet.sim_data_points[i].id = custom_htonl((long) SIM_FLIGHTMODEL_WEIGHT_PAYLOAD );
	sim_packet.sim_data_points[i].value = custom_htonf(XPLMGetDataf(weight_payload));
	i++;
	sim_packet.sim_data_points[i].id = custom_htonl((long) SIM_FLIGHTMODEL_WEIGHT_FUEL );
	sim_packet.sim_data_points[i].value = custom_htonf(XPLMGetDataf(weight_fuel));
	i++;

	sim_packet.sim_data_points[i].id = custom_htonl((long) SIM_TIME_SIM_SPEED );
	sim_packet.sim_data_points[i].value = custom_htonf(XPLMGetDataf(sim_speed));
	i++;
	sim_packet.sim_data_points[i].id = custom_htonl((long) SIM_TIME_GROUND_SPEED );
	sim_packet.sim_data_points[i].value = custom_htonf(XPLMGetDataf(ground_speed));
	i++;

	
	
	// now we know the number of datapoints
	sim_packet.nb_of_sim_data_points = custom_htonl( (long) i );


	// packet size : char[4] + long + ( # * ( long + float) )
	packet_size = 8 + i * 8;
	if ( packet_size > max_packet_size) {
        max_packet_size = packet_size;
        sprintf(msg, "XData: max packet size (ADCD): %d\n", max_packet_size);
        XPLMDebugString(msg);
    }
	return packet_size;

}
*/

int createAircraftPacket(void) {
	strncpy(acf_packet.packet_id, "ACFT", 4);

	// haven't cached the datarefs here as it'll only happen occasionally
	XPLMDataRef dataref_acf_tailnum = XPLMFindDataRef("sim/aircraft/view/acf_tailnum");
	XPLMGetDatab(dataref_acf_tailnum, acf_packet.tailnum, 0, 40);
	
	XPLMDataRef dataref_acf_icao = XPLMFindDataRef("sim/aircraft/view/acf_ICAO");
	XPLMGetDatab(dataref_acf_icao, acf_packet.icao, 0, 40);

	XPLMDataRef dataref_acf_desc = XPLMFindDataRef("sim/aircraft/view/acf_descrip");
	XPLMGetDatab(dataref_acf_desc, acf_packet.description, 0, 260);
	
	char filepath[512];  // discarded
	XPLMGetNthAircraftModel(0, acf_packet.filename, filepath);
	

	int packet_size = 4 + 40 + 40 + 260 + 256;
	return packet_size;
}




