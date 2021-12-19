#include <QtTest/QtTest>

class DummyTest: public QObject
{
    Q_OBJECT
private slots:
    void passed(){
        QVERIFY(1 == 1);
    }
};
