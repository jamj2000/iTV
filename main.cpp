/****************************************************************************
**
** Copyright (C) 2013 jamj (jamj2000@gmail.com)
**
** Credits:
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies). for qmediaplayer
** Copyright (C) 2013 TVenLinux.com for channel list.
**
** License GPLv3+
**
***************************************************************************/


 #include <QtGui>
 #include "mediaplayer.h"

 const qreal DefaultVolume = -1.0;

 int main (int argc, char *argv[])
 {
     Q_INIT_RESOURCE(mediaplayer);
     QApplication app(argc, argv);
     app.setApplicationName("inTV");
     app.setOrganizationName("jamj2000");
     app.setWindowIcon(QIcon(QString(":/images/tv.png")));
     app.setQuitOnLastWindowClosed(true);

     QString fileName;
     qreal volume = DefaultVolume;
     bool smallScreen = false;


     MediaPlayer player;
     player.setSmallScreen(smallScreen);
     if (DefaultVolume != volume)
         player.setVolume(volume);
     //if (!fileName.isNull())
     //    player.setFile(fileName);

     if (smallScreen)
         player.showMaximized();
     else
         player.show();

     return app.exec();
 } 
