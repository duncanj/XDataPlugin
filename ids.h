
// Sim value ids ===============================================================

// Aircraft position
#define SIM_FLIGHTMODEL_POSITION_GROUNDSPEED 0
#define SIM_FLIGHTMODEL_POSITION_TRUE_AIRSPEED 1
#define SIM_FLIGHTMODEL_POSITION_MAGPSI 2		// magnetic heading
#define SIM_FLIGHTMODEL_POSITION_HPATH 3		// actual heading, relative to Z axis
#define SIM_FLIGHTMODEL_POSITION_LATITUDE 4		// double!
#define SIM_FLIGHTMODEL_POSITION_LONGITUDE 5 	// double!
#define SIM_FLIGHTMODEL_POSITION_PHI 6 			// roll angle
#define SIM_FLIGHTMODEL_POSITION_R 7 			// rotation rate
#define SIM_FLIGHTMODEL_POSITION_MAGVAR 8		// local magnetic variation
#define SIM_FLIGHTMODEL_POSITION_ELEVATION 9	// above MSL
#define SIM_FLIGHTMODEL_POSITION_Y_AGL 10		// height above ground
#define SIM_FLIGHTMODEL_POSITION_THETA 11       // pitch
#define SIM_FLIGHTMODEL_POSITION_VPATH 12       // fpa - actual pitch
#define SIM_FLIGHTMODEL_POSITION_ALPHA 13       // aoa - pitch relative to flown path
#define SIM_FLIGHTMODEL_POSITION_BETA 14        // yaw ( = slip or drift ? )
#define SIM_FLIGHTMODEL_FAILURES_ONGROUND_ANY 15 // It was misplaced and is not really a failure, you can use that to indicate when the wheels are on the ground

#define SIM_FLIGHTMODEL_WEIGHT_TOTAL 16        // total weight, kg
#define SIM_FLIGHTMODEL_WEIGHT_PAYLOAD 17        // payload weight, kg
#define SIM_FLIGHTMODEL_WEIGHT_FUEL 18        // total fuel weight, kg

#define SIM_TIME_SIM_SPEED 19			// sim/time/sim_speed  0=paused, 1=normal, 2=double normal speed, 4, 8.
#define SIM_TIME_GROUND_SPEED 20		// sim/time/ground_speed  0=paused, 1=normal, 2=double normal speed, 4, 8.


// Plugin Version
#define PLUGIN_VERSION_ID 999

