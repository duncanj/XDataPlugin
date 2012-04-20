
// packet creation functions
//int		createSituationPacket(void);
int		createAircraftPacket(void);

// command packet decode function
void decodeCommandPacket(void);
void decodeRequestPacket(void);

// The data packets
extern struct SimDataPacket     	sim_packet;
extern struct AircraftDataPacket	acf_packet[10];
extern struct CommandPacket    		efis_packet;
extern struct RequestPacket    		req_packet;
