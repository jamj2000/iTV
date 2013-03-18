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
 #include "librtmp/rtmp.h"

#include <QTemporaryFile>
#include <QToolButton>
#include <QMovie>

 #define SLIDER_RANGE 8

 #include "mediaplayer.h"
 #include "ui_settings.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>





 MediaVideoWidget::MediaVideoWidget(MediaPlayer *player, QWidget *parent) :
     Phonon::VideoWidget(parent), m_player(player), m_action(this)
 {
     m_action.setCheckable(true);
     m_action.setChecked(false);
     m_action.setShortcut(QKeySequence( Qt::AltModifier + Qt::Key_Return));
     m_action.setShortcutContext(Qt::WindowShortcut);
     connect(&m_action, SIGNAL(toggled(bool)), SLOT(setFullScreen(bool)));
     addAction(&m_action);
     setAcceptDrops(true);
 }

 void MediaVideoWidget::setFullScreen(bool enabled)
 {
     Phonon::VideoWidget::setFullScreen(enabled);
     emit fullScreenChanged(enabled);
 }

 void MediaVideoWidget::mouseDoubleClickEvent(QMouseEvent *e)
 {
     Phonon::VideoWidget::mouseDoubleClickEvent(e);
     setFullScreen(!isFullScreen());
 }

 void MediaVideoWidget::keyPressEvent(QKeyEvent *e)
 {
     if(!e->modifiers()) {
         // On non-QWERTY Symbian key-based devices, there is no space key.
         // The zero key typically is marked with a space character.
         if (e->key() == Qt::Key_Space || e->key() == Qt::Key_0) {
             m_player->playPause();
             e->accept();
             return;
         }

         // On Symbian devices, there is no key which maps to Qt::Key_Escape
         // On devices which lack a backspace key (i.e. non-QWERTY devices),
         // the 'C' key maps to Qt::Key_Backspace
         else if (e->key() == Qt::Key_Escape || e->key() == Qt::Key_Backspace) {
             setFullScreen(false);
             e->accept();
             return;
         }
     }
     Phonon::VideoWidget::keyPressEvent(e);
 }

 bool MediaVideoWidget::event(QEvent *e)
 {
     switch(e->type())
     {
     case QEvent::Close:
         //we just ignore the cose events on the video widget
         //this prevents ALT+F4 from having an effect in fullscreen mode
         e->ignore();
         return true;
     case QEvent::MouseMove:
 #ifndef QT_NO_CURSOR
         unsetCursor();
 #endif
         //fall through
     case QEvent::WindowStateChange:
         {
             //we just update the state of the checkbox, in case it wasn't already
             m_action.setChecked(windowState() & Qt::WindowFullScreen);
             const Qt::WindowFlags flags = m_player->windowFlags();
             if (windowState() & Qt::WindowFullScreen) {
                 m_timer.start(1000, this);
             } else {
                 m_timer.stop();
 #ifndef QT_NO_CURSOR
                 unsetCursor();
 #endif
             }
         }
         break;
     default:
         break;
     }

     return Phonon::VideoWidget::event(e);
 }

 void MediaVideoWidget::timerEvent(QTimerEvent *e)
 {
     if (e->timerId() == m_timer.timerId()) {
         //let's store the cursor shape
 #ifndef QT_NO_CURSOR
         setCursor(Qt::BlankCursor);
 #endif
     }
     Phonon::VideoWidget::timerEvent(e);
 }

 void MediaVideoWidget::dropEvent(QDropEvent *e)
 {
     m_player->handleDrop(e);
 }

 void MediaVideoWidget::dragEnterEvent(QDragEnterEvent *e) {
     if (e->mimeData()->hasUrls())
         e->acceptProposedAction();
 }



 MediaPlayer::MediaPlayer() :
             nextEffect(0), settingsDialog(0), ui(0),
             m_AudioOutput(Phonon::VideoCategory),
             m_videoWidget(new MediaVideoWidget(this))
 {
     setWindowTitle(tr("iTV -- Reproductor de TV"));
     setContextMenuPolicy(Qt::CustomContextMenu);
     m_videoWidget->setContextMenuPolicy(Qt::CustomContextMenu);

     espera = new QMovie(":/images/wait.gif");
     scrollArea = new QScrollArea();
     scrollArea->setFixedWidth(130);
     scrollArea->setWidgetResizable(true);

     scrollAreaWidgetContents = new QWidget();

     QHBoxLayout *mainLayout = new QHBoxLayout(this);
     QVBoxLayout *channels = new QVBoxLayout(scrollAreaWidgetContents);

     //la1      = createChannel("la1",      QIcon(":/images/la1.jpg"), SLOT(channelClicked()));
     //la2      = createChannel("la2",      QIcon(":/images/la2.jpg"), SLOT(channelClicked()));
     //la24h    = createChannel("la24h",    QIcon(":/images/la24h.jpg"), SLOT(channelClicked()));
     a3       = createChannel("a3"   ,    QIcon(":/images/a3.png"), SLOT(channelClicked()));
     lasexta  = createChannel("lasexta",  QIcon(":/images/lasexta.png"), SLOT(channelClicked()));
     lasexta3 = createChannel("lasexta3", QIcon(":/images/lasexta3.png"), SLOT(channelClicked()));
     neox     = createChannel("neox",     QIcon(":/images/neox.png"), SLOT(channelClicked()));
     nitro    = createChannel("nitro",    QIcon(":/images/nitro.png"), SLOT(channelClicked()));
     xplora   = createChannel("xplora",   QIcon(":/images/xplora.png"), SLOT(channelClicked()));


     discovery= createChannel("discovery",QIcon(":/images/discovery.png"), SLOT(channelClicked()));
     divinity = createChannel("divinity", QIcon(":/images/divinity.png"), SLOT(channelClicked()));
     energy   = createChannel("energy",   QIcon(":/images/energy.png"), SLOT(channelClicked()));
     euronews = createChannel("euronews", QIcon(":/images/euronews.png"), SLOT(channelClicked()));
     paramount= createChannel("paramount",QIcon(":/images/paramount.png"), SLOT(channelClicked()));
     rt       = createChannel("rt",       QIcon(":/images/rt.png"), SLOT(channelClicked()));


     canalsur = createChannel("canalsur", QIcon(":/images/canalsur.png"), SLOT(channelClicked()));
     la13tv   = createChannel("la13tv",   QIcon(":/images/la13tv.png"), SLOT(channelClicked()));
     aljazeera= createChannel("aljazeera",QIcon(":/images/aljazeera.png"), SLOT(channelClicked()));
     panamericana= createChannel("panamericana",QIcon(":/images/panamericana.png"), SLOT(channelClicked()));
     globaltv = createChannel("globaltv",QIcon(":/images/globaltv.png"), SLOT(channelClicked()));
     kisstv   = createChannel("kisstv",QIcon(":/images/kisstv.png"), SLOT(channelClicked()));
     partytv  = createChannel("partytv",QIcon(":/images/partytv.png"), SLOT(channelClicked()));
     lobastv  = createChannel("lobastv",QIcon(":/images/lobastv.png"), SLOT(channelClicked()));

     buttons = new QButtonGroup ();

     buttons->addButton(a3);
     buttons->addButton(lasexta);
     buttons->addButton(lasexta3);
     buttons->addButton(neox);
     buttons->addButton(nitro);
     buttons->addButton(xplora);

     buttons->addButton(discovery);
     buttons->addButton(divinity);
     buttons->addButton(energy);
     buttons->addButton(euronews);
     buttons->addButton(paramount);
     buttons->addButton(rt);

     buttons->addButton(canalsur);
     buttons->addButton(la13tv);
     buttons->addButton(aljazeera);
     buttons->addButton(panamericana);
     buttons->addButton(globaltv);
     buttons->addButton(kisstv);
     buttons->addButton(partytv);
     buttons->addButton(lobastv);



     //channels->addWidget(la1);
     //channels->addWidget(la2);
     //channels->addWidget(la24h);
     channels->addWidget(a3);
     channels->addWidget(lasexta);
     channels->addWidget(lasexta3);
     channels->addWidget(neox);
     channels->addWidget(nitro);
     channels->addWidget(xplora);

     channels->addWidget(discovery);
     channels->addWidget(divinity);
     channels->addWidget(energy);
     channels->addWidget(euronews);
     channels->addWidget(paramount);
     channels->addWidget(rt);


     channels->addWidget(canalsur);
     channels->addWidget(la13tv);
     channels->addWidget(aljazeera);
     channels->addWidget(panamericana);
     channels->addWidget(globaltv);
     channels->addWidget(kisstv);
     channels->addWidget(partytv);
     channels->addWidget(lobastv);


     QPushButton *openButton = new QPushButton(this);
     openButton->setIcon(style()->standardIcon(QStyle::SP_DesktopIcon));

     QPalette bpal;
     QColor arrowcolor = bpal.buttonText().color();
     if (arrowcolor == Qt::black)
         arrowcolor = QColor(80, 80, 80);
     bpal.setBrush(QPalette::ButtonText, arrowcolor);
     openButton->setPalette(bpal);


     volume = new Phonon::VolumeSlider(&m_AudioOutput);

     QVBoxLayout *vLayout = new QVBoxLayout();


     QHBoxLayout *layout = new QHBoxLayout();


     info = new QLabel(this);
     info->setFrameShape(QFrame::NoFrame);
     info->setFrameShadow(QFrame::Plain);
     info->setMinimumSize(800,450);
     info->setMaximumSize(1600,900);
     info->setAcceptDrops(false);
     info->setMargin(2);
     //info->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
     info->setLineWidth(2);
     info->setAutoFillBackground(true);

     QPalette palette;
     palette.setBrush(QPalette::WindowText, Qt::white);

     info->setStyleSheet("border-image:url(:/images/screen.png) ; border-width:3px");
     info->setPalette(palette);
     info->setText(tr("<center>Sin video</center>"));

     volume->setFixedWidth(150);

     layout->addWidget(openButton);
     layout->addStretch();
     layout->addWidget(volume);

     QVBoxLayout *buttonPanelLayout = new QVBoxLayout();
     buttonPanelLayout->addLayout(layout);
     buttonPanelLayout->setContentsMargins(0, 0, 0, 0);

     QWidget *buttonPanelWidget = new QWidget(this);
     buttonPanelWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
     buttonPanelWidget->setLayout(buttonPanelLayout);


     vLayout->addWidget(buttonPanelWidget);
     vLayout->addWidget(info);
     initVideoWindow();
     vLayout->addWidget(&m_videoWindow);
     m_videoWindow.hide();


     scrollArea->setWidget(scrollAreaWidgetContents);

     mainLayout->addWidget(scrollArea);
     //mainLayout->addStretch();
     mainLayout->addLayout(vLayout);


     setLayout(mainLayout);


     // Create menu bar:
     fileMenu = new QMenu(this);

     QMenu *aspectMenu = fileMenu->addMenu(tr("&Aspecto"));
     QActionGroup *aspectGroup = new QActionGroup(aspectMenu);
     connect(aspectGroup, SIGNAL(triggered(QAction*)), this, SLOT(aspectChanged(QAction*)));
     aspectGroup->setExclusive(true);
     QAction *aspectActionAuto = aspectMenu->addAction(tr("Auto"));
     aspectActionAuto->setCheckable(true);
     aspectActionAuto->setChecked(true);
     aspectGroup->addAction(aspectActionAuto);
     QAction *aspectActionScale = aspectMenu->addAction(tr("Escalado"));
     aspectActionScale->setCheckable(true);
     aspectGroup->addAction(aspectActionScale);
     QAction *aspectAction16_9 = aspectMenu->addAction(tr("16/9"));
     aspectAction16_9->setCheckable(true);
     aspectGroup->addAction(aspectAction16_9);
     QAction *aspectAction4_3 = aspectMenu->addAction(tr("4/3"));
     aspectAction4_3->setCheckable(true);
     aspectGroup->addAction(aspectAction4_3);

     QMenu *scaleMenu = fileMenu->addMenu(tr("&Modo de escalado"));
     QActionGroup *scaleGroup = new QActionGroup(scaleMenu);
     connect(scaleGroup, SIGNAL(triggered(QAction*)), this, SLOT(scaleChanged(QAction*)));
     scaleGroup->setExclusive(true);
     QAction *scaleActionFit = scaleMenu->addAction(tr("Ajustar"));
     scaleActionFit->setCheckable(true);
     scaleActionFit->setChecked(true);
     scaleGroup->addAction(scaleActionFit);
     QAction *scaleActionCrop = scaleMenu->addAction(tr("Escalar y recortar"));
     scaleActionCrop->setCheckable(true);
     scaleGroup->addAction(scaleActionCrop);

     m_fullScreenAction = fileMenu->addAction(tr("Pantalla completa"));
     m_fullScreenAction->setCheckable(true);
     m_fullScreenAction->setEnabled(false); // enabled by hasVideoChanged
     bool b = connect(m_fullScreenAction, SIGNAL(toggled(bool)), m_videoWidget, SLOT(setFullScreen(bool)));
     Q_ASSERT(b);
     b = connect(m_videoWidget, SIGNAL(fullScreenChanged(bool)), m_fullScreenAction, SLOT(setChecked(bool)));
     Q_ASSERT(b);

     fileMenu->addSeparator();
     QAction *settingsAction = fileMenu->addAction(tr("&Opciones"));

     // Setup signal connections:
        openButton->setMenu(fileMenu);

     connect(settingsAction, SIGNAL(triggered(bool)), this, SLOT(showSettingsDialog()));

     connect(m_videoWidget, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(showContextMenu(const QPoint &)));
     connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(showContextMenu(const QPoint &)));
     connect(&m_MediaObject, SIGNAL(metaDataChanged()), this, SLOT(updateInfo()));
     connect(&m_MediaObject, SIGNAL(totalTimeChanged(qint64)), this, SLOT(updateTime()));
     connect(&m_MediaObject, SIGNAL(tick(qint64)), this, SLOT(updateTime()));
     connect(&m_MediaObject, SIGNAL(finished()), this, SLOT(finished()));
     connect(&m_MediaObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)), this, SLOT(stateChanged(Phonon::State,Phonon::State)));
     connect(&m_MediaObject, SIGNAL(bufferStatus(int)), this, SLOT(bufferStatus(int)));
     connect(&m_MediaObject, SIGNAL(hasVideoChanged(bool)), this, SLOT(hasVideoChanged(bool)));


     setAcceptDrops(true);

     m_audioOutputPath = Phonon::createPath(&m_MediaObject, &m_AudioOutput);
     Phonon::createPath(&m_MediaObject, m_videoWidget);

     resize(minimumSizeHint());
 }


 MediaPlayer::~MediaPlayer() {
     if (process) {
        process->kill();
        delete process;
     }

     if (fifo)
         unlink (fifo);

 }



 void MediaPlayer::stateChanged(Phonon::State newstate, Phonon::State oldstate)
 {
     Q_UNUSED(oldstate);

     if (oldstate == Phonon::LoadingState) {
         QRect videoHintRect = QRect(QPoint(0, 0), m_videoWindow.sizeHint());
         QRect newVideoRect = QApplication::desktop()->screenGeometry().intersected(videoHintRect);
         if (!m_smallScreen) {
             if (m_MediaObject.hasVideo()) {
                 // Flush event que so that sizeHint takes the
                 // recently shown/hidden m_videoWindow into account:
                 qApp->processEvents();
                 resize(sizeHint());
             } else
                 resize(minimumSize());
         }
     }

     switch (newstate) {
         case Phonon::ErrorState:
             if (m_MediaObject.errorType() == Phonon::FatalError) {

             } else {
                 m_MediaObject.pause();
             }
             QMessageBox::warning(this, "Phonon Mediaplayer", m_MediaObject.errorString(), QMessageBox::Close);
             break;

         case Phonon::StoppedState:
             m_videoWidget->setFullScreen(false);
             // Fall through

         case Phonon::PausedState:
             break;
         case Phonon::PlayingState:
             if (m_MediaObject.hasVideo())
                 m_videoWindow.show();
             // Fall through
         case Phonon::BufferingState:
             break;
         case Phonon::LoadingState:
             break;
     }

 }

 void MediaPlayer::initSettingsDialog()
 {
     settingsDialog = new QDialog(this);
     ui = new Ui_settings();
     ui->setupUi(settingsDialog);

     connect(ui->brightnessSlider, SIGNAL(valueChanged(int)), this, SLOT(setBrightness(int)));
     connect(ui->hueSlider, SIGNAL(valueChanged(int)), this, SLOT(setHue(int)));
     connect(ui->saturationSlider, SIGNAL(valueChanged(int)), this, SLOT(setSaturation(int)));
     connect(ui->contrastSlider , SIGNAL(valueChanged(int)), this, SLOT(setContrast(int)));
     connect(ui->aspectCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setAspect(int)));
     connect(ui->scalemodeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setScale(int)));

     ui->brightnessSlider->setValue(int(m_videoWidget->brightness() * SLIDER_RANGE));
     ui->hueSlider->setValue(int(m_videoWidget->hue() * SLIDER_RANGE));
     ui->saturationSlider->setValue(int(m_videoWidget->saturation() * SLIDER_RANGE));
     ui->contrastSlider->setValue(int(m_videoWidget->contrast() * SLIDER_RANGE));
     ui->aspectCombo->setCurrentIndex(m_videoWidget->aspectRatio());
     ui->scalemodeCombo->setCurrentIndex(m_videoWidget->scaleMode());
     connect(ui->effectButton, SIGNAL(clicked()), this, SLOT(configureEffect()));

 #ifdef Q_WS_X11
     //Cross fading is not currently implemented in the GStreamer backend
     ui->crossFadeSlider->setVisible(false);
     ui->crossFadeLabel->setVisible(false);
     ui->crossFadeLabel1->setVisible(false);
     ui->crossFadeLabel2->setVisible(false);
     ui->crossFadeLabel3->setVisible(false);
 #endif
     ui->crossFadeSlider->setValue((int)(2 * m_MediaObject.transitionTime() / 1000.0f));

     // Insert audio devices:
     QList<Phonon::AudioOutputDevice> devices = Phonon::BackendCapabilities::availableAudioOutputDevices();
     for (int i=0; i<devices.size(); i++){
         QString itemText = devices[i].name();
         if (!devices[i].description().isEmpty()) {
             itemText += QString::fromLatin1(" (%1)").arg(devices[i].description());
         }
         ui->deviceCombo->addItem(itemText);
         if (devices[i] == m_AudioOutput.outputDevice())
             ui->deviceCombo->setCurrentIndex(i);
     }

     // Insert audio effects:
     ui->audioEffectsCombo->addItem(tr("<sin efecto>"));
     QList<Phonon::Effect *> currEffects = m_audioOutputPath.effects();
     Phonon::Effect *currEffect = currEffects.size() ? currEffects[0] : 0;
     QList<Phonon::EffectDescription> availableEffects = Phonon::BackendCapabilities::availableAudioEffects();
     for (int i=0; i<availableEffects.size(); i++){
         ui->audioEffectsCombo->addItem(availableEffects[i].name());
         if (currEffect && availableEffects[i] == currEffect->description())
             ui->audioEffectsCombo->setCurrentIndex(i+1);
     }
     connect(ui->audioEffectsCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(effectChanged()));

 }

 void MediaPlayer::setVolume(qreal volume)
 {
     m_AudioOutput.setVolume(volume);
 }

 void MediaPlayer::setSmallScreen(bool smallScreen)
 {
     m_smallScreen = smallScreen;
 }

 void MediaPlayer::effectChanged()
 {
     int currentIndex = ui->audioEffectsCombo->currentIndex();
     if (currentIndex) {
         QList<Phonon::EffectDescription> availableEffects = Phonon::BackendCapabilities::availableAudioEffects();
         Phonon::EffectDescription chosenEffect = availableEffects[currentIndex - 1];

         QList<Phonon::Effect *> currEffects = m_audioOutputPath.effects();
         Phonon::Effect *currentEffect = currEffects.size() ? currEffects[0] : 0;

         // Deleting the running effect will stop playback, it is deleted when removed from path
         if (nextEffect && !(currentEffect && (currentEffect->description().name() == nextEffect->description().name())))
             delete nextEffect;

         nextEffect = new Phonon::Effect(chosenEffect);
     }
     ui->effectButton->setEnabled(currentIndex);
 }

 void MediaPlayer::showSettingsDialog()
 {
     const bool hasPausedForDialog = playPauseForDialog();

     if (!settingsDialog)
         initSettingsDialog();

     float oldBrightness = m_videoWidget->brightness();
     float oldHue = m_videoWidget->hue();
     float oldSaturation = m_videoWidget->saturation();
     float oldContrast = m_videoWidget->contrast();
     Phonon::VideoWidget::AspectRatio oldAspect = m_videoWidget->aspectRatio();
     Phonon::VideoWidget::ScaleMode oldScale = m_videoWidget->scaleMode();
     int currentEffect = ui->audioEffectsCombo->currentIndex();
     settingsDialog->exec();

     if (settingsDialog->result() == QDialog::Accepted){
         m_MediaObject.setTransitionTime((int)(1000 * float(ui->crossFadeSlider->value()) / 2.0f));
         QList<Phonon::AudioOutputDevice> devices = Phonon::BackendCapabilities::availableAudioOutputDevices();
         m_AudioOutput.setOutputDevice(devices[ui->deviceCombo->currentIndex()]);
         QList<Phonon::Effect *> currEffects = m_audioOutputPath.effects();
         QList<Phonon::EffectDescription> availableEffects = Phonon::BackendCapabilities::availableAudioEffects();

         if (ui->audioEffectsCombo->currentIndex() > 0){
             Phonon::Effect *currentEffect = currEffects.size() ? currEffects[0] : 0;
             if (!currentEffect || currentEffect->description() != nextEffect->description()){
                 foreach(Phonon::Effect *effect, currEffects) {
                     m_audioOutputPath.removeEffect(effect);
                     delete effect;
                 }
                 m_audioOutputPath.insertEffect(nextEffect);
             }
         } else {
             foreach(Phonon::Effect *effect, currEffects) {
                 m_audioOutputPath.removeEffect(effect);
                 delete effect;
                 nextEffect = 0;
             }
         }
     } else {
         // Restore previous settings
         m_videoWidget->setBrightness(oldBrightness);
         m_videoWidget->setSaturation(oldSaturation);
         m_videoWidget->setHue(oldHue);
         m_videoWidget->setContrast(oldContrast);
         m_videoWidget->setAspectRatio(oldAspect);
         m_videoWidget->setScaleMode(oldScale);
         ui->audioEffectsCombo->setCurrentIndex(currentEffect);
     }

     if (hasPausedForDialog)
         m_MediaObject.play();
 }

 void MediaPlayer::initVideoWindow()
 {
     QVBoxLayout *videoLayout = new QVBoxLayout();
     videoLayout->addWidget(m_videoWidget);
     videoLayout->setContentsMargins(0, 0, 0, 0);
     m_videoWindow.setLayout(videoLayout);
     m_videoWindow.setMinimumSize(100, 100);
 }

 void MediaPlayer::configureEffect()
 {
     if (!nextEffect)
         return;

     QList<Phonon::Effect *> currEffects = m_audioOutputPath.effects();
     const QList<Phonon::EffectDescription> availableEffects = Phonon::BackendCapabilities::availableAudioEffects();
     if (ui->audioEffectsCombo->currentIndex() > 0) {
         Phonon::EffectDescription chosenEffect = availableEffects[ui->audioEffectsCombo->currentIndex() - 1];

         QDialog effectDialog;
         effectDialog.setWindowTitle(tr("Configurar efecto"));
         QVBoxLayout *topLayout = new QVBoxLayout(&effectDialog);

         QLabel *description = new QLabel("<b>Description:</b><br>" + chosenEffect.description(), &effectDialog);
         description->setWordWrap(true);
         topLayout->addWidget(description);

         QScrollArea *scrollArea = new QScrollArea(&effectDialog);
         topLayout->addWidget(scrollArea);

         QVariantList savedParamValues;
         foreach(Phonon::EffectParameter param, nextEffect->parameters()) {
             savedParamValues << nextEffect->parameterValue(param);
         }

         QWidget *scrollWidget = new Phonon::EffectWidget(nextEffect);
         scrollWidget->setMinimumWidth(320);
         scrollWidget->setContentsMargins(10, 10, 10,10);
         scrollArea->setWidget(scrollWidget);

         QDialogButtonBox *bbox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &effectDialog);
         connect(bbox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), &effectDialog, SLOT(accept()));
         connect(bbox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), &effectDialog, SLOT(reject()));
         topLayout->addWidget(bbox);

         effectDialog.exec();

         if (effectDialog.result() != QDialog::Accepted) {
             //we need to restore the parameters values
             int currentIndex = 0;
             foreach(Phonon::EffectParameter param, nextEffect->parameters()) {
                 nextEffect->setParameterValue(param, savedParamValues.at(currentIndex++));
             }

         }
     }
 }

 void MediaPlayer::handleDrop(QDropEvent *e)
 {
     QList<QUrl> urls = e->mimeData()->urls();
     if (e->proposedAction() == Qt::MoveAction){
         // Just add to the queue:
         for (int i=0; i<urls.size(); i++)
             m_MediaObject.enqueue(Phonon::MediaSource(urls[i].toLocalFile()));
     } else {
         // Create new queue:
         m_MediaObject.clearQueue();
         if (urls.size() > 0) {
             QString fileName = urls[0].toLocalFile();
             QDir dir(fileName);
             if (dir.exists()) {
                 dir.setFilter(QDir::Files);
                 QStringList entries = dir.entryList();
                 if (entries.size() > 0) {
                     setFile(fileName + QDir::separator() +  entries[0]);
                     for (int i=1; i< entries.size(); ++i)
                         m_MediaObject.enqueue(fileName + QDir::separator() + entries[i]);
                 }
             } else {
                 setFile(fileName);
                 for (int i=1; i<urls.size(); i++)
                     m_MediaObject.enqueue(Phonon::MediaSource(urls[i].toLocalFile()));
             }
         }
     }

     m_MediaObject.play();
 }

 void MediaPlayer::dropEvent(QDropEvent *e)
 {
     if (e->mimeData()->hasUrls() && e->proposedAction() != Qt::LinkAction) {
         e->acceptProposedAction();
         handleDrop(e);
     } else {
         e->ignore();
     }
 }

 void MediaPlayer::dragEnterEvent(QDragEnterEvent *e)
 {
     dragMoveEvent(e);
 }

 void MediaPlayer::dragMoveEvent(QDragMoveEvent *e)
 {
     if (e->mimeData()->hasUrls()) {
         if (e->proposedAction() == Qt::CopyAction || e->proposedAction() == Qt::MoveAction){
             e->acceptProposedAction();
         }
     }
 }

 void MediaPlayer::playPause()
 {
     if (m_MediaObject.state() == Phonon::PlayingState)
         m_MediaObject.pause();
     else {
         if (m_MediaObject.currentTime() == m_MediaObject.totalTime())
             m_MediaObject.seek(0);
         m_MediaObject.play();

     }
 }



 void MediaPlayer::setFile(const QString &fileName)
 {
     setWindowTitle(fileName.right(fileName.length() - fileName.lastIndexOf('/') - 1));
     m_MediaObject.setCurrentSource(Phonon::MediaSource(fileName));
     m_MediaObject.play();
 }


 ///////////////

