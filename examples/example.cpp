#include <iostream>
#include <QApplication>
#include <QObject>
#include <QDebug>

#include "../src/PxMatrixClient.h"

int main(int argc, char *argv[]){
    QApplication app(argc, argv);

    // UserSettings::initialize(std::nullopt);
    
    auto client = new PxMatrixClient();
    QObject::connect(client, &PxMatrixClient::userDisplayNameReady,[](const QString &name){
        qDebug() << "User Display Name: " << name;
    });

    QObject::connect(client, &PxMatrixClient::userAvatarReady,[](const QString &avatar){
        qDebug() << "User avatar      : " << avatar;
    });

    QObject::connect(client, &PxMatrixClient::roomListReady,[](const mtx::responses::Rooms &rooms){
        qDebug() << "Joine  Rooms: " << rooms.join.size();
        qDebug() << "Invite Rooms: " << rooms.invite.size();
        qDebug() << "Leave  Rooms: " << rooms.leave.size();
    });
    client->initialize("@hamzeh_test01:pantherx.org","matrix.pantherx.org:443","syt_aGFtemVoX3Rlc3QwMQ_fYmSAoBKiuVyXIrEpvDB_3S4zmr");
    client->getProfileInfo();
    return app.exec();     
}
