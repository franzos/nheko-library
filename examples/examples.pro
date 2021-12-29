message(" * Building the examples")
CONFIG += release
CONFIG += c++17

QT += gui widgets

#LIBS += -L/home/panther/Desktop/dev/matrix-client-library/build/src -lmatrix-client-library
LIBS += -lQMatrixClient
LIBS += -lolm
LIBS += -lspdlog
LIBS += -llmdb
LIBS += -lcmark
LIBS += -lcrypto -lssl


#unix:!macx: LIBS += -L$$OUT_PWD/../src/ -lmatrix-client-library
#INCLUDEPATH += $$PWD/../src
#DEPENDPATH += $$PWD/../src

message($$LIBS)
SOURCES = profileInfo.cpp
TEMPLATE = app
TARGET = profileInfo