void MediaPlayer::Espera (QString cadena){
    m_videoWindow.hide();
    espera->start();
    info->setMovie(espera);
    info->setAlignment(Qt::AlignRight|Qt::AlignBottom);
    info->setStyleSheet(QString("background-position: center; background-image: url(:/images/%1.png);background-attachment: fixed; background-repeat: no-repeat; ").arg(cadena));
    info->show();
}


void MediaPlayer::channelClicked(){
    QToolButton *clickedButton = qobject_cast<QToolButton *>(sender());
    QString text = clickedButton->text();
    qDebug() << text;

    if (m_MediaObject.state() == Phonon::PlayingState)
        m_MediaObject.pause();
        //   m_MediaObject.clear();

    if (process) {
        process->close();
        //process->kill();
        delete process;
    }
    unlink(fifo);


    QString comando="";
    int err = mkfifo (fifo, 0777);



    //if (text=="la1")
    //    comando="rtmpdump -r rtmp://cp68975.live.edgefcs.net:1935/live --playpath LA1_AKA_WEB_NOG@58877 -W http://www.rtve.es/swf/4.1.11/RTVEPlayerVideo.swf -p http://www.rtve.es/noticias/directo-la-1 -t rtmp://cp68975.live.edgefcs.net:1935/live -v -q -o /tmp/iTV";
    //else if (text=="la2")
    //    comando="rtmpdump -r rtmp://cp68975.live.edgefcs.net:1935/live --playpath LA2_AKA_WEB_NOG@60554 -W http://www.rtve.es/swf/4.1.11/RTVEPlayerVideo.swf -p http://www.rtve.es/television/la-2-directo -t rtmp://cp68975.live.edgefcs.net:1935/live -q -v -o /tmp/iTV";
    //else if (text=="la24h")
    //    comando="
    Espera (text);
    if (text=="a3")
        comando="rtmpdump -r rtmp://antena3fms35livefs.fplive.net:1935/antena3fms35live-live --playpath stream-antena3 -W http://www.antena3.com/static/swf/A3Player.swf?nocache=200 -p http://www.antena3.com/directo/ -q -v -o /tmp/iTV";    
    else if (text=="lasexta")
        comando="rtmpdump -r rtmp://antena3fms35livefs.fplive.net:1935/antena3fms35live-live/stream-lasexta -W http://www.antena3.com/static/swf/A3Player.swf -p http://www.lasexta.com/directo -q -v -o /tmp/iTV";
    else if (text=="lasexta3")
        comando="rtmpdump -r rtmp://174.36.251.140/live/lasexta3lacaja?id=15912 -W http://www.ucaster.eu/static/scripts/eplayer.swf -p http:schuster92.com -q -v -o /tmp/iTV";
    else if (text=="neox")
        comando="rtmpdump -r rtmp://live.zcast.us:1935/liveorigin/_definst_ --playpath neoxlacaja-lI7mjw6RDa -W http://player.zcast.us/player58.swf -p http://zcast.us/gen.php?ch=neoxlacaja-lI7mjw6RDa&width=670&height=400 -q -v -o /tmp/iTV";
    else if (text=="nitro")
        comando="rtmpdump -r rtmp://173.193.242.248/live --playpath nitrolacajatv?id=126587 -W http://mips.tv/content/scripts/eplayer.swf -p http://mips.tv/embedplayer/nitrolacajatv/1/670/400 -q -v -o /tmp/iTV";
    else if (text=="xplora")
        comando="rtmpdump -r rtmp://antena3fms35geobloqueolivefs.fplive.net:1935/antena3fms35geobloqueolive-live/stream-xplora -W http://www.antena3.com/static/swf/A3Player.swf -p http://www.lasexta.com/xplora/directo -q -v -o /tmp/iTV";

    else if (text=="discovery")
        comando="rtmpdump -r rtmp://184.173.181.44/live --playpath discoverylacajatv?id=14680 -W http://www.ucaster.eu/static/scripts/eplayer.swf -p http://www.ucaster.eu/embedded/discoverylacajatv/1/650/400 -q -v -o /tmp/iTV";
    else if (text=="divinity")
        comando="rtmpdump -r rtmp://68.68.17.102/live --playpath discomaxlacajatv -W http://www.udemy.com/static/flash/player5.9.swf -p http://www.castamp.com/embed.php?c=discomaxlacajatv&tk=5mD8Tatf&vwidth=650&vheight=400 -q -v -o /tmp/iTV";
    else if (text=="energy")
        comando="rtmpdump -r rtmp://68.68.31.224/live --playpath lacajatvenergy -W http://www.udemy.com/static/flash/player5.9.swf -p http://www.castamp.com/embed.php?c=lacajatvenergy&vwidth=670&vheight=400 -q -v -o /tmp/iTV";
    else if (text=="euronews")
        comando="rtmpdump -r rtmp://fr-par-1.stream-relay.hexaglobe.net:1935/rtpeuronewslive --playpath es_video350_flash_all.sdp -W http://es.euronews.com/media/player_live_1_14.swf -p http://es.euronews.com/noticias/en-directo/ -q -v -o /tmp/iTV";
    else if (text=="paramount")
        comando="rtmpdump -r rtmp://173.193.46.109/live --playpath 179582 -W http://static.castalba.tv/player.swf -p http://castalba.tv/embed.php?cid=9947&wh=680&ht=400&r=lacajatv.es -q -v -o /tmp/iTV";
    else if (text=="rt")
        comando="rtmpdump -r rtmp://149.11.34.6/live --playpath russiantoday.stream -q -v -o /tmp/iTV";

    else if (text=="canalsur")
        comando="wget --quiet -O /tmp/iTV http://195.10.10.220/rtva/andaluciatelevisionh264.flv";
    else if (text=="la13tv")
        comando="rtmpdump -r rtmp://xiiitvlivefs.fplive.net/xiiitvlive-live --playpath stream13tv -W http://static.hollybyte.com/public/players/flowplayer/swf/flowplayer.commercial.swf -p http://live.13tv.hollybyte.tv/embed/4f33a91894a05f5f49020000 -q -v -o /tmp/iTV";
    else if (text=="aljazeera")
        comando="rtmpdump -r rtmp://aljazeeraflashlivefs.fplive.net:1935/aljazeeraflashlive-live --playpath aljazeera_eng_high -W http://admin.brightcove.com/viewer/us20121113.1511/federatedVideoUI/BrightcovePlayer.swf -p http://www.aljazeera.com/watch_now/ -q -v -o /tmp/iTV";
    else if (text=="panamericana")
        comando="rtmpdump -r rtmp://demo5.iblups.com/demo --playpath nm5esQgmkT  -W http://iblups.com/playertvlive123456789panamericanatv.swf -p http://iblups.com/e_panamericanatv-490-320 -q -v -o /tmp/iTV";
    else if (text=="globaltv")
        comando="rtmpdump -r rtmp://demo13.iblups.com/demo --playpath hTWNttHSsq -W http://iblups.com/playertvlive123456789globaltv.swf -p http://iblups.com/e_globaltv-490-33 -q -v -o /tmp/iTV";
    else if (text=="kisstv")
        comando="rtmpdump -r rtmp://kisstelevision.es.flash3.glb.ipercast.net/kisstelevision.es-live --playpath live -W http://kisstelevision.en-directo.com/kisstelevision_avw.swf -p http://www.kisstelevision.es -q -v -o /tmp/iTV";
    else if (text=="lobastv")
        comando="rtmpdump -r rtmp://149.11.34.6/live --playpath lobas.stream -q -v -o /tmp/iTV";
    else if (text=="partytv")
        comando="rtmpdump -r rtmp://149.11.34.6/live --playpath partytv.stream -q -v -o /tmp/iTV";
    else
        qDebug () << "Cadena " << text << " no reconocida";


    //comando.append (" /tmp/iTV");

    if (comando!="") {
        process = new QProcess();
        process->start(comando);
        qDebug() << comando;
    }

    m_MediaObject.setCurrentSource(Phonon::MediaSource(QString("file:///tmp/iTV")));
    m_MediaObject.play();


}



