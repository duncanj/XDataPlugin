
// Global constants ============================

// version info
#define PLUGIN_VERSION_TEXT "XData Plugin 1.0 Beta 1"
#define PLUGIN_VERSION_NUMBER 10001


// max number of IP/UDP destinations
#define NUM_DEST 4


// defaults for local IP address and ports
#define DEFAULT_DEST_IP "127.0.0.1"
#define DEFAULT_DEST_PORT 49010
#define DEFAULT_RECV_PORT 49009

#define DEBUG 0

// config file name
#if IBM
#define CFG_FILE "Resources\\plugins\\XData_plugin.cfg"
#else
#define CFG_FILE "Resources/plugins/XData_plugin.cfg"
#endif
