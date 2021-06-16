INCLUDEPATH += $${PWD}

HEADERS     += $${PWD}/qtucl.h

SOURCES     += $${PWD}/qtlzo.cpp
SOURCES     += $${PWD}/ScriptableLzo.cpp

CONFIG(debug, debug|release) {
LIBS        += -llzo2d
} else {
LIBS        += -llzo2
}
