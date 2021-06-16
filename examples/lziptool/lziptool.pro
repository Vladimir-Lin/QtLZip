QT             = core
QT            -= gui
QT            += QtLZip

CONFIG        += console

TEMPLATE       = app

SOURCES       += $${PWD}/lziptool.cpp

win32 {
RC_FILE        = $${PWD}/lziptool.rc
OTHER_FILES   += $${PWD}/lziptool.rc
OTHER_FILES   += $${PWD}/*.js
}
