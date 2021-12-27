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
    Client *client;
private slots:
    void initTestCase(){
        client = Client::instance();
        client->enableLogger(true);
        QObject::connect(client,  &Client::loginReady, [&](const mtx::responses::Login &res){
            loginInfo = res;
            eventLoop.quit();
        });
        QObject::connect(client,  &Client::loginErrorOccurred, [&](const std::string &out){
            qCritical() << QString::fromStdString(out);
            eventLoop.quit();
        });

        if(client->hasValidUser()){
            loginInfo = client->userInformation();
            qDebug() << "has valid";
        } else {
            client->loginWithPassword(deviceName, userId, password, serverAddress); 
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
        
        client->bootstrap( loginInfo.user_id.to_string(),
                            serverAddress,
                            loginInfo.access_token);
        eventLoop.exec();
    }
};
