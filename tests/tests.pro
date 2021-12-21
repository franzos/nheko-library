QT += testlib
QT += gui widgets

SOURCES += main.cpp

HEADERS += testrunner.h \
           LoginTest.h 
    

CONFIG += testcase
CONFIG += no_testcase_installs
CONFIG += qt warn_on depend_includepath
CONFIG += c++14
CONFIG += cmdline

TEMPLATE = app
message(" * Building the px-matrix-client-library tests")
TARGET = run-all-tests
