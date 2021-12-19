CONFIG += release

LIBS  = -lQMatrixClient
LIBS += -lSQLiteCpp

message(" * Building the px-matrix-client-library")

HEADERS = dummy.h
SOURCES = dummy.cpp

TEMPLATE = lib
TARGET = px-matrix-client-library