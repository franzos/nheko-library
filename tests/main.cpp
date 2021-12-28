
#include <QApplication>
#include <signal.h>
#include "testrunner.h"

#include "AuthenticationTest.h"
#include "ProfileInfoTest.h"
#include "RoomTest.h"
#include "UserSettingsTest.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    int status = 0;
    // ------------------------------------------------------------------------------------ Add tests here
    runTests<UserSettingsTest>(argc, argv, &status);
    runTests<AuthenticationTest>(argc, argv, &status);
    runTests<ProfileInfoTest>(argc, argv, &status);
    runTests<RoomTest>(argc, argv, &status);
    // --------------------------------------------------------------------------------------------------- 
    raise(SIGINT);
    return app.exec();     
}

