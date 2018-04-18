// Copyright (C) Alfabook srl
// License: http://www.gnu.org/licenses/gpl-2.0.txt GNU General Public License v2

#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#  include <QtWidgets>
#else
#  include <QtGui>
#endif

#include <QDesktopWidget>
#include <QDir>
#include <QProcess>
#include <QTimer>
#include <QFont>

#include <QDebug>

#include "player.h"

const int WIDTH = 690;
const int HEIGHT = 60;

//! [0]
Player::Player(QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowSystemMenuHint)
{
    /*QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(1000);*/

    QAction *quitAction = new QAction(tr("E&xit"), this);
    quitAction->setShortcut(tr("Ctrl+Q"));
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    addAction(quitAction);

    screenWidth = QApplication::desktop()->screenGeometry().width();
    screenHeight = QApplication::desktop()->screenGeometry().height();

    //setGeometry(QRect((screenWidth - WIDTH) / 2, (screenHeight - 50), WIDTH, HEIGHT));

    loadImages();

    setContextMenuPolicy(Qt::ActionsContextMenu);
    setToolTip(tr("Microninja Video Player"));
    setWindowTitle(tr("Microninja Video Player"));

    setMouseTracking(true);

    mousePos = NONE;
    isMoving = false;
    loopEnabled = false;

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerStatusHandler()));

    QTimer::singleShot(0, this, SLOT(startupMove()));
}

Player::~Player()
{
    if(status == PLAYING || status == PAUSED) {
        disconnect(process, SIGNAL(finished(int)), this, SLOT(deleteProcess()));

        QStringList arguments;
        arguments<<"stop";

        QProcess::execute("dbuscontrol.sh", arguments);
        process->waitForFinished(1000);
        process->deleteLater();
    }
    timer->stop();
    delete timer;
}

void Player::startupMove()
{
    move((screenWidth - WIDTH) / 2, (screenHeight - 120));
    startPos = QPoint((screenWidth - WIDTH) / 2, (screenHeight - 120));
    currentPos = startPos;
}

