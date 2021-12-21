#include <iostream>
#include <QApplication>
#include <QObject>
#include <QDebug>

#include "../src/PxMatrixClient.h"
#include "../src/UserSettingsPage.h"

int main(int argc, char *argv[]){
    QApplication app(argc, argv);

    UserSettings::initialize(std::nullopt);
    
    auto client = new PxMatrixClient(UserSettings::instance());
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
    client->bootstrap("@hamzeh_test01:pantherx.org","matrix.pantherx.org:443","syt_aGFtemVoX3Rlc3QwMQ_fYmSAoBKiuVyXIrEpvDB_3S4zmr");
    return app.exec();     
}
