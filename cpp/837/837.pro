QT       += core gui network axcontainer charts sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    data_manager.cpp \
    data_model.cpp \
    dialog_onoff.cpp \
    dialog_setposition.cpp \
    dialog_settimerange.cpp \
    dialog_setvalue.cpp \
    events.cpp \
    form_common.cpp \
    form_gasfeed.cpp \
    form_history.cpp \
    form_reactor.cpp \
    form_analysis.cpp \
    form_trend.cpp \
    historychart.cpp \
    main.cpp \
    mainwindow.cpp \
    trendchart.cpp

HEADERS += \
    ThreadSafeQueue.h \
    asyncfuture.h \
    data_manager.h \
    data_model.h \
    dialog_onoff.h \
    dialog_setposition.h \
    dialog_settimerange.h \
    dialog_setvalue.h \
    driver_base.h \
    events.h \
    form_common.h \
    form_gasfeed.h \
    form_history.h \
    form_reactor.h \
    form_analysis.h \
    form_trend.h \
    historychart.h \
    mainwindow.h \
    resourcedef.h \
    threadsafe_lookup_table.h \
    trendchart.h

FORMS += \
    dialog_onoff.ui \
    dialog_setposition.ui \
    dialog_settimerange.ui \
    dialog_setvalue.ui \
    form_analysis.ui \
    form_gasfeed.ui \
    form_history.ui \
    form_reactor.ui \
    form_trend.ui \
    mainwindow.ui

TRANSLATIONS += \
    837_zh_CN.ts

INCLUDEPATH += \
    C:\Users\huayu\Documents\HUAYUN\YASHEN\EccpHome\CPlusPlus\3rdLib
    #$$PWD/../../../3rdLib

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:contains(QT_ARCH, x86_64):CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../YASHEN/EccpHome/CPlusPlus/3rdLib/qredisclient/lib/ -lqredisclient
win32:contains(QT_ARCH, x86_64):CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../YASHEN/EccpHome/CPlusPlus/3rdLib/qredisclient/lib/ -lqredisclientd
win32:contains(QT_ARCH, i386):CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../YASHEN/EccpHome/CPlusPlus/3rdLib/qredisclient/lib/ -lqredisclient_32
win32:contains(QT_ARCH, i386):CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../YASHEN/EccpHome/CPlusPlus/3rdLib/qredisclient/lib/ -lqredisclient_32d

#win32:contains(QT_ARCH, x86_64):CONFIG(release, debug|release): LIBS += -L$$PWD/../../../3rdLib/qredisclient/lib/ -lqredisclient
#win32:contains(QT_ARCH, x86_64):CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../3rdLib/qredisclient/lib/ -lqredisclientd
#win32:contains(QT_ARCH, i386):CONFIG(release, debug|release): LIBS += -L$$PWD/../../../3rdLib/qredisclient/lib/ -lqredisclient_32
#win32:contains(QT_ARCH, i386):CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../3rdLib/qredisclient/lib/ -lqredisclient_32d

DISTFILES += \
    DataPointsCfg.xml \
    drivers.json

RESOURCES += \
    res.qrc