void Player::loadImages()
{
    play_normal = QImage(":/images/play_normal.png");
    play_light = QImage(":/images/play_light.png");
    play_disabled = QImage(":/images/play_disabled.png");

    pause_light = QImage(":/images/pause_light.png");
    pause_normal = QImage(":/images/pause_normal.png");
    pause_disabled = QImage(":/images/pause_disabled.png");

    stop_light = QImage(":/images/stop_light.png");
    stop_normal = QImage(":/images/stop_normal.png");
    stop_disabled = QImage(":/images/stop_disabled.png");

    loop_light = QImage(":/images/loop_light.png");
    loop_normal = QImage(":/images/loop_normal.png");
    loop_disabled = QImage(":/images/loop_disabled.png");
    loop_active = QImage(":/images/loop_active.png");
    loop_active_light = QImage(":/images/loop_active_light.png");

    open_light = QImage(":/images/open_light.png");
    open_normal = QImage(":/images/open_normal.png");
    open_disabled = QImage(":/images/open_disabled.png");

    fullscreen_light = QImage(":/images/fullscreen_light.png");
    fullscreen_normal = QImage(":/images/fullscreen_normal.png");
    fullscreen_disabled = QImage(":/images/fullscreen_disabled.png");

    volume_up_light = QImage(":/images/volume_up_light.png");
    volume_up_normal = QImage(":/images/volume_up_normal.png");
    volume_up_disabled = QImage(":/images/volume_up_disabled.png");

    volume_down_light = QImage(":/images/volume_down_light.png");
    volume_down_normal = QImage(":/images/volume_down_normal.png");
    volume_down_disabled = QImage(":/images/volume_down_disabled.png");

    quit_normal = QImage(":/images/quit_normal.png");
    quit_light = QImage(":/images/quit_light.png");

    play_normal = play_normal.scaledToWidth(24, Qt::SmoothTransformation);
    play_light = play_light.scaledToWidth(24, Qt::SmoothTransformation);
    play_disabled = play_disabled.scaledToWidth(24, Qt::SmoothTransformation);

    pause_normal = pause_normal.scaledToWidth(24, Qt::SmoothTransformation);
    pause_light = pause_light.scaledToWidth(24, Qt::SmoothTransformation);
    pause_disabled = pause_disabled.scaledToWidth(24, Qt::SmoothTransformation);

    stop_normal = stop_normal.scaledToWidth(24, Qt::SmoothTransformation);
    stop_light = stop_light.scaledToWidth(24, Qt::SmoothTransformation);
    stop_disabled = stop_disabled.scaledToWidth(24, Qt::SmoothTransformation);

    loop_normal = loop_normal.scaledToWidth(24, Qt::SmoothTransformation);
    loop_light = loop_light.scaledToWidth(24, Qt::SmoothTransformation);
    loop_disabled = loop_disabled.scaledToWidth(24, Qt::SmoothTransformation);
    loop_active = loop_active.scaledToWidth(24, Qt::SmoothTransformation);
    loop_active_light = loop_active_light.scaledToWidth(24, Qt::SmoothTransformation);

    quit_normal = quit_normal.scaledToWidth(24, Qt::SmoothTransformation);
    quit_light = quit_light.scaledToWidth(24, Qt::SmoothTransformation);

    open_light = open_light.scaledToWidth(24, Qt::SmoothTransformation);
    open_normal = open_normal.scaledToWidth(24, Qt::SmoothTransformation);
    open_disabled = open_disabled.scaledToWidth(24, Qt::SmoothTransformation);

    fullscreen_light = fullscreen_light.scaledToWidth(24, Qt::SmoothTransformation);
    fullscreen_normal = fullscreen_normal.scaledToWidth(24, Qt::SmoothTransformation);
    fullscreen_disabled = fullscreen_disabled.scaledToWidth(24, Qt::SmoothTransformation);

    volume_up_light = volume_up_light.scaledToWidth(24, Qt::SmoothTransformation);
    volume_up_normal = volume_up_normal.scaledToWidth(24, Qt::SmoothTransformation);
    volume_up_disabled = volume_up_disabled.scaledToWidth(24, Qt::SmoothTransformation);

    volume_down_light = volume_down_light.scaledToWidth(24, Qt::SmoothTransformation);
    volume_down_normal = volume_down_normal.scaledToWidth(24, Qt::SmoothTransformation);
    volume_down_disabled = volume_down_disabled.scaledToWidth(24, Qt::SmoothTransformation);

    status = GETTING_RES;
    process = NULL;
    fullscreen = false;

    duration = 0;
    position = 0;
}

void Player::deleteProcess()
{    
    QProcess* proc = qobject_cast<QProcess*>(sender());
    proc->deleteLater();

    if(proc == process) {
        process = NULL;

        if(status == PLAYING) {
            status = FINISHED;
            position = 0;
            timer->stop();
            //update();

            if(loopEnabled)
                play();
            else
                update();
        }
    }
}

void Player::processRes()
{
    movieW = 0;
    movieH = 0;

    resOut = resOut.replace(QString(","), QString(""));
    QStringList words = resOut.split(" ");
    foreach(QString s, words) {

        if(s.contains(videoPath))
            continue;
        if(s.contains("x")) {
            QStringList resList = s.split("x");

            if(resList.size() == 2) {
                if(resList.at(0) == "0")
                    continue;
                else {
                    movieW = resList.at(0).toInt();
                    movieH = resList.at(1).toInt();
                }
            }
        }
    }

    if((movieW > 0) && (movieH > 0) && (movieW <= 3840) && (movieW <= 2160)) {
        status = STARTING_PLAY;
        play();
    }
}

void Player::readData()
{
    resOut += process->readAllStandardError();
}

