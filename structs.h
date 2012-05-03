

// Packet data structures
struct SimDataPoint {
	long	id;
	float	value;
};

struct SimDataPacket {
	char				packet_id[4];
	long				nb_of_sim_data_points;
	struct SimDataPoint sim_data_points[150];
};


struct AircraftDataPacket {
	char				packet_id[4];
	char                tailnum[40];
	char                icao[40];
	char                description[260];
	char 				filename[256];
};

struct CommandPacket {
	long				nb_of_command_points;
	struct SimDataPoint command_points[100];
};

struct AirportRepositionPacket {
	char packet_id[4];
	char apt_id[32];
	char apt_name[256];
	float apt_height;
	float apt_lat;
	float apt_lon;	
};

struct RequestForDataref {
	long 				ref_id;
	float				frequency;
	char				dataref_name[80];
};

struct RequestPacket {
	long				nb_of_requests;
	struct RequestForDataref requests[10];
};

// id is implicit - these will be stored in an array
struct RequestRecord {
	int			enabled;
	XPLMDataTypeID datatype;
	XPLMDataRef dataref;
	char		dataref_name[80];
	int			time_last_sent;
	int 		every_millis;
	int			time_next_send;
};


struct RequestRecord  request_records[1000]; // the uber structure of every requested dataref.
int max_requested_index; // the biggest index requested so far, so we don't have to search too much




