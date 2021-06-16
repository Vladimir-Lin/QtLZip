NAME         = QtLZip
TARGET       = $${NAME}
QT           = core
QT          -= gui
CONFIG(static,static|shared) {
# static version does not support Qt Script now
QT          -= script
} else {
QT          += script
}

load(qt_build_config)
load(qt_module)

INCLUDEPATH += $${PWD}/../../include/QtLZip
INCLUDEPATH += $${PWD}/../../include/QtLZip/lzip

include ($${PWD}/lzip/lzip.pri)

HEADERS     += $${PWD}/../../include/QtLZip/QtLZip
HEADERS     += $${PWD}/../../include/QtLZip/qtlzip.h

SOURCES     += $${PWD}/qtlzip.cpp
SOURCES     += $${PWD}/lzipcrc32.cpp
SOURCES     += $${PWD}/state.cpp
SOURCES     += $${PWD}/bitmodel.cpp
SOURCES     += $${PWD}/lenmodel.cpp
SOURCES     += $${PWD}/fileheader.cpp
SOURCES     += $${PWD}/filetrailer.cpp
SOURCES     += $${PWD}/rangedecoder.cpp
SOURCES     += $${PWD}/decoder.cpp
SOURCES     += $${PWD}/ScriptableLZip.cpp

OTHER_FILES += $${PWD}/../../include/$${NAME}/*.pri

include ($${PWD}/../../doc/Qt/Qt.pri)
