
#include <QApplication>
#include <signal.h>
#include "testrunner.h"

#include "AuthenticationTest.h"
#include "ProfileInfoTest.h"
#include "RoomTest.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    int status = 0;
    // ------------------------------------------------------------------------------------ Add tests here
    runTests<AuthenticationTest>(argc, argv, &status);
    //runTests<ProfileInfoTest>(argc, argv, &status);
    // --------------------------------------------------------------------------------------------------- 
    raise(SIGINT);
    return app.exec();     
}