void Player::play(const QString &path)
{    
    if(path.size()) {
        mousePos = NONE;
        status = GETTING_RES;

        videoPath = path;

        /*play();
        update();
        return;*/
    }
    if(status == PAUSED) {
        QStringList arguments;
        arguments<<"pause";

        QProcess::execute("dbuscontrol.sh", arguments);
        status = PLAYING;
        timer->start();
        update();
    } else {

        QString program = "omxplayer";
        QStringList arguments;

        if(status == GETTING_RES)
        {
            position = 0;
            resOut = "";
            arguments<<"-i";
        }

        arguments<<videoPath;

        if((status == STARTING_PLAY) || (status == FINISHED) || (status == STOPPED)) {

            int availableWidth = screenWidth;
            int availableHeight = screenHeight - 130;

            sizeMovW = movieW;
            sizeMovH = movieH;

            int startPosX, startPosY;

            double ratioW = 1;
            double ratioH = 1;

            double videoRatio = 1;

            if(availableWidth < movieW) {
                ratioW = (double)movieW / (double)availableWidth;
            }
            if(availableHeight < movieH) {
                ratioH = (double)movieH / (double)availableHeight;
            }

            if(ratioW > 1 && (ratioW >= ratioH)) {
                videoRatio = ratioW;
            }
            else if(ratioH > 1 && (ratioH >= ratioW)) {
                videoRatio = ratioH;
            }

            //qDebug()<<"VIDEO RATIO: " <<videoRatio;

            if(videoRatio > 1) {
                sizeMovW = (double)movieW / videoRatio;
                sizeMovH = (double)movieH / videoRatio;
            }

            startPosX = (availableWidth - sizeMovW) / 2;
            startPosY = availableHeight - sizeMovH;

            startVideoPos = QPoint(startPosX, startPosY);

            //move video relative to win position
            startPosX = startVideoPos.x() + (currentPos.x() - startPos.x());
            startPosY = startVideoPos.y() + (currentPos.y() - startPos.y());

            int endPosX = sizeMovW + startPosX;
            int endPosY = sizeMovH + startPosY;
            //end move video

            //fullscreen = false;

            arguments<<"--win";
            if(fullscreen == false) {
                arguments<<QString::number(startPosX) + " " + QString::number(startPosY) + " " +
                           QString::number(endPosX) + " " + QString::number(endPosY);
            } else {
                startPosX = 0;
                startPosY = 0;

                endPosX = screenWidth;
                endPosY = screenHeight;

                arguments<<QString::number(startPosX) + " " + QString::number(startPosY) + " " +
                           QString::number(endPosX) + " " + QString::number(endPosY);
            }
        }

        if(process && (process->state() != QProcess::NotRunning)) {
            disconnect(process, SIGNAL(readyReadStandardError()), this, SLOT(readData()));
            disconnect(process, SIGNAL(finished(int)), this, SLOT(processRes()));
            process->terminate();
        }

        process = new QProcess(this);

        if(status == GETTING_RES) {
            connect(process, SIGNAL(readyReadStandardError()), this, SLOT(readData()));
            connect(process, SIGNAL(finished(int)), this, SLOT(processRes()));
        }
        connect(process, SIGNAL(finished(int)), this, SLOT(deleteProcess()));

        process->start(program, arguments);

        if((status == STARTING_PLAY) || (status == FINISHED) || (status == STOPPED)) {
            status = PLAYING;
            timer->start(500);
            update();
        }
    }
}

void Player::pause()
{ 
    QStringList arguments;
    arguments<<"pause";

    QProcess::execute("dbuscontrol.sh", arguments);
    status = PAUSED;
}

void Player::stop()
{
    QStringList arguments;
    arguments<<"stop";

    QProcess::execute("dbuscontrol.sh", arguments);
    timer->stop();
    status = STOPPED;
    position = 0;
}

void Player::volumeUp()
{
    QStringList arguments;
    arguments<<"volumeup";

    QProcess::execute("dbuscontrol.sh", arguments);
}

void Player::volumeDown()
{
    QStringList arguments;
    arguments<<"volumedown";

    QProcess::execute("dbuscontrol.sh", arguments);
}

void Player::readStatus() {
    QProcess* p = qobject_cast<QProcess*>(sender());
    statusString += p->readAllStandardOutput();
}

