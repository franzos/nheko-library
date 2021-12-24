#include <QtTest/QtTest>
#include <QEventLoop>
#include <iostream>

#include "../src/PXMatrixClient.h"

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
        px::mtx_client::init();
        auto loginTest = px::mtx_client::authentication();
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
            loginTest->loginWithPassword(deviceName, userId, password, serverAddress); 
            eventLoop.exec();
        }
    }
    void displayNameAndAvatar(){
        auto chat = px::mtx_client::chat();
        QObject::connect(chat, &Chat::userDisplayNameReady,[&](const std::string &name){
            qDebug() << QString::fromStdString(name);
            eventLoop.quit();
        });

        QObject::connect(chat, &Chat::userAvatarReady,[&](const std::string &avatar){
            qDebug() << QString::fromStdString(avatar);
            eventLoop.quit();
        });
        
        chat->initialize( loginInfo.user_id.to_string(),
                            serverAddress,
                            loginInfo.access_token);
        eventLoop.exec();
    }
};
