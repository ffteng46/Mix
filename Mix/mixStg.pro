TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += api_market.cpp \
    api_trader.cpp \
    calculate.cpp \
    read_data.cpp \
    S36.cpp \
    S36Dlg.cpp \
    sendOpenOrder.cpp \
    stdafx.cpp \
    Strategy.cpp \
    taskEditorDialog.cpp

OTHER_FILES += \
    mixStg.pro.user

HEADERS += \
    api_config_extern.h \
    api_config.h \
    api_market.h \
    api_trader.h \
    calculate.h \
    read_data.h \
    resource.h \
    S36.h \
    S36Dlg.h \
    sendOpenOrder.h \
    stdafx.h \
    Strategy.h \
    targetver.h \
    taskEditorDialog.h