QToolButton *MediaPlayer::createChannel(const QString &text,const QIcon &icon, const char *member) {
    QToolButton *button = new QToolButton(scrollAreaWidgetContents);
    button->setFixedSize(100,100);
    button->setIcon(icon);
    button->setText(text);
    button->setIconSize(QSize(90,90));
    button->setCheckable(true);
    button->setStyleSheet("QToolButton:checked {border: 2px solid #ffffff; background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1,  stop: 0 #ffffff, stop: 1 #5555ff);}");

    connect(button, SIGNAL(clicked()), this, member);
    return button;
}


/*
 void MediaPlayer::setLocation(const QString& location)
 {
     if (process) {
         process->kill();
         delete process;
     }
     unlink(fifo);
     if (flujo){
         flujo->close();
         delete flujo;
     }


     flujo = new QTemporaryFile();
     if (m_MediaObject.state() == Phonon::PlayingState)
         m_MediaObject.pause();


    process = new QProcess();

     int err = mkfifo (fifo, 0777);
     //setWindowTitle(location.right(location.length() - location.lastIndexOf('/') - 1));
     //m_MediaObject.setCurrentSource(Phonon::MediaSource(QUrl::fromEncoded(location.toUtf8())));
     //m_MediaObject.play();


     flujo->open();
     QString comando="rtmpdump -r rtmp://antena3fms35livefs.fplive.net:1935/antena3fms35live-live --playpath stream-antena3 -W http://www.antena3.com/static/swf/A3Player.swf?nocache=200 -p http://www.antena3.com/directo/ -q -v -o ";
     //comando.append (flujo->fileName());
     comando.append ("/tmp/caca");
     process->start(comando);


     qDebug() << comando;

     ////
     // QProcess::execute("rtmpdump -r rtmp://149.11.34.6/live --playpath russiantoday.stream -q -v -o /tmp/caca");
     //process->start(comando);
     //process.start("rtmpdump -r rtmp://cp68975.live.edgefcs.net:1935/live --playpath LA1_AKA_WEB_NOG@58877 -W http://www.rtve.es/swf/4.1.11/RTVEPlayerVideo.swf -p http://www.rtve.es/noticias/directo-la-1 -t rtmp://cp68975.live.edgefcs.net:1935/live -v -q -o /tmp/caca");
     //process.start("rtmpdump -r rtmp://cp68975.live.edgefcs.net:1935/live --playpath LA2_AKA_WEB_NOG@60554 -W http://www.rtve.es/swf/4.1.11/RTVEPlayerVideo.swf -p http://www.rtve.es/television/la-2-directo -t rtmp://cp68975.live.edgefcs.net:1935/live -q -v -o /tmp/caca");
     //process.start("rtmpdump -r rtmp://antena3fms35livefs.fplive.net:1935/antena3fms35live-live --playpath stream-antena3 -W http://www.antena3.com/static/swf/A3Player.swf?nocache=200 -p http://www.antena3.com/directo/ -q -v -o /tmp/caca");
     //process.start("rtmpdump -r rtmp://149.11.34.6/live --playpath russiantoday.stream -q -v -o /tmp/caca");
     //m_MediaObject.setCurrentSource(Phonon::MediaSource(QString("file:///tmp/caca")));
     //QResource res(":/images/wait.flv");

       m_MediaObject.setCurrentSource(Phonon::MediaSource(QString("file:///tmp/caca")));
       m_MediaObject.play();
 }
*/



