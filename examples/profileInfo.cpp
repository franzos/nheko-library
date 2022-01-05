#include <iostream>
#include <QApplication>
#include <QObject>
#include <QDebug>
#include <QEventLoop>
#include <QThread>
#include "../src/Client.h"

UserInformation loginInfo;
int main(int argc, char *argv[]){
    QApplication app(argc, argv);

    auto client = Client::instance();
    QObject::connect(client,  &Client::loginOk, [&](const UserInformation &res){
        loginInfo = res;
        client->start();
    });
    QObject::connect(client,  &Client::loginErrorOccurred, [&](const QString &out){
        qCritical() << out;
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

    QObject::connect(client, &Client::newUpdated,[client](const mtx::responses::Sync &sync){
        for(auto const &room: sync.rooms.join) {
            auto info = client->roomInfo(QString::fromStdString(room.first));
            qDebug() << "JOIN: " << QString::fromStdString(room.first) << info.name;
        }

        for(auto const &room: sync.rooms.invite) {
            auto info = client->roomInfo(QString::fromStdString(room.first));
            qDebug() << "INV : " << QString::fromStdString(room.first) << info.name;
        }

        for(auto const &room: sync.rooms.leave) {
            qDebug() << "LEFT: " << QString::fromStdString(room.first);
        }
    });
    
    QObject::connect(client,  &Client::dropToLogin, [&](const QString &msg){
        QString deviceName = "test";
        QString userId = "@hamzeh_test01:pantherx.org";
        QString password = "pQn3mDGsYR";
        QString serverAddress = "https://matrix.pantherx.org";   
        qWarning() << msg;
        client->loginWithPassword(deviceName, userId, password, serverAddress); 
    });

    QObject::connect(client,  &Client::initiateFinished, [&](){
        auto rooms = client->joinedRoomList();
        qInfo() << "Initiate Finished (" << rooms.size() << ")";
        for(auto const &r: rooms.toStdMap()){
            qInfo() << "Joined rooms: " << r.first;
        }
    });
    client->enableLogger(true,true);
    client->start();
    
    // QThread::sleep(10);
    // authentication->logout();
    return app.exec();     
}
