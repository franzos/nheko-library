
#include <QApplication>
#include <signal.h>
#include "testrunner.h"

#include "LoginTest.h"
#include "ProfileInfoTest.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    int status = 0;
    // ------------------------------------------------------------------------------------ Add tests here
    runTests<LoginTest>(argc, argv, &status);
    runTests<ProfileInfoTest>(argc, argv, &status);
    // --------------------------------------------------------------------------------------------------- 
    raise(SIGINT);
    return app.exec();     
}

