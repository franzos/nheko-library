#include "dummytest.h"

#include "testrunner.h"

int main(int argc, char *argv[])
{

    int status = 0;

    runTests<DummyTest>(argc, argv, &status);
    
    return status;

}
