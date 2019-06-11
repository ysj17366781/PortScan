TARGET		= pinger

QT       += core network

TRANSLATIONS	+= $(NU_HOME)/translation/pinger_zh.ts
TRANSLATIONS	+= $(NU_HOME)/translation/pinger_en.ts
include($(NU_HOME)/include/nucleon.pri)

INCLUDEPATH	+= \
		$(NU_HOME)/src/cproject/hmi/include/nucommon \
		$(NU_HOME)/include/residentapi \
		$(NU_HOME)/include/support/osilayer \

HEADERS 	+= \
		pinger.h \
		debugger.h \
		util.h \
		myipdelegate.h \
		myipmodel.h	\

SOURCES 	+= \
		main.cpp \
		pinger.cpp \
		debugger.cpp \
		util.cpp \
		myipdelegate.cpp \
		myipmodel.cpp	\
		

LIBS		+=-losilayer -lnucommon -lresidentapi

RESOURCES += \
    application.qrc