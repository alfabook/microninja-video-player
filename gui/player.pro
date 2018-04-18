greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += RELEASE

TARGET = microninja-video-player

HEADERS       = player.h
SOURCES       = player.cpp \
                main.cpp

TRANSLATIONS = ../po/microninja-video-player_it.ts

# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/widgets/shapedclock
INSTALLS += target

RESOURCES += \
    resources.qrc
