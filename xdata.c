/*
 * HellWorld.c
 * 
 * This plugin implements the canonical first program.  In this case, we will 
 * create a window that has the text hello-world in it.  As an added bonus
 * the  text will change to 'This is a plugin' while the mouse is held down
 * in the window.  
 * 
 * This plugin demonstrates creating a window and writing mouse and drawing
 * callbacks for that window.
 * 
 */

#include <stdio.h>
#include <string.h>
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"
#include "XPLMPlugin.h"

#include "globals.h"
#include "net.h"
#include "logic.h"
#include "settings.h"

/*
 * Global Variables.  We will store our single window globally.  We also record
 * whether the mouse is down from our mouse handler.  The drawing handler looks
 * at this information and draws the appropriate display.
 * 
 */

XPLMWindowID	gWindow = NULL;
int				gClicked = 0;


// Define global vars
// plugin status
int     	xdata_plugin_enabled;
int		    xdata_send_enabled;



void MyDrawWindowCallback(
                                   XPLMWindowID         inWindowID,    
                                   void *               inRefcon);    

void MyHandleKeyCallback(
                                   XPLMWindowID         inWindowID,    
                                   char                 inKey,    
                                   XPLMKeyFlags         inFlags,    
                                   char                 inVirtualKey,    
                                   void *               inRefcon,    
                                   int                  losingFocus);    

int MyHandleMouseClickCallback(
                                   XPLMWindowID         inWindowID,    
                                   int                  x,    
                                   int                  y,    
                                   XPLMMouseStatus      inMouse,    
                                   void *               inRefcon);    


/*
 * XPluginStart
 * 
 * Our start routine registers our window and does any other initialization we 
 * must do.
 * 
 */
PLUGIN_API int XPluginStart(
						char *		outName,
						char *		outSig,
						char *		outDesc)
{
	/* First we must fill in the passed in buffers to describe our
	 * plugin to the plugin-system. */

	strcpy(outName, "XData");
	strcpy(outSig, "xplanesdk.examples.xdata");
	strcpy(outDesc, "A plugin that makes a window and shows a dataref.");

	initSettings();	
	
	registerDataRefs();
	registerCallbacks();

	
    if (openSocket() == 0) {
        return 0;
    }
	
	setAddresses();


	/* Now we create a window.  We pass in a rectangle in left, top,
	 * right, bottom screen coordinates.  We pass in three callbacks. */

	gWindow = XPLMCreateWindow(
					50, 600, 200, 500,			/* Area of the window. */
					1,							/* Start visible. */
					MyDrawWindowCallback,		/* Callbacks */
					MyHandleKeyCallback,
					MyHandleMouseClickCallback,
					NULL);						/* Refcon - not used. */
					
	/* We must return 1 to indicate successful initialization, otherwise we
	 * will not be called back again. */
	 
	return 1;
}

/*
 * XPluginStop
 * 
 * Our cleanup routine deallocates our window.
 * 
 */
PLUGIN_API void	XPluginStop(void)
{
	unregisterCallbacks();
	unregisterDataRefs();
	
	XPLMDestroyWindow(gWindow);
	closeSocket();
}

/*
 * XPluginDisable
 * 
 * We do not need to do anything when we are disabled, but we must provide the handler.
 * 
 */
PLUGIN_API void XPluginDisable(void)
{
	xdata_plugin_enabled = 0;
	xdata_send_enabled = 0;
}

/*
 * XPluginEnable.
 * 
 * We don't do any enable-specific initialization, but we must return 1 to indicate
 * that we may be enabled at this time.
 * 
 */
PLUGIN_API int XPluginEnable(void)
{
	xdata_send_enabled = 1;
	xdata_plugin_enabled = 1;
	
	return bindSocket();
}

/*
 * XPluginReceiveMessage
 * 
 * We don't have to do anything in our receive message handler, but we must provide one.
 * 
 */
