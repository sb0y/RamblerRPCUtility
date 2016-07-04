#-------------------------------------------------
#
# Project created by QtCreator 2016-05-28T20:41:24
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ramblerpcutility
TEMPLATE = app
CONFIG += c++14

VERSION = 0.0.0.1
QMAKE_TARGET_COMPANY = Rambler
QMAKE_TARGET_PRODUCT = RambleRPCUtility
QMAKE_TARGET_DESCRIPTION = Rambler JSON\XML test query utility
QMAKE_TARGET_COPYRIGHT = andrey@bagrintsev.me

include( src/src.pri )
include( src/ui/ui.pri )

unix:!mac:DEFINES += HAVE_X11
DEFINES += VERSION=$$VERSION
#DEFINES += BUILD_DATE='"\\\"$(shell date)\\\""'
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x000000

unix {
	execfiles.path = /usr/bin
	execfiles.files = ramblerpcutility

	appmenu.path = /usr/share/applications
	appmenu.files += resources/*.desktop

	icons.path = /usr/share/pixmaps
	icons.files += resources/*.png

	INSTALLS += execfiles
	INSTALLS += icons
	INSTALLS += appmenu
}
