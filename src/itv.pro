 ######################################################################
 # Automatically generated by qmake (2.01a) Thu Aug 23 18:02:14 2007
 ######################################################################

 TEMPLATE = app

 QT += phonon

#  LIBS += -lrtmp

 FORMS += settings.ui
 RESOURCES += mediaplayer.qrc

 !win32:CONFIG += CONSOLE

 SOURCES += main.cpp mediaplayer.cpp
 HEADERS += mediaplayer.h

 target.path = /usr/bin
 INSTALLS += target 
