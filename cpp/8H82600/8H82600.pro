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
    dialog_setvalue.cpp \
    events.cpp \
    form_common.cpp \
    form_cylindera.cpp \
    form_cylinderb.cpp \
    form_gasfeed.cpp \
    form_gassampling.cpp \
    form_liquidcollection.cpp \
    form_liquiddistributor.cpp \
    form_liquidfeeda.cpp \
    form_liquidfeedb.cpp \
    form_liquidsamplinga.cpp \
    form_liquidsamplingb.cpp \
    form_liquidswitch.cpp \
    form_motorcontrol.cpp \
    form_reactora.cpp \
    form_reactorb.cpp \
    form_trend.cpp \
    main.cpp \
    mainwindow.cpp \
    sampling_ui_item.cpp

HEADERS += \
    ThreadSafeQueue.h \
    asyncfuture.h \
    data_manager.h \
    data_model.h \
    dialog_onoff.h \
    dialog_setposition.h \
    dialog_setvalue.h \
    driver_base.h \
    events.h \
    form_common.h \
    form_cylindera.h \
    form_cylinderb.h \
    form_gasfeed.h \
    form_gassampling.h \
    form_liquidcollection.h \
    form_liquiddistributor.h \
    form_liquidfeeda.h \
    form_liquidfeedb.h \
    form_liquidsamplinga.h \
    form_liquidsamplingb.h \
    form_liquidswitch.h \
    form_motorcontrol.h \
    form_reactora.h \
    form_reactorb.h \
    form_trend.h \
    mainwindow.h \
    resourcedef.h \
    sampling_ui_item.h

FORMS += \
    dialog_onoff.ui \
    dialog_setposition.ui \
    dialog_setvalue.ui \
    form_cylindera.ui \
    form_cylinderb.ui \
    form_gasfeed.ui \
    form_gassampling.ui \
    form_liquidcollection.ui \
    form_liquiddistributor.ui \
    form_liquidfeeda.ui \
    form_liquidfeedb.ui \
    form_liquidsamplinga.ui \
    form_liquidsamplingb.ui \
    form_liquidswitch.ui \
    form_motorcontrol.ui \
    form_reactora.ui \
    form_reactorb.ui \
    form_trend.ui \
    mainwindow.ui

TRANSLATIONS += \
    8H82600_zh_CN.ts

INCLUDEPATH += \
    C:\Users\huayu\Documents\HUAYUN\YASHEN\EccpHome\CPlusPlus\3rdLib \
    C:\Users\huayu\Documents\HUAYUN\YASHEN\EccpHome\CPlusPlus\3rdLib\opencv\v3_4_16\include
    #$$PWD/../../../3rdLib \
    #$$PWD/../../../3rdLib/opencv/v3_4_16/include

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../YASHEN/EccpHome/CPlusPlus/3rdLib/qredisclient/lib/ -lqredisclient
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../YASHEN/EccpHome/CPlusPlus/3rdLib/qredisclient/lib/ -lqredisclientd

win32:contains(QT_ARCH, x86_64):CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../YASHEN/EccpHome/CPlusPlus/3rdLib/qredisclient/lib/ -lqredisclient \
                                                                    -L$$PWD/../../../../YASHEN/EccpHome/CPlusPlus/3rdLib/opencv/v3_4_16/lib/x64/ -lopencv_core3416.dll -lopencv_highgui3416.dll -lopencv_video3416.dll -lopencv_videoio3416.dll -lopencv_imgproc3416.dll

win32:contains(QT_ARCH, x86_64):CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../YASHEN/EccpHome/CPlusPlus/3rdLib/qredisclient/lib/ -lqredisclientd \
                                                                    -L$$PWD/../../../../YASHEN/EccpHome/CPlusPlus/3rdLib/opencv/v3_4_16/lib/x64/ -lopencv_core3416.dll -lopencv_highgui3416.dll -lopencv_video3416.dll -lopencv_videoio3416.dll -lopencv_imgproc3416.dll

win32:contains(QT_ARCH, i386):CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../YASHEN/EccpHome/CPlusPlus/3rdLib/qredisclient/lib/ -lqredisclient_32 \
                                                                    -L$$PWD/../../../../YASHEN/EccpHome/CPlusPlus/3rdLib/opencv/v3_4_16/lib/ -lopencv_core3416.dll -lopencv_highgui3416.dll -lopencv_video3416.dll -lopencv_videoio3416.dll -lopencv_imgproc3416.dll

win32:contains(QT_ARCH, i386):CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../YASHEN/EccpHome/CPlusPlus/3rdLib/qredisclient/lib/ -lqredisclient_32d \
                                                                    -L$$PWD/../../../../YASHEN/EccpHome/CPlusPlus/3rdLib/opencv/v3_4_16/lib/ -lopencv_core3416.dll -lopencv_highgui3416.dll -lopencv_video3416.dll -lopencv_videoio3416.dll -lopencv_imgproc3416.dll


#win32:contains(qt_arch, x86_64):config(release, debug|release): libs += -l$$pwd/../../../3rdlib/qredisclient/lib/ -lqredisclient

#win32:contains(qt_arch, x86_64):config(debug, debug|release): libs += -l$$pwd/../../../3rdlib/qredisclient/lib/ -lqredisclientd

#win32:contains(qt_arch, i386):config(release, debug|release): libs += -l$$pwd/../../../3rdlib/qredisclient/lib/ -lqredisclient_32 \
#                                                                    -l$$pwd/../../../3rdlib/opencv/v3_4_16/lib/ -lopencv_core3416.dll -lopencv_highgui3416.dll -lopencv_video3416.dll -lopencv_videoio3416.dll -lopencv_imgproc3416.dll

#win32:contains(qt_arch, i386):config(debug, debug|release): libs += -l$$pwd/../../../3rdlib/qredisclient/lib/ -lqredisclient_32d \
#                                                                    -l$$pwd/../../../3rdlib/opencv/v3_4_16/lib/ -lopencv_core3416.dll -lopencv_highgui3416.dll -lopencv_video3416.dll -lopencv_videoio3416.dll -lopencv_imgproc3416.dll





DISTFILES += \
    drivers.json

RESOURCES += \
    res.qrc
