CONFIG += release
CONFIG += c++17

SHARED_LIB{
    CONFIG += shared
    message(" + Build as shared library.")
} else {
    CONFIG += static
    message(" + Build as static library.")
}

QT += dbus 
LIBS +=-L $$(LIBRARY_PATH) -lQMatrixClient
LIBS += -lolm
LIBS += -lspdlog
LIBS += -llmdb
LIBS += -lqt5keychain
LIBS += -lcrypto -lssl

INCLUDEPATH += src
message(" * Building the matrix-client-library")
message($$LIBS)
HEADERS =   Authentication.h \
            Cache_p.h \
            Cache.h \
            CacheCryptoStructs.h \
            CacheStructs.h \
            Client.h \
            EventAccessors.h \
            Logging.h \
            MatrixClient.h \
            UserSettingsPage.h \
            Utils.h \
            encryption/Olm.h \
            timeline/EventStore.h \
            timeline/Reaction.h

SOURCES =   Authentication.cpp \
            Cache.cpp \
            Client.cpp \
            EventAccessors.cpp \
            Logging.cpp \
            MatrixClient.cpp \
            UserSettingsPage.cpp \
            Utils.cpp \
            encryption/Olm.cpp \
            timeline/EventStore.cpp \
            timeline/Reaction.cpp

TEMPLATE = lib
TARGET = matrix-client-library