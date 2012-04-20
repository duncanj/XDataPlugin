Problem: undefined reference to `somefunction()'

	C:\dev\xplugins\xdata>make
	make -f Makefile.Release
	make[1]: Entering directory `/c/dev/xplugins/xdata'
	g++ -enable-stdcall-fixup -Wl,-enable-auto-import -Wl,-enable-runtime-pseudo-rel
	oc -Wl,-s -shared -Wl,--out-implib,release/libwin.xpl1.a -o release/win.xpl1.dll
	 release/net.o release/settings.o release/xdata.o  release/win.xpl_resource_res.o
	 -L../SDK/Libraries/Win -L../../Qt/2010.05/mingw/lib -lXPLM -lXPWidgets ../ms/li
	b/libws2_32.a
	Creating library file: release/libwin.xpl1.a
	release/xdata.o:xdata.cpp:(.text+0x1a1): undefined reference to `openSocket()'
	collect2: ld returned 1 exit status
	make[1]: *** [release/win.xpl1.dll] Error 1
	make[1]: Leaving directory `/c/dev/xplugins/xdata'
	make: *** [release] Error 2
	
Solution: don't put C code in CPP files.  Rename xdata.cpp to xdata.c, clear out all built/generated files, then rebuild.

	
we want
 every second:
	lat/lon
	alt
	ground speed
	airspeed
	heading (magnetic)
	paused
 if acf changed, and/or perhaps every 30 sec:
	callsign/tailnum - sim/aircraft/view/acf_tailnum	byte[40]
	description - sim/aircraft/view/acf_descrip		byte[260]
	
 later, possibly
	sim/time/sim_speed	int	y	ratio	This is the multiplier for real-time...1 = realtme, 2 = 2x, 0 = paused, etc.
	sim/time/ground_speed	int	y	ratio	This is the multiplier on ground speed, for faster travel via double-distance
	
Build commands:
 qmake (do this first, to generate makefiles)
 make (do this second, to build the plugin) 