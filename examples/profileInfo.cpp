#include <iostream>
#include <QApplication>
#include <QObject>
#include <QDebug>

#include "../src/PxMatrixClient.h"

int main(int argc, char *argv[]){
    QApplication app(argc, argv);

    auto client = new PxMatrixClient();
    QObject::connect(client, &PxMatrixClient::userDisplayNameReady,[](const QString &name){
        qInfo() << "User Display Name: " << name;
    });

    QObject::connect(client, &PxMatrixClient::userAvatarReady,[](const QString &avatar){
        qInfo() << "User avatar      : " << avatar;
    });

    QObject::connect(client, &PxMatrixClient::roomListReady,[](const mtx::responses::Rooms &rooms){
        qInfo() << "Joine  Rooms: " << rooms.join.size();
        qInfo() << "Invite Rooms: " << rooms.invite.size();
        qInfo() << "Leave  Rooms: " << rooms.leave.size();
    });
    client->initialize("@hamzeh_test01:pantherx.org","matrix.pantherx.org:443","syt_aGFtemVoX3Rlc3QwMQ_OGlfkHJTGULbfPmoWyYb_0nCa6p");
    client->getProfileInfo();
    return app.exec();     
}
