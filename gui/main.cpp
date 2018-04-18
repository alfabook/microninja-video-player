// Copyright (C) Alfabook srl
// License: http://www.gnu.org/licenses/gpl-2.0.txt GNU General Public License v2

#include <QApplication>
#include <QTranslator>
#include <QLocale>

#include "player.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QLocale locale;
    QTranslator translator;

    QString lang = locale.name();
    if(lang.length() > 2) {
        lang = lang.left(2);
    }

    translator.load("microninja-video-player_" + lang, "/usr/share/locale/microninja/microninja-video-player/translations");
    app.installTranslator(&translator);

    Player player;
    player.show();

    if(argc == 2)
        player.play(argv[1]);

    if(argc == 3) {
        if(QString(argv[1]) == "--loop") {
            player.play(argv[2]);
            player.setLoop();
        }
    }
    return app.exec();
}
