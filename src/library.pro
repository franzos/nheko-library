CONFIG += release
CONFIG += c++17

QT += core gui widgets quick
LIBS  = -lQMatrixClient
LIBS += -lSQLiteCpp
LIBS += -lolm
LIBS += -lspdlog

message(" * Building the px-matrix-client-library")



HEADERS =   Cache_p.h \
            Cache.h \
            CacheCryptoStructs.h \
            CacheStructs.h \
            Chat.h \
            EventAccessors.h \
            Login.h \
            Logging.h \
            MatrixClient.h \
            UserSettingsPage.h \
            Utils.h \
            encryption/Olm.h \
            timeline/EventStore.h \
            timeline/Reaction.h

SOURCES =   Cache.cpp \
            Chat.cpp \
            EventAccessors.cpp \
            Login.cpp \
            Logging.cpp \
            MatrixClient.cpp \
            UserSettingsPage.cpp \
            Utils.cpp \
            encryption/Olm.cpp \
            timeline/EventStore.cpp \
            timeline/Reaction.cpp

TEMPLATE = lib
TARGET = px-matrix-client-library