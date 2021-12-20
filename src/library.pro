CONFIG += release

LIBS  = -lQMatrixClient
LIBS += -lSQLiteCpp

message(" * Building the px-matrix-client-library")

HEADERS = Login.h \
        MatrixClient.h

SOURCES = Login.cpp \
         MatrixClient.cpp

TEMPLATE = lib
TARGET = px-matrix-client-library