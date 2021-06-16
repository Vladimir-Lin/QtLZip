# LZip functions

INCLUDEPATH += $${PWD}

HEADERS     += $${PWD}/lzip.h
HEADERS     += $${PWD}/encoder.h
HEADERS     += $${PWD}/encoder_base.h
HEADERS     += $${PWD}/fast_encoder.h

SOURCES     += $${PWD}/encoder.cc
SOURCES     += $${PWD}/encoder_base.cc
SOURCES     += $${PWD}/fast_encoder.cc

OTHER_FILES += $${PWD}/main.cc
OTHER_FILES += $${PWD}/arg_parser.h
OTHER_FILES += $${PWD}/arg_parser.cc
