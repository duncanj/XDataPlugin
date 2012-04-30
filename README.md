XDataPlugin
===========

The XData plugin for XPlane (www.xplane.com)

XData is a generic plugin for X-Plane, which listens for UDP packets requesting particular items of data (datarefs) 
from the X-Plane simulator, along with the frequency you wish the data to be sent back.   The plugin then sends back
UDP packets containing the requested data, at the specified frequency.  For example, you can request engine 
temperatures, aircraft location, etc.

Here is a list of all the datarefs available in the current version of X-Plane:
http://www.xsquawkbox.net/xpsdk/docs/DataRefs.html


Status
======
The project is currently in anearly embryonic stage.  The plugin works and accepts requests for datarefs, then logs the
dataref values to the X-Plane logfile - returning data is not implemented yet, but shouldn't take long once I have a 
few free evenings.


Future plans
============
Send updates to X-Plane so that you can change values in the simulator from outside.

Implement a Java library that will interface with XData, so you can effectively interact with XPlane
without knowing any C code.

Write a java application that uses XData.