void Player::statusFinished() {
    QProcess* p = qobject_cast<QProcess*>(sender());
    disconnect(p, SIGNAL(readyReadStandardOutput()), this, SLOT(readStatus()));
    disconnect(p, SIGNAL(finished(int)), this, SLOT(statusFinished()));

    QStringList statusList = statusString.split("\n");

    QString duration;
    QString position;
    QString pause;
    if(statusList.size() >= 3) {
        QStringList ds = statusList.at(0).split(": ");
        if(ds.size() == 2) {
            duration = ds.at(1);
        }
        QStringList pos = statusList.at(1).split(": ");
        if(pos.size() == 2) {
            position = pos.at(1);
        }
        QStringList pas = statusList.at(2).split(": ");
        if(pas.size() == 2) {
            pause = pas.at(1);
        }
    }

    this->duration = duration.toLong() / 1000000;
    this->position = position.toLong() / 1000000;

    p->deleteLater();

    update();
}

void Player::timerStatusHandler()
{
    statusString = "";
    QStringList arguments;
    arguments<<"status";

    QProcess *p = new QProcess(this);

    connect(p, SIGNAL(readyReadStandardOutput()), this, SLOT(readStatus()));
    connect(p, SIGNAL(finished(int)), this, SLOT(statusFinished()));

    p->start("dbuscontrol.sh", arguments);
}

void Player::setVideoFullscreen()
{
    if(!fullscreen) {
        int startPosX = 0;
        int startPosY = 0;

        int endPosX = screenWidth;
        int endPosY = screenHeight;

        QStringList arguments;
        arguments<<"setvideopos";
        arguments<<QString::number(startPosX);
        arguments<<QString::number(startPosY);
        arguments<<QString::number(endPosX);
        arguments<<QString::number(endPosY);

        QProcess::execute("dbuscontrol.sh", arguments);
        fullscreen = true;
    } else {
        moveVideo();
        fullscreen = false;
    }

}

void Player::moveVideo()
{
    int startPosX = startVideoPos.x() + (currentPos.x() - startPos.x());
    int startPosY = startVideoPos.y() + (currentPos.y() - startPos.y());

    int endPosX = sizeMovW + startPosX;
    int endPosY = sizeMovH + startPosY;

    QStringList arguments;
    arguments<<"setvideopos";
    arguments<<QString::number(startPosX);
    arguments<<QString::number(startPosY);
    arguments<<QString::number(endPosX);
    arguments<<QString::number(endPosY);

    QProcess::execute("dbuscontrol.sh", arguments);
}

void Player::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Space)
    {
        if(status == PLAYING) {
            pause();
            update();
        }
        else if(status == PAUSED) {
            play();
            update();
        }
    }
    if(event->key() == Qt::Key_Plus)
    {
        if(status == PLAYING) {
            volumeUp();
        }
    }
    if(event->key() == Qt::Key_Minus)
    {
        if(status == PLAYING) {
            volumeDown();
        }
    }
    if((event->key() == Qt::Key_F) || (event->key() == Qt::Key_F11))
    {
        if(status == PLAYING) {
            setVideoFullscreen();
        }
    }
    if((event->key() == Qt::Key_Escape) && (fullscreen))
    {
        if(status == PLAYING) {
            setVideoFullscreen();
        }
    }
}

void Player::setLoop() {
    loopEnabled = true;
    update();
}

//! [0]

