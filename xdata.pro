# Shared library without any Qt functionality
TEMPLATE = lib
QT -= gui core

CONFIG += warn_on plugin release
CONFIG -= thread exceptions qt rtti debug

VERSION = 1.0.0

INCLUDEPATH += ../SDK/CHeaders/XPLM
INCLUDEPATH += ../SDK/CHeaders/Wrappers
INCLUDEPATH += ../SDK/CHeaders/Widgets
INCLUDEPATH += ../../Qt/2010.05/mingw/include

# Defined to use X-Plane SDK 2.0 capabilities - no backward compatibility before 9.0
DEFINES += XPLM200

win32 {
    DEFINES += APL=0 IBM=1 LIN=0
    LIBS += -L../SDK/Libraries/Win
#	LIBS += -L../ms/lib
	LIBS += -L../../Qt/2010.05/mingw/lib
    LIBS += -lXPLM -lXPWidgets ../ms/lib/libws2_32.a
    TARGET = win.xpl
}

unix:!macx {
    DEFINES += APL=0 IBM=0 LIN=1
    TARGET = lin.xpl
    # WARNING! This requires the latest version of the X-SDK !!!!
    QMAKE_CXXFLAGS += -fvisibility=hidden
}

macx {
    DEFINES += APL=1 IBM=0 LIN=0
    TARGET = mac.xpl
    QMAKE_LFLAGS += -flat_namespace -undefined suppress

    # Build for multiple architectures.
    # The following line is only needed to build universal on PPC architectures.
    # QMAKE_MAC_SDK=/Developer/SDKs/MacOSX10.4u.sdk
    # The following line defines for which architectures we build.
    CONFIG += x86 ppc
}

HEADERS += \
	net.h \
	logic.h \
	xdata.h \
	settings.h \
	endianess.h \
	ids.h \
	datarefs.h \
	structs.h \
	packets.h \
	receiver.h \
	globals.h

SOURCES += \
	net.c \	
	logic.c \	
	settings.c \
	datarefs.c \
	packets.c \
	receiver.c \
	xdata.c