//////////////////////

 bool MediaPlayer::playPauseForDialog()
 {
     // If we're running on a small screen, we want to pause the video when
     // popping up dialogs. We neither want to tamper with the state if the
     // user has paused.
     if (m_smallScreen && m_MediaObject.hasVideo()) {
         if (Phonon::PlayingState == m_MediaObject.state()) {
             m_MediaObject.pause();
             return true;
         }
     }
     return false;
 }

 /*
 void MediaPlayer::openFile()
 {
     const bool hasPausedForDialog = playPauseForDialog();

     QStringList fileNames = QFileDialog::getOpenFileNames(this, QString(),
                                                           QDesktopServices::storageLocation(QDesktopServices::MusicLocation));

     if (hasPausedForDialog)
         m_MediaObject.play();

     m_MediaObject.clearQueue();
     if (fileNames.size() > 0) {
         QString fileName = fileNames[0];
         setFile(fileName);
         for (int i=1; i<fileNames.size(); i++)
             m_MediaObject.enqueue(Phonon::MediaSource(fileNames[i]));
     }

 }
*/

 void MediaPlayer::bufferStatus(int percent)
 {
     if (percent != 100){
             QString str = QString::fromLatin1("(%1%)").arg(percent);

     }
 }

 void MediaPlayer::setSaturation(int val)
 {
     m_videoWidget->setSaturation(val / qreal(SLIDER_RANGE));
 }

 void MediaPlayer::setHue(int val)
 {
     m_videoWidget->setHue(val / qreal(SLIDER_RANGE));
 }

 void MediaPlayer::setAspect(int val)
 {
     m_videoWidget->setAspectRatio(Phonon::VideoWidget::AspectRatio(val));
 }

 void MediaPlayer::setScale(int val)
 {
     m_videoWidget->setScaleMode(Phonon::VideoWidget::ScaleMode(val));
 }

 void MediaPlayer::setBrightness(int val)
 {
     m_videoWidget->setBrightness(val / qreal(SLIDER_RANGE));
 }

 void MediaPlayer::setContrast(int val)
 {
     m_videoWidget->setContrast(val / qreal(SLIDER_RANGE));
 }

 void MediaPlayer::updateInfo()
 {
     /*
     int maxLength = 30;
     QString font = "<font color=#ffeeaa>";
     QString fontmono = "<font family=\"monospace,courier new\" color=#ffeeaa>";

     QMap <QString, QString> metaData = m_MediaObject.metaData();
     QString trackArtist = metaData.value("ARTIST");
     if (trackArtist.length() > maxLength)
         trackArtist = trackArtist.left(maxLength) + "...";

     QString trackTitle = metaData.value("TITLE");
     int trackBitrate = metaData.value("BITRATE").toInt();

     QString fileName;
     if (m_MediaObject.currentSource().type() == Phonon::MediaSource::Url) {
         fileName = m_MediaObject.currentSource().url().toString();
     } else {
         fileName = m_MediaObject.currentSource().fileName();
         fileName = fileName.right(fileName.length() - fileName.lastIndexOf('/') - 1);
         if (fileName.length() > maxLength)
             fileName = fileName.left(maxLength) + "...";
     }

     QString title;
     if (!trackTitle.isEmpty()) {
         if (trackTitle.length() > maxLength)
             trackTitle = trackTitle.left(maxLength) + "...";
         title = "Title: " + font + trackTitle + "<br></font>";
     } else if (!fileName.isEmpty()) {
         if (fileName.length() > maxLength)
             fileName = fileName.left(maxLength) + "...";
         title = font + fileName + "</font>";

         if (m_MediaObject.currentSource().type() == Phonon::MediaSource::Url) {
             title.prepend("Url: ");
         } else {
             title.prepend("File: ");
         }

     }

     QString artist;
     if (!trackArtist.isEmpty())
         artist = "Artist:  " + font + trackArtist + "</font>";

     QString bitrate;
     if (trackBitrate != 0)
         bitrate = "<br>Bitrate:  " + font + QString::number(trackBitrate/1000) + "kbit</font>";

     */

     //info->setText(title + artist + bitrate);
 }

 void MediaPlayer::updateTime()
 {
     long len = m_MediaObject.totalTime();
     long pos = m_MediaObject.currentTime();
     QString timeString;
     if (pos || len)
     {
         int sec = pos/1000;
         int min = sec/60;
         int hour = min/60;
         int msec = pos;

         QTime playTime(hour%60, min%60, sec%60, msec%1000);
         sec = len / 1000;
         min = sec / 60;
         hour = min / 60;
         msec = len;

         QTime stopTime(hour%60, min%60, sec%60, msec%1000);
         QString timeFormat = "m:ss";
         if (hour > 0)
             timeFormat = "h:mm:ss";
         timeString = playTime.toString(timeFormat);
         if (len)
             timeString += " / " + stopTime.toString(timeFormat);
     }

 }

 void MediaPlayer::rewind()
 {
     m_MediaObject.seek(0);
 }

 void MediaPlayer::forward()
 {
     QList<Phonon::MediaSource> queue = m_MediaObject.queue();
     if (queue.size() > 0) {
         m_MediaObject.setCurrentSource(queue[0]);

         m_MediaObject.play();
     }
 }

 /*
 void MediaPlayer::openUrl()
 {
     QSettings settings;
     settings.beginGroup(QLatin1String("BrowserMainWindow"));
     QString sourceURL = settings.value("location").toString();
     bool ok = false;
     sourceURL = QInputDialog::getText(this, tr("Open Location"), tr("Please enter a valid address here:"), QLineEdit::Normal, sourceURL, &ok);
     if (ok && !sourceURL.isEmpty()) {
         setLocation(sourceURL);
         settings.setValue("location", sourceURL);
     }
 }

*/


 /*!
  \since 4.6
  */
 /*
 void MediaPlayer::openRamFile()
 {
     QSettings settings;
     settings.beginGroup(QLatin1String("BrowserMainWindow"));

     const QStringList fileNameList(QFileDialog::getOpenFileNames(this,
                                                                   QString(),
                                                                   settings.value("openRamFile").toString(),
                                                                   QLatin1String("RAM files (*.ram)")));

     if (fileNameList.isEmpty())
         return;

     QFile linkFile;
     QList<QUrl> list;
     QByteArray sourceURL;
     for (int i = 0; i < fileNameList.count(); i++ ) {
         linkFile.setFileName(fileNameList[i]);
         if (linkFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
             while (!linkFile.atEnd()) {
                 sourceURL = linkFile.readLine().trimmed();
                 if (!sourceURL.isEmpty()) {
                     const QUrl url(QUrl::fromEncoded(sourceURL));
                     if (url.isValid())
                         list.append(url);
                 }
             }
             linkFile.close();
         }
     }

     if (!list.isEmpty()) {
         m_MediaObject.clearQueue();
         setLocation(list[0].toString());
         for (int i = 1; i < list.count(); i++)
             m_MediaObject.enqueue(Phonon::MediaSource(list[i]));
         m_MediaObject.play();
     }


     settings.setValue("openRamFile", fileNameList[0]);
 }

 */

 void MediaPlayer::finished()
 {
 }

 void MediaPlayer::showContextMenu(const QPoint &p)
 {
     fileMenu->popup(m_videoWidget->isFullScreen() ? p : mapToGlobal(p));
 }

 void MediaPlayer::scaleChanged(QAction *act)
 {
     if (act->text() == tr("Escalar y recortar"))
         m_videoWidget->setScaleMode(Phonon::VideoWidget::ScaleAndCrop);
     else
         m_videoWidget->setScaleMode(Phonon::VideoWidget::FitInView);
 }

 void MediaPlayer::aspectChanged(QAction *act)
 {
     if (act->text() == tr("16/9"))
         m_videoWidget->setAspectRatio(Phonon::VideoWidget::AspectRatio16_9);
     else if (act->text() == tr("Escalado"))
         m_videoWidget->setAspectRatio(Phonon::VideoWidget::AspectRatioWidget);
     else if (act->text() == tr("4/3"))
         m_videoWidget->setAspectRatio(Phonon::VideoWidget::AspectRatio4_3);
     else
         m_videoWidget->setAspectRatio(Phonon::VideoWidget::AspectRatioAuto);
 }

 void MediaPlayer::hasVideoChanged(bool bHasVideo)
 {
     info->setVisible(!bHasVideo);
     m_videoWindow.setVisible(bHasVideo);
     m_fullScreenAction->setEnabled(bHasVideo);
 }