//! [1]
void Player::mousePressEvent(QMouseEvent *event)
{
    //position handle enlarged
    QRect enlarged = position_handle;
    enlarged.setHeight(position_handle.height() + 4);
    //end handle enlarged

    if((event->button() == Qt::LeftButton) && (quit_rect.contains(event->pos())))
    {
        QApplication::quit();
    }
    else if((event->button() == Qt::LeftButton) && (open_rect.contains(event->pos())))
    {
        if((status == GETTING_RES) || (status == FINISHED) || (status == STOPPED)) {
            QString fileName = QFileDialog::getOpenFileName(this,
                                                            tr("Open Video"), QDir::home().absolutePath(), tr("Video Files (*.mkv *.mp4 *.mpeg *.avi *.flv *.divx *.ogv *.webm *.3gp)"));

            mousePos = NONE;
            status = GETTING_RES;

            if(fileName.size()) {
                videoPath = fileName;
                play();
            }

            update();
        }
    }
    else if((event->button() == Qt::LeftButton) && (play_rect.contains(event->pos())))
    {
        if((status == PAUSED) || (status == FINISHED) || (status == STOPPED)) {
            play();
        }
        update();
    }
    else if((event->button() == Qt::LeftButton) && (pause_rect.contains(event->pos())))
    {
        if(status == PLAYING) {
            pause();
        }
        update();
    }
    else if((event->button() == Qt::LeftButton) && (stop_rect.contains(event->pos())))
    {
        if(status == PLAYING) {
            stop();
        }
        update();
    }
    else if((event->button() == Qt::LeftButton) && (loop_rect.contains(event->pos())))
    {
        if(status == PLAYING) {
            loopEnabled = !loopEnabled;
        }
        update();
    }
    else if((event->button() == Qt::LeftButton) && (volume_up_rect.contains(event->pos())))
    {
        if(status == PLAYING) {
            volumeUp();
        }
    }
    else if((event->button() == Qt::LeftButton) && (volume_down_rect.contains(event->pos())))
    {
        if(status == PLAYING) {
            volumeDown();
        }
    }
    else if((event->button() == Qt::LeftButton) && (fullscreen_rect.contains(event->pos())))
    {
        if(status == PLAYING) {
            setVideoFullscreen();
        }
    }
    else if((event->button() == Qt::LeftButton) && (enlarged.contains(event->pos())))
    {
        if((status == PLAYING)) {
            double percentage = (double)(event->pos().x() - position_handle.x()) / (double)position_handle.width();
            unsigned long long msecPos = percentage * duration * 1000000;

            QStringList arguments;
            arguments<<"setposition";
            arguments<<QString::number(msecPos);
            QProcess::execute("dbuscontrol.sh", arguments);
        }
    }
    else if (event->button() == Qt::LeftButton) {
        dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
        isMoving = true;
    }
}
//! [1]

//! [2]

void Player::mouseMoveEvent(QMouseEvent *event)
{
    //tooltips
    if(open_rect.contains(event->pos())) {
        setToolTip(tr("Open Video"));
    } else if(quit_rect.contains(event->pos())) {
        setToolTip(tr("Quit"));
    } else if(play_rect.contains(event->pos())) {
        setToolTip(tr("Play"));
    } else if(pause_rect.contains(event->pos())) {
        setToolTip(tr("Pause"));
    } else if(stop_rect.contains(event->pos())) {
        setToolTip(tr("Stop"));
    } else if(loop_rect.contains(event->pos())) {
        setToolTip(tr("Loop"));
    } else if(volume_up_rect.contains(event->pos())) {
        setToolTip(tr("Volume up"));
    } else if(volume_down_rect.contains(event->pos())) {
        setToolTip(tr("Volume down"));
    } else if(fullscreen_rect.contains(event->pos())) {
        setToolTip(tr("Fullscreen"));
    } else {
        setToolTip(tr("Microninja Video Player"));
    }

    if (!isMoving && (open_rect.contains(event->pos()))) {
        mousePos = OPEN;
        update();
    }
    else if (!isMoving && (quit_rect.contains(event->pos()))) {
        mousePos = QUIT;
        update();
    }
    else if (!isMoving && (play_rect.contains(event->pos()))) {
        mousePos = PLAY;
        update();
    }
    else if (!isMoving && (stop_rect.contains(event->pos()))) {
        mousePos = STOP;
        update();
    }
    else if (!isMoving && (loop_rect.contains(event->pos()))) {
        mousePos = LOOP;
        update();
    }
    else if (!isMoving && (pause_rect.contains(event->pos()))) {
        mousePos = PAUSE;
        update();
    }
    else if (!isMoving && (fullscreen_rect.contains(event->pos()))) {
        mousePos = FULLSCREEN;
        update();
    }
    else if (!isMoving && (volume_up_rect.contains(event->pos()))) {
        mousePos = VOLUME_UP;
        update();
    }
    else if (!isMoving && (volume_down_rect.contains(event->pos()))) {
        mousePos = VOLUME_DOWN;
        update();
    }
    else {
        if(isMoving) {
            move(event->globalPos() - dragPosition);
            currentPos = event->globalPos() - dragPosition;
            event->accept();

            if((status == PLAYING) || (status == PAUSED)) {
                moveVideo();
            }
        }
        else if(mousePos != NONE) {
            mousePos = NONE;
            update();
        }
    }
}

