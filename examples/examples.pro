message(" * Building the examples")
CONFIG += release
CONFIG += c++17

QT += gui widgets
LIBS += -L /home/panther/Desktop/dev/matrix-client-library/qmake-build/src -lmatrix-client-library
message($$LIBS)
SOURCES = profileInfo.cpp
TEMPLATE = app
TARGET = profileInfo