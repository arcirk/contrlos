QT = core

CONFIG += c++17 cmdline

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        ../../global/variant/item_data.cpp \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DEFINES += IS_USE_QT_LIB

HEADERS += \
    ../../global/global.hpp \
    ../../global/variant/item_data.h

INCLUDEPATH += ../../global
INCLUDEPATH += $(BOOST_INCLDUE)
Boost_USE_STATIC_LIBS = ON

windows:DEFINES += _CRT_SECURE_NO_WARNINGS