void Player::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
        isMoving = false;
}

//! [2]

//! [3]
void Player::paintEvent(QPaintEvent *)
{
    const unsigned int baseY = 8;
    const unsigned int baseX = 20;

    QColor topColor(55, 55, 55);
    QColor bottomColor(11, 11, 11);

    QPainter painter(this);
    QLinearGradient gradient(0, 0, 0, height()); // diagonal gradient from top-left to bottom-right
    gradient.setColorAt(0, topColor);
    gradient.setColorAt(1, bottomColor);

    painter.fillRect(QRect(0, 0, width(), height()), gradient);

    painter.setPen(Qt::white);
    painter.drawRect(QRect(baseX, baseY, (width() - baseX*2), 44));

    open_rect = QRect(baseX + 13, baseY + 10, open_normal.width(), open_normal.height());
    if((status == GETTING_RES) || (status == STOPPED) || (status == FINISHED)) {
        if(mousePos != OPEN)
            painter.drawImage(open_rect, open_normal);
        else
            painter.drawImage(open_rect, open_light);
    } else {
        painter.drawImage(open_rect, open_disabled);
    }

    painter.drawLine(QLine(baseX + 50, baseY, baseX + 50, baseY + 44));

    play_rect = QRect(baseX + 63, baseY + 10, play_normal.width(), play_normal.height());
    pause_rect = QRect(baseX + 113, baseY + 10, pause_normal.width(), pause_normal.height());
    stop_rect = QRect(baseX + 163, baseY + 10, stop_normal.width(), stop_normal.height());
    loop_rect = QRect(baseX + 213, baseY + 10, loop_normal.width(), loop_normal.height());

    painter.drawLine(QLine(baseX + 250, baseY, baseX + 250, baseY + 44));

    volume_up_rect = QRect(baseX + 263, baseY + 10, volume_up_normal.width(), volume_up_normal.height());
    volume_down_rect = QRect(baseX + 313, baseY + 10, volume_down_normal.width(), volume_down_normal.height());

    painter.drawLine(QLine(baseX + 350, baseY, baseX + 350, baseY + 44));

    fullscreen_rect = QRect(baseX + 363, baseY + 10, fullscreen_normal.width(), fullscreen_normal.height());

    painter.drawLine(QLine(baseX + 400, baseY, baseX + 400, baseY + 44));
    painter.drawLine(QLine((width() - 13 - quit_normal.width() - 34),
                           baseY, (width() - 13 - quit_normal.width() - 34), baseY + 44));

    if(status == GETTING_RES) {
        painter.drawImage(play_rect, play_disabled);
        painter.drawImage(pause_rect, pause_disabled);
        painter.drawImage(stop_rect, stop_disabled);
        painter.drawImage(loop_rect, loop_disabled);
        painter.drawImage(fullscreen_rect, fullscreen_disabled);
        painter.drawImage(volume_up_rect, volume_up_disabled);
        painter.drawImage(volume_down_rect, volume_down_disabled);
    } else if(status == PLAYING) {
        painter.drawImage(play_rect, play_disabled);

        if(mousePos != PAUSE)
            painter.drawImage(pause_rect, pause_normal);
        else
            painter.drawImage(pause_rect, pause_light);
        if(mousePos != STOP)
            painter.drawImage(stop_rect, stop_normal);
        else
            painter.drawImage(stop_rect, stop_light);
        if(!loopEnabled) {
            if(mousePos != LOOP)
                painter.drawImage(loop_rect, loop_normal);
            else
                painter.drawImage(loop_rect, loop_light);
        } else {
            if(mousePos != LOOP)
                painter.drawImage(loop_rect, loop_active);
            else
                painter.drawImage(loop_rect, loop_active_light);
        }
        if(mousePos != FULLSCREEN)
            painter.drawImage(fullscreen_rect, fullscreen_normal);
        else
            painter.drawImage(fullscreen_rect, fullscreen_light);
        if(mousePos != VOLUME_UP)
            painter.drawImage(volume_up_rect, volume_up_normal);
        else
            painter.drawImage(volume_up_rect, volume_up_light);
        if(mousePos != VOLUME_DOWN)
            painter.drawImage(volume_down_rect, volume_down_normal);
        else
            painter.drawImage(volume_down_rect, volume_down_light);
    } else if((status == STOPPED) || (status == FINISHED) || (status == PAUSED)) {
        if(mousePos != PLAY)
            painter.drawImage(play_rect, play_normal);
        else
            painter.drawImage(play_rect, play_light);
        painter.drawImage(pause_rect, pause_disabled);
        painter.drawImage(stop_rect, stop_disabled);
        painter.drawImage(loop_rect, loop_disabled);
        painter.drawImage(fullscreen_rect, fullscreen_disabled);
        painter.drawImage(volume_up_rect, volume_up_disabled);
        painter.drawImage(volume_down_rect, volume_down_disabled);
    }

    //drawing status
    QFont font = painter.font();
    font.setPointSize(font.pointSize() * 1.2);
    painter.setFont(font);

    position_handle = QRect(baseX + 413, baseY + 10, 170, 8);

    if((status == PLAYING) || (status == PAUSED)) {
        //drawing handle position
        painter.setPen(Qt::gray);
        painter.setBrush(Qt::gray);
        painter.drawRect(position_handle);

        if(position > 0) {
            double ratio = (double)position / (double)duration;

            painter.setPen(Qt::green);
            painter.setBrush(Qt::green);
            painter.drawRect(QRect(position_handle.x(), position_handle.y(),
                                   ratio * position_handle.width(), position_handle.height()));
            painter.setPen(Qt::gray);
            painter.setBrush(Qt::gray);
        }
        //end drawing handle position

        int hoursDuration = duration / 3600;
        int minutesDuration = (duration - hoursDuration*3600)/60;
        int secondsDuration = duration%60;

        int hoursPosition = position / 3600;
        int minutesPosition = (position - hoursPosition*3600)/60;
        int secondsPosition = position%60;

        QString pos;
        QString dur;
        if(duration < 60) {
            dur.sprintf("00:%02d", secondsDuration);
            pos.sprintf("00:%02d", secondsPosition);
        } else if(duration < 3600) {
            dur.sprintf("%02d:%02d", minutesDuration, secondsDuration);
            pos.sprintf("%02d:%02d", minutesPosition, secondsPosition);
        } else {
            dur.sprintf("%02d:%02d:%02d", hoursDuration, minutesDuration, secondsDuration);
            pos.sprintf("%02d:%02d:%02d", hoursPosition, minutesPosition, secondsPosition);
        }
        QString textDurPos = pos + " / " + dur;
        painter.drawText(QRect(baseX + 413, baseY + 25, 170, baseY + 44), textDurPos);
    } else {
        //drawing handle position
        painter.setPen(Qt::gray);
        painter.setBrush(Qt::gray);
        painter.drawRect(position_handle);
        //end drawing handle position
        painter.drawText(QRect(baseX + 413, baseY + 25, 170, baseY + 44), "00:00:00/00:00:00");
    }
    //end drawing status

    quit_rect = QRect(width() - baseX - 13 - quit_normal.width(),
                      baseY + 10, quit_normal.width(), quit_normal.height());
    if(mousePos != QUIT)
        painter.drawImage(quit_rect, quit_normal);
    else
        painter.drawImage(quit_rect, quit_light);

}
//! [3]

//! [4]
void Player::resizeEvent(QResizeEvent * /* event */)
{
    QRegion rectRegio(height()/8, 0, width() - height()/4,
                      height(), QRegion::Rectangle);

    QRegion circleLeft(0, 0, height()/4, height(), QRegion::Ellipse);
    QRegion circleRight(width() - height()/4, 0, height()/4, height(), QRegion::Ellipse);

    QRegion maskedRegion = rectRegio.united(circleLeft).united(circleRight);

    setMask(maskedRegion);
}
//! [4]

//! [5]
QSize Player::sizeHint() const
{
    return QSize(WIDTH, HEIGHT);
}
//! [5]
