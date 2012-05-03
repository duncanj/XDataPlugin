
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


// config variables
int                 dest_enable[NUM_DEST];
char                dest_ip[NUM_DEST][20];
unsigned short int  dest_port[NUM_DEST];

unsigned short int  recv_port;
unsigned long int   recv_rate;
float               recv_delay;



// Config settings ---------------------------------------------------------------------------

void defaultSettings() {

	int i;

	for (i=0; i<NUM_DEST; i++) {
		dest_enable[i] = i==0 ? 1 : 0;
		strcpy(dest_ip[i], DEFAULT_DEST_IP);
		dest_port[i] = DEFAULT_DEST_PORT + i;
	}
	recv_port = DEFAULT_RECV_PORT;
	recv_rate = 10;
	recv_delay = 1.0f / (float)recv_rate;
}


void readConfig() {

	FILE	*cfg_file;
	char	cfg_line[120];
	char	param[40];
	char	dest_param[20];
	char	s_value[80];
	int		d_value;
	int i;

	cfg_file = fopen(CFG_FILE, "r");
	if (cfg_file != NULL) {
		XPLMDebugString("XData: reading ");
		XPLMDebugString(CFG_FILE);
		XPLMDebugString("\n");
		while (!feof(cfg_file)) {
			fgets(cfg_line, sizeof(cfg_line), cfg_file);
			sscanf(cfg_line, "%s %s", param, s_value);
			sscanf(s_value, "%d", &d_value);

			if (strcmp(param, "recv_port")==0) {
				if (d_value==0) {
					recv_port = DEFAULT_RECV_PORT;
				} else {
					recv_port = d_value;
				}
			}

			if (strcmp(param, "recv_rate")==0) {
				recv_rate = d_value;
				recv_delay = 1.0f / (float)recv_rate;
			}

			for (i=0; i<NUM_DEST; i++) {

				sprintf(dest_param, "dest_enable[%d]", i);
				if (strcmp(param, dest_param)==0) {
					dest_enable[i] = d_value;
				}

				sprintf(dest_param, "dest_ip[%d]", i);
				if (strcmp(param, dest_param)==0) {
					strcpy(dest_ip[i], s_value);
				}

				sprintf(dest_param, "dest_port[%d]", i);
				if (strcmp(param, dest_param)==0) {
					dest_port[i] = d_value;
				}

			}
		}
		fclose(cfg_file);
	}
}


void initSettings() {

    defaultSettings();
    readConfig();

}



void writeSettings() {

	FILE	*cfg_file;
	int		i;

	XPLMDebugString("XData: writing ");
	XPLMDebugString(CFG_FILE);
	// for the "\n", see below...

	cfg_file = fopen(CFG_FILE, "w");
	if (cfg_file != NULL) {

		fprintf(cfg_file, "cfg_version %d\n", PLUGIN_VERSION_NUMBER);

		fprintf(cfg_file, "recv_port %d\n", recv_port);
		fprintf(cfg_file, "recv_rate %ld\n", recv_rate);

		for (i=0; i<NUM_DEST; i++) {
			fprintf(cfg_file, "dest_enable[%d] %d\n", i, dest_enable[i]);
			fprintf(cfg_file, "dest_ip[%d] %s\n", i, dest_ip[i]);
			fprintf(cfg_file, "dest_port[%d] %d\n", i, dest_port[i]);
		}

		fprintf(cfg_file, "\n");

		fclose(cfg_file);

		XPLMDebugString(" : OK\n");

	} else {
		XPLMDebugString(" : ERROR!\n");
	}

}


