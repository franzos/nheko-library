#include <QtTest/QtTest>
#include <QEventLoop>
#include <iostream>

#include "../src/Client.h"

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
    Client *client;
private slots:
    void initTestCase(){
        client = Client::instance();
        client->enableLogger(false);
        auth = new Authentication();
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
        QObject::connect(client, &Client::userDisplayNameReady,[&](const std::string &name){
            qDebug() << QString::fromStdString(name);
            eventLoop.quit();
        });

        QObject::connect(client, &Client::userAvatarReady,[&](const std::string &avatar){
            qDebug() << QString::fromStdString(avatar);
            eventLoop.quit();
        });
        
        client->initialize( loginInfo.user_id.to_string(),
                            serverAddress,
                            loginInfo.access_token);
        eventLoop.exec();
    }
};
