CONFIG += release
CONFIG += c++17

QT += core gui widgets quick
LIBS  = -lQMatrixClient
LIBS += -lSQLiteCpp
LIBS += -lolm
LIBS += -lspdlog

message(" * Building the px-matrix-client-library")

HEADERS =   AvatarProvider.h \
            Cache_p.h \
            Cache.h \
            CacheCryptoStructs.h \
            CacheStructs.h \
            ChatPage.h \
            EventAccessors.h \
            Logging.h \
            MatrixClient.h \
            MxcImageProvider.h \
            UserSettingsPage.h \
            Utils.h \
            encryption/Olm.h \
            timeline/EventStore.h \
            timeline/Reaction.h

SOURCES =   AvatarProvider.cpp \
            Cache.cpp \
            ChatPage.cpp \
            EventAccessors.cpp \
            Logging.cpp \
            MatrixClient.cpp \
            MxcImageProvider.h \
            UserSettingsPage.cpp \
            Utils.cpp \
            encryption/Olm.cpp \
            timeline/EventStore.cpp \
            timeline/Reaction.cpp

TEMPLATE = lib
TARGET = px-matrix-client-library