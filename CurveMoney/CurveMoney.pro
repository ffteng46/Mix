TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CXXFLAGS += -std=gnu++11
LIBS += -L../build-article_price-Desktop-Debug -larticle_price
INCLUDEPATH += ../../boost_1_62_0
INCLUDEPATH += ../glog_0_3_3
LIBS += -L./lib
LIBS += -L../../boost_1_62_0/stage/lib
LIBS += -L../glog_0_3_3
SOURCES += main.cpp \
    EESTraderDemo.cpp \
    property.cpp \
    TimeProcesser.cpp \
    EESQuoteDemo.cpp \
    tcpServer.cpp
HEADERS += \
    property.h\
    EesTraderApi.h\
    EesTraderDefine.h\
    EESTraderDemo.h\
    EesTraderErr.h \
    TimeProcesser.h \
    EESQuoteApi.h \
    EESQuoteDefine.h \
    EESQuoteDemo.h \
    tcpServer.h

OTHER_FILES += \
    send.sh

LIBS += -lboost_locale -ldl -lboost_date_time -lboost_system -lboost_thread  -lpthread -lboost_chrono   -lEESTraderApi -lglog -lEESQuoteApi
