// Copyright (C) Alfabook srl
// License: http://www.gnu.org/licenses/gpl-2.0.txt GNU General Public License v2

#ifndef VIDEO_PLAYER_H
#define VIDEO_PLAYER_H

#include <QWidget>

enum VideoStatus
{
    GETTING_RES,
    STARTING_PLAY,
    PLAYING,
    PAUSED,
    STOPPED,
    FINISHED
};

enum MousePos
{
    NONE,
    OPEN,
    PLAY,
    PAUSE,    
    STOP,
    LOOP,
    FULLSCREEN,
    VOLUME_UP,
    VOLUME_DOWN,
    QUIT
};

//! [0]
class QProcess;
class QTimer;
class Player : public QWidget
{
    Q_OBJECT

public:
    Player(QWidget *parent = 0);
    ~Player();

    QSize sizeHint() const;

private:
    void loadImages();

protected:
    void keyPressEvent(QKeyEvent *);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

public slots:
    void play(const QString &path=QString());
    void setLoop();

private slots:
    //move video pos
    void startupMove();

    //status
    void readStatus();
    void statusFinished();
    void timerStatusHandler();

    //process
    void deleteProcess();
    void processRes();
    void readData();

    void pause();
    void stop();
    void volumeUp();
    void volumeDown();
    void moveVideo();
    void setVideoFullscreen();

private:
    QTimer *timer;

    QPoint dragPosition;
    QPoint startPos;
    QPoint currentPos;
    QPoint startVideoPos;

    VideoStatus status;
    QString videoPath;
    QProcess *process;
    QString resOut;
    QString statusString;

    unsigned int duration;
    unsigned int position;

    int movieW;
    int movieH;
    int sizeMovW;
    int sizeMovH;

    int screenWidth;
    int screenHeight;

    bool fullscreen;

    QImage play_light;
    QImage play_normal;
    QImage play_disabled;

    QImage pause_light;
    QImage pause_normal;
    QImage pause_disabled;

    QImage stop_light;
    QImage stop_normal;
    QImage stop_disabled;

    QImage loop_light;
    QImage loop_normal;
    QImage loop_disabled;
    QImage loop_active;
    QImage loop_active_light;

    QImage fullscreen_light;
    QImage fullscreen_normal;
    QImage fullscreen_disabled;

    QImage volume_up_light;
    QImage volume_up_normal;
    QImage volume_up_disabled;

    QImage volume_down_light;
    QImage volume_down_normal;
    QImage volume_down_disabled;

    QImage open_light;
    QImage quit_light;

    QImage open_normal;
    QImage quit_normal;

    QImage open_disabled;

    QRect open_rect;
    QRect play_rect;
    QRect pause_rect;
    QRect quit_rect;
    QRect stop_rect;
    QRect loop_rect;
    QRect fullscreen_rect;
    QRect volume_up_rect;
    QRect volume_down_rect;

    QRect position_handle;

    MousePos mousePos;

    bool isMoving;
    bool loopEnabled;

};
//! [0]

#endif
