#-------------------------------------------------
#
# Project created by QtCreator 2018-09-23T00:05:40
#
#-------------------------------------------------

QT       -= core gui

TARGET = article_price
TEMPLATE = lib

DEFINES += ARTICLE_PRICE_LIBRARY

SOURCES += article_price.cpp

HEADERS += article_price.h\
        article_price_global.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
