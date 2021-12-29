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
    QObject::connect(client,  &Client::loginOk, [&](const mtx::responses::Login &res){
        loginInfo = res;
        eventLoop.quit();
    });
    QObject::connect(client,  &Client::loginErrorOccurred, [&](const std::string &out){
        qCritical() << QString::fromStdString(out);
        eventLoop.quit();
    });

    QObject::connect(client, &Client::logoutOk,[](){
        qInfo() << "Logged out";
    });

    QObject::connect(client, &Client::userDisplayNameReady,[](const std::string &name){
        qInfo() << "User Display Name: " << QString::fromStdString(name);
    });

    QObject::connect(client, &Client::userAvatarReady,[](const std::string &avatar){
        qInfo() << "User avatar      : " << QString::fromStdString(avatar);
    });

    QObject::connect(client, &Client::roomListUpdated,[client](const mtx::responses::Rooms &rooms){
        for(auto const &room: rooms.join) {
            auto info = client->roomInfo(room.first);
            qDebug() << "JOIN: " << QString::fromStdString(room.first) << QString::fromStdString(info.name);
        }

        for(auto const &room: rooms.invite) {
            auto info = client->roomInfo(room.first);
            qDebug() << "INV : " << QString::fromStdString(room.first) << QString::fromStdString(info.name);
        }

        for(auto const &room: rooms.leave) {
            qDebug() << "LEFT: " << QString::fromStdString(room.first);
        }
    });
    

    if(client->hasValidUser()){
        loginInfo = client->userInformation();
    } else {
        std::string deviceName = "test";
        std::string userId = "@hamzeh_test01:pantherx.org";
        std::string password = "pQn3mDGsYR";
        std::string serverAddress = "https://matrix.pantherx.org";   
        client->loginWithPassword(deviceName, userId, password, serverAddress); 
        eventLoop.exec();
    }
    
    client->bootstrap(loginInfo.user_id.to_string(),
                        "https://matrix.pantherx.org",
                        loginInfo.access_token);
    auto rooms = client->joinedRoomList();
    // QThread::sleep(10);
    // authentication->logout();
    return app.exec();     
}
