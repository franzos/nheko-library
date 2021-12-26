#include <iostream>
#include <QApplication>
#include <QObject>
#include <QDebug>
#include <QEventLoop>
#include <QThread>
#include "../src/PXMatrixClient.h"

mtx::responses::Login loginInfo;
int main(int argc, char *argv[]){
    QApplication app(argc, argv);

    px::mtx_client::init();
    auto authentication = px::mtx_client::authentication();
    QEventLoop eventLoop;
    QObject::connect(authentication,  &Authentication::loginOk, [&](const mtx::responses::Login &res){
        loginInfo = res;
        eventLoop.quit();
    });
    QObject::connect(authentication,  &Authentication::loginErrorOccurred, [&](const std::string &out){
        qCritical() << QString::fromStdString(out);
        eventLoop.quit();
    });

    QObject::connect(authentication, &Authentication::logoutOk,[](){
        qInfo() << "Logged out";
    });

    if(authentication->hasValidUser()){
        loginInfo = authentication->userInformation();
    } else {
        std::string deviceName = "test";
        std::string userId = "@hamzeh_test01:pantherx.org";
        std::string password = "pQn3mDGsYR";
        std::string serverAddress = "https://matrix.pantherx.org";   
        authentication->loginWithPassword(deviceName, userId, password, serverAddress); 
        eventLoop.exec();
    }

    auto chat = px::mtx_client::chat();
    QObject::connect(chat, &Chat::userDisplayNameReady,[](const std::string &name){
        qInfo() << "User Display Name: " << QString::fromStdString(name);
    });

    QObject::connect(chat, &Chat::userAvatarReady,[](const std::string &avatar){
        qInfo() << "User avatar      : " << QString::fromStdString(avatar);
    });

    QObject::connect(chat, &Chat::roomListUpdated,[chat](const mtx::responses::Rooms &rooms){
        for(auto const room: rooms.join) {
            auto info = chat->roomInfo(room.first);
            qDebug() << "JOIN: " << QString::fromStdString(room.first) << QString::fromStdString(info.name);
        }

        for(auto const room: rooms.invite) {
            auto info = chat->roomInfo(room.first);
            qDebug() << "INV : " << QString::fromStdString(room.first) << QString::fromStdString(info.name);
        }

        for(auto const room: rooms.leave) {
            qDebug() << "LEFT: " << QString::fromStdString(room.first);
        }
    });
    
    chat->initialize(loginInfo.user_id.to_string(),
                        "https://matrix.pantherx.org",
                        loginInfo.access_token);
    auto rooms = chat->joinedRoomList();
    // QThread::sleep(10);
    // authentication->logout();
    return app.exec();     
}
