all: microninja-video-player

microninja-video-player:
	cd gui && qmake-qt4 && make clean && make

