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

 #ifndef MEDIALAYER_H
 #define MEDIAPLAYER_H

 #include <QtGui/QWidget>
 #include <QtGui/QApplication>
 #include <QtCore/QTimerEvent>
 #include <QtGui/QShowEvent>
 #include <QtGui/QIcon>
 #include <QtCore/QBasicTimer>
 #include <QtGui/QAction>

 #include <phonon/audiooutput.h>
 #include <phonon/backendcapabilities.h>
 #include <phonon/effect.h>
 #include <phonon/effectparameter.h>
 #include <phonon/effectwidget.h>
 #include <phonon/mediaobject.h>
 #include <phonon/seekslider.h>
 #include <phonon/videowidget.h>
 #include <phonon/volumeslider.h>

 #include <QProcess>
 #include <QThread>
 #include <QTemporaryFile>
 #include <QToolButton>
 #include <QScrollArea>

 class QPushButton;
 class QLabel;
 class QSlider;
 class QTextEdit;
 class QMenu;
 class Ui_settings;
 class QMovie;

 class MediaPlayer;



 class MediaVideoWidget : public Phonon::VideoWidget
 {
     Q_OBJECT

 public:
     MediaVideoWidget(MediaPlayer *player, QWidget *parent = 0);

 public slots:
     // Over-riding non-virtual Phonon::VideoWidget slot
     void setFullScreen(bool);

 signals:
     void fullScreenChanged(bool);

 protected:
     void mouseDoubleClickEvent(QMouseEvent *e);
     void keyPressEvent(QKeyEvent *e);
     bool event(QEvent *e);
     void timerEvent(QTimerEvent *e);
     void dropEvent(QDropEvent *e);
     void dragEnterEvent(QDragEnterEvent *e);

 private:
     MediaPlayer *m_player;
     QBasicTimer m_timer;
     QAction m_action;
 };

 class MediaPlayer :  public QWidget
 {
     Q_OBJECT
 public:
     MediaPlayer();
     ~MediaPlayer();


     void dragEnterEvent(QDragEnterEvent *e);
     void dragMoveEvent(QDragMoveEvent *e);
     void dropEvent(QDropEvent *e);
     void handleDrop(QDropEvent *e);
     void setFile(const QString &text);
     //void setLocation(const QString &location);
     void initVideoWindow();
     void initSettingsDialog();
     void setVolume(qreal volume);
     void setSmallScreen(bool smallScreen);


 public slots:
     //void openFile();
     void rewind();
     void forward();
     void updateInfo();
     void updateTime();
     void finished();
     void playPause();
     void scaleChanged(QAction *);
     void aspectChanged(QAction *);

 private slots:
     void setAspect(int);
     void setScale(int);
     void setSaturation(int);
     void setContrast(int);
     void setHue(int);
     void setBrightness(int);
     void stateChanged(Phonon::State newstate, Phonon::State oldstate);
     void effectChanged();
     void showSettingsDialog();
     void showContextMenu(const QPoint& point);
     void bufferStatus(int percent);
     //void openUrl();

     void channelClicked();

     //void openRamFile();
     void configureEffect();
     void hasVideoChanged(bool);



 private:
     bool playPauseForDialog();
     void Espera (QString cadena);

     QToolButton *createChannel(const QString &text, const QIcon &icon, const char *member);

     QMenu *fileMenu;

     Phonon::VolumeSlider *volume;
     QSlider *m_hueSlider;
     QSlider *m_satSlider;
     QSlider *m_contSlider;
     QLabel *info;
     Phonon::Effect *nextEffect;
     QDialog *settingsDialog;
     Ui_settings *ui;
     QAction *m_fullScreenAction;

     QWidget m_videoWindow;
     Phonon::MediaObject m_MediaObject;
     Phonon::AudioOutput m_AudioOutput;
     MediaVideoWidget *m_videoWidget;
     Phonon::Path m_audioOutputPath;
     bool m_smallScreen;

     QProcess *process=NULL;
     //QTemporaryFile *flujo=NULL;

     const char *fifo = "/tmp/iTV";
     QString channel;

     QScrollArea *scrollArea;
     QWidget *scrollAreaWidgetContents;

     QButtonGroup *buttons;

     //QToolButton *la1;
     //QToolButton *la2;
     //QToolButton *la24h;
     QToolButton *a3;
     QToolButton *lasexta;
     QToolButton *lasexta3;
     QToolButton *neox;
     QToolButton *nitro;
     QToolButton *xplora;

     QToolButton *discovery;
     QToolButton *divinity;
     QToolButton *energy;
     QToolButton *euronews;
     QToolButton *paramount;
     QToolButton *rt;


     QToolButton *canalsur;
     QToolButton *la13tv;
     QToolButton *aljazeera;
     QToolButton *panamericana;
     QToolButton *globaltv;
     QToolButton *kisstv;
     QToolButton *partytv;
     QToolButton *lobastv;

     QMovie *espera;

 };

 #endif //MEDIAPLAYER_H 
