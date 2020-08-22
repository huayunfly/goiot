QT       += core gui network

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
    dialog_setvalue.cpp \
    events.cpp \
    form_common.cpp \
    form_gasfeed.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    ThreadSafeQueue.h \
    asyncfuture.h \
    data_manager.h \
    data_model.h \
    dialog_setvalue.h \
    driver_base.h \
    events.h \
    form_common.h \
    form_gasfeed.h \
    mainwindow.h

FORMS += \
    dialog_setvalue.ui \
    form_gasfeed.ui \
    mainwindow.ui

TRANSLATIONS += \
    8H82600_zh_CN.ts

INCLUDEPATH += \
    #C:\Users\huayu\Documents\HUAYUN\YASHEN\EccpHome\CPlusPlus\3rdLib
    $$PWD/../../../3rdLib

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../YASHEN/EccpHome/CPlusPlus/3rdLib/qredisclient/lib/ -lqredisclient
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../YASHEN/EccpHome/CPlusPlus/3rdLib/qredisclient/lib/ -lqredisclientd

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../3rdLib/qredisclient/lib/ -lqredisclient
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../3rdLib/qredisclient/lib/ -lqredisclientd

DISTFILES += \
    drivers.json
