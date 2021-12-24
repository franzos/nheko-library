#ifndef TESTRUNNER_H
#define TESTRUNNER_H

#include <QtTest>
#include <QDebug>
template <typename TestClass>
void runTests(int argc, char* argv[], int* status)
{
    QTEST_DISABLE_KEYPAD_NAVIGATION TestClass tc;
    *status |= QTest::qExec(&tc, argc, argv);
    qDebug() << "________________________________________________________________________________\n\n";
    QThread::sleep(1);
}

#endif // TESTRUNNER_H