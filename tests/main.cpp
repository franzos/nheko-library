
#include <QApplication>
#include "LoginTest.h"
#include "testrunner.h"
#include <signal.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    int status = 0;

    runTests<LoginTest>(argc, argv, &status);
    //runTest<loginFaile>(argc, argv, &status);
    raise(SIGINT);
    return app.exec();     
}

