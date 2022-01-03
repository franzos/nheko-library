#include <iostream>
#include <QApplication>
#include <QObject>
#include <QDebug>
#include <QEventLoop>
#include <QThread>
#include "../src/Client.h"

mtx::responses::Login loginInfo;
int main(int argc, char *argv[]){
    QApplication app(argc, argv);

    auto client = Client::instance();
    QEventLoop eventLoop;
    QObject::connect(client,  &Client::loginOk, [&](const UserInformation &res){
        //loginInfo = res;
        eventLoop.quit();
    });
    QObject::connect(client,  &Client::loginErrorOccurred, [&](const QString &out){
        qCritical() << out;
        eventLoop.quit();
    });

    QObject::connect(client, &Client::logoutOk,[](){
        qInfo() << "Logged out";
    });

    QObject::connect(client, &Client::userDisplayNameReady,[](const QString &name){
        qInfo() << "User Display Name: " << name;
    });

    QObject::connect(client, &Client::userAvatarReady,[](const QString &avatar){
        qInfo() << "User avatar      : " << avatar;
    });

    QObject::connect(client, &Client::roomListUpdated,[client](const mtx::responses::Rooms &rooms){
        for(auto const &room: rooms.join) {
            auto info = client->roomInfo(QString::fromStdString(room.first));
            qDebug() << "JOIN: " << QString::fromStdString(room.first) << QString::fromStdString(info.name);
        }

        for(auto const &room: rooms.invite) {
            auto info = client->roomInfo(QString::fromStdString(room.first));
            qDebug() << "INV : " << QString::fromStdString(room.first) << QString::fromStdString(info.name);
        }

        for(auto const &room: rooms.leave) {
            qDebug() << "LEFT: " << QString::fromStdString(room.first);
        }
    });
    

    if(client->hasValidUser()){
        //loginInfo = client->userInformation();
    } else {
        std::string deviceName = "test";
        std::string userId = "@hamzeh_test01:pantherx.org";
        std::string password = "pQn3mDGsYR";
        std::string serverAddress = "https://matrix.pantherx.org";   
        client->loginWithPassword(QString::fromStdString(deviceName), QString::fromStdString(userId), QString::fromStdString(password), QString::fromStdString(serverAddress)); 
        eventLoop.exec();
    }
    
    // client->bootstrap(loginInfo.user_id.to_string(),
    //                     "https://matrix.pantherx.org",
    //                     loginInfo.access_token);
    auto rooms = client->joinedRoomList();
    // QThread::sleep(10);
    // authentication->logout();
    return app.exec();     
}
