#include <QtTest/QtTest>
#include <QEventLoop>
#include <iostream>

#include "../src/Authentication.h"
#include "../src/Chat.h"

class ProfileInfoTest: public QObject
{
    Q_OBJECT
private:
    mtx::responses::Login loginInfo;
    std::string deviceName = "test";
    std::string userId = "@hamzeh_test01:pantherx.org";
    std::string password = "pQn3mDGsYR";
    std::string serverAddress = "https://matrix.pantherx.org";   
    QEventLoop eventLoop;
private slots:
    void initTestCase(){
        Authentication *loginTest = new Authentication;
        QEventLoop eventLoop;
        QObject::connect(loginTest,  &Authentication::loginOk, [&](const mtx::responses::Login &res){
            loginInfo = res;
            eventLoop.quit();
        });
        QObject::connect(loginTest,  &Authentication::errorOccurred, [&](const std::string &out){
            qCritical() << QString::fromStdString(out);
            eventLoop.quit();
        });

        if(loginTest->hasValidUser()){
            loginInfo = loginTest->userInformation();
            qDebug() << "has valid";
        } else {
            loginTest->loginWithUsername(deviceName, userId, password, serverAddress); 
            eventLoop.exec();
        }
    }
    void test(){
        auto client = new Chat();
        QObject::connect(client, &Chat::userDisplayNameReady,[&](const std::string &name){
            qDebug() << QString::fromStdString(name);
            eventLoop.quit();
        });

        QObject::connect(client, &Chat::userAvatarReady,[&](const std::string &avatar){
            qDebug() << QString::fromStdString(avatar);
            eventLoop.quit();
        });
        
        client->initialize( loginInfo.user_id.to_string(),
                            serverAddress,
                            loginInfo.access_token);
        eventLoop.exec();
    }
};
