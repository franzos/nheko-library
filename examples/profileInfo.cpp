#include <iostream>
#include <QApplication>
#include <QObject>
#include <QDebug>
#include <QEventLoop>
#include "../src/PXMatrixClient.h"

mtx::responses::Login loginInfo;
int main(int argc, char *argv[]){
    QApplication app(argc, argv);

    px::mtx_client::init();
    auto loginTest = px::mtx_client::authentication();
    QEventLoop eventLoop;
    QObject::connect(loginTest,  &Authentication::loginOk, [&](const mtx::responses::Login &res){
        loginInfo = res;
        eventLoop.quit();
    });
    QObject::connect(loginTest,  &Authentication::loginErrorOccurred, [&](const std::string &out){
        qCritical() << QString::fromStdString(out);
        eventLoop.quit();
    });

    if(loginTest->hasValidUser()){
        loginInfo = loginTest->userInformation();
    } else {
        std::string deviceName = "test";
        std::string userId = "@hamzeh_test01:pantherx.org";
        std::string password = "pQn3mDGsYR";
        std::string serverAddress = "https://matrix.pantherx.org";   
        loginTest->loginWithPassword(deviceName, userId, password, serverAddress); 
        eventLoop.exec();
    }

    auto chat = px::mtx_client::chat();
    QObject::connect(chat, &Chat::userDisplayNameReady,[](const std::string &name){
        qInfo() << "User Display Name: " << QString::fromStdString(name);
    });

    QObject::connect(chat, &Chat::userAvatarReady,[](const std::string &avatar){
        qInfo() << "User avatar      : " << QString::fromStdString(avatar);
    });

    QObject::connect(chat, &Chat::roomListReady,[chat](){
        auto rooms = chat->joinedRoomList();
        for(auto const room: rooms){
            auto info = room.second;
            qDebug() << " + "   << room.first 
                                << QString::fromStdString(info.name) 
                                << QString::fromStdString(info.topic) 
                                << QString::fromStdString(info.avatar_url) 
                                << QString::fromStdString(info.version) << info.member_count;
        }
    });
    chat->initialize(loginInfo.user_id.to_string(),
                        "https://matrix.pantherx.org",
                        loginInfo.access_token);
    return app.exec();     
}
