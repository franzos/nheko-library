#include <QtTest/QtTest>
#include <QEventLoop>
#include <iostream>

#include "../src/UserSettingsPage.h"


class UserSettingsTest: public QObject
{
    Q_OBJECT
    UserSettings *settings;
private slots:
    void initTestCase(){
        UserSettings::initialize(std::nullopt);
        settings = UserSettings::instance().get();
    }

    void secretStore(){
        settings->storeSecret("secret_key", "secret_value");
        auto secret = settings->secret("secret_key");
        QCOMPARE(secret, "secret_value");
    }
    
    void deleteStore(){
        settings->deleteSecret("secret_key");
        auto secret = settings->secret("secret_key");
        QCOMPARE(secret, QString::fromStdString(""));
    }
};
