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
    Authentication *auth;
private slots:
    void initTestCase(){
        px::mtx_client::init();
        auth = px::mtx_client::authentication();
        QObject::connect(auth,  &Authentication::loginOk, [&](const mtx::responses::Login &res){
            loginInfo = res;
            eventLoop.quit();
        });
        QObject::connect(auth,  &Authentication::loginErrorOccurred, [&](const std::string &out){
            qCritical() << QString::fromStdString(out);
            eventLoop.quit();
        });

        if(auth->hasValidUser()){
            loginInfo = auth->userInformation();
            qDebug() << "has valid";
        } else {
            auth->loginWithPassword(deviceName, userId, password, serverAddress); 
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

    void cleanupTestCase(){
        connect(auth, &Authentication::logoutOk,[&](){
            eventLoop.quit();
        });
        connect(auth, &Authentication::logoutErrorOccurred,[&](const std::string &err){
            QFAIL(err.c_str());
            eventLoop.quit();
        });
        auth->logout();
        eventLoop.exec();
    }
};