PLUGIN_API void XPluginReceiveMessage(
					XPLMPluginID	inFromWho,
					long			inMessage,
					void *			inParam)
{
	if( inMessage == XPLM_MSG_PLANE_LOADED )
    {
        int* planeNo = (int*)(inParam);
        if ( planeNo == 0)
        {
            // do some stuff
			XPLMDebugString("XData: aircraft changed\n");

			char acf_desc[260];
			XPLMDataRef dataref_acf_desc = XPLMFindDataRef("sim/aircraft/view/acf_descrip");
			XPLMGetDatab(dataref_acf_desc, acf_desc, 0, 260);
			
			XPLMDebugString("XData: aircraft description: ");
			XPLMDebugString(acf_desc);
			XPLMDebugString("\n");
			
			sendAircraftPacket();
        }
    }
	else
	if( inMessage == XPLM_MSG_PLANE_CRASHED )
	{
		sendCrashedPacket();
	}

}

/*
 * MyDrawingWindowCallback
 * 
 * This callback does the work of drawing our window once per sim cycle each time
 * it is needed.  It dynamically changes the text depending on the saved mouse
 * status.  Note that we don't have to tell X-Plane to redraw us when our text
 * changes; we are redrawn by the sim continuously.
 * 
 */
void MyDrawWindowCallback(
                                   XPLMWindowID         inWindowID,    
                                   void *               inRefcon)
{
	int		left, top, right, bottom;
	float	color[] = { 1.0, 1.0, 1.0 }; 	/* RGB White */
	
	/* First we get the location of the window passed in to us. */
	XPLMGetWindowGeometry(inWindowID, &left, &top, &right, &bottom);
	
	/* We now use an XPLMGraphics routine to draw a translucent dark
	 * rectangle that is our window's shape. */
	XPLMDrawTranslucentDarkBox(left, top, right, bottom);
	
	char acftail[40];
	XPLMDataRef dataref_acf_tailnum = XPLMFindDataRef("sim/aircraft/view/acf_tailnum");
	XPLMGetDatab(dataref_acf_tailnum, acftail, 0, 40);
	

	/* Finally we draw the text into the window, also using XPLMGraphics
	 * routines.  The NULL indicates no word wrapping. */
	XPLMDrawString(color, left + 5, top - 20, 
		(char*)("XData plugin"), NULL, xplmFont_Basic);
	XPLMDrawString(color, left + 5, top - 50, 
		(char*)(gClicked ? "I'm a plugin" : acftail), NULL, xplmFont_Basic);
		
}                                   

/*
 * MyHandleKeyCallback
 * 
 * Our key handling callback does nothing in this plugin.  This is ok; 
 * we simply don't use keyboard input.
 * 
 */
void MyHandleKeyCallback(
                                   XPLMWindowID         inWindowID,    
                                   char                 inKey,    
                                   XPLMKeyFlags         inFlags,    
                                   char                 inVirtualKey,    
                                   void *               inRefcon,    
                                   int                  losingFocus)
{
}                                   

/*
 * MyHandleMouseClickCallback
 * 
 * Our mouse click callback toggles the status of our mouse variable 
 * as the mouse is clicked.  We then update our text on the next sim 
 * cycle.
 * 
 */
int MyHandleMouseClickCallback(
                                   XPLMWindowID         inWindowID,    
                                   int                  x,    
                                   int                  y,    
                                   XPLMMouseStatus      inMouse,    
                                   void *               inRefcon)
{
	/* If we get a down or up, toggle our status click.  We will
	 * never get a down without an up if we accept the down. */
	if ((inMouse == xplm_MouseDown) || (inMouse == xplm_MouseUp))
		gClicked = 1 - gClicked;
	
	/* Returning 1 tells X-Plane that we 'accepted' the click; otherwise
	 * it would be passed to the next window behind us.  If we accept
	 * the click we get mouse moved and mouse up callbacks, if we don't
	 * we do not get any more callbacks.  It is worth noting that we 
	 * will receive mouse moved and mouse up even if the mouse is dragged
	 * out of our window's box as long as the click started in our window's 
	 * box. */
	return 1;
}                                      