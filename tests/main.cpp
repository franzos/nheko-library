#include "LoginTest.h"

#include "testrunner.h"

int main(int argc, char *argv[])
{

    int status = 0;

    runTests<LoginTest>(argc, argv, &status);
    
    return status;

}
