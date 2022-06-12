#include <iostream>
#include <QApplication>
#include <QObject>
#include <QDebug>
#include <QEventLoop>
#include <QThread>
#include "../src/Client.h"
#include "../src/encryption/SelfVerificationStatus.h"
#include "../src/encryption/DeviceVerificationFlow.h"
#include "../src/encryption/VerificationManager.h"

UserInformation loginInfo;
QMap <QString, Timeline*> timelineMap;
bool onetimeLogin = true;
bool oneTimeFlagJustForTesst = true;
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
        QString userId = "@hamzeh_test02:pantherx.org";
        QString password = "powwow-babbling-deviator"; 
        QString serverAddress = "https://matrix.pantherx.org";   
        qWarning() << msg;
        if(!onetimeLogin)
            return;
        client->loginWithPassword(deviceName, userId, password, serverAddress); 
        onetimeLogin = false;
    });

    QObject::connect(client,  &Client::initiateFinished, [&](){
        auto rooms = client->joinedRoomList();
        qInfo() << "Initiate Finished (" << rooms.size() << ")";
        for(auto const &r: rooms.toStdMap()){
            qInfo() << "Joined rooms: " << r.first;

            auto timeline = client->timeline(r.first);
            if(timeline){
                auto ev = timeline->getEvents(0,timeline->eventSize());
                qDebug() << "-------------------------------------------------------------------------------";
                qDebug() << "OLD MESSAGES";
                for(auto const &e: ev) {
                    qDebug() << e.userid << e.event_id << e.body << e.timestamp;
                }

                QObject::connect(timeline, &Timeline::newEventsStored, [timeline](int from, int len){
                    qDebug() << "-------------------------------------------------------------------------------";
                    qDebug() << "New MESSAGES";
                    auto events = timeline->getEvents(from, len);
                    for(auto const &e: events){
                        qDebug() << e.userid << e.event_id << e.body << e.timestamp;
                        if(e.body == "Hamzeh: answer");
                            // timeline->sendMessage("Hi, I got your message");
                    }
                    qDebug() << "-------------------------------------------------------------------------------";
                });
                QObject::connect(timeline, &Timeline::lastMessageChanged,[](const DescInfo &e){
                    qDebug() << "-------------------------------------------------------------------------------";
                    qDebug() << "LAST MESSAGE";
                    qDebug() << e.userid << e.event_id << e.body << e.timestamp;
                    qDebug() << "-------------------------------------------------------------------------------";
                });
            }
            // timeline->sendMessage("Hello, I'm here now.");
        }
        // -------------------------------------------
        // auto _verificationManager = client->verificationManager();
        // QObject::connect(_verificationManager, &VerificationManager::newDeviceVerificationRequest, [](DeviceVerificationFlow *flow){
        //     QObject::connect(flow,&DeviceVerificationFlow::stateChanged,[flow](){
        //         qDebug() << "--------------------------------------------------";
        //         qDebug() << flow->stateEnum() << flow->state(); 
        //         // if(flow->stateEnum() == DeviceVerificationFlow::State::CompareEmoji){
        //             auto keys = flow->getSasList();
        //             for(auto const&k: keys){
        //                 qDebug() << k;
        //             }
        //         // }
        //         qDebug() << "--------------------------------------------------";
        //     });
        //     if(oneTimeFlagJustForTesst) {
        //         oneTimeFlagJustForTesst = false;
        //         flow->next();
        //     }
        // });
        // auto selfVerificationStatus = _verificationManager->selfVerificationStatus();
        // QObject::connect(selfVerificationStatus, &SelfVerificationStatus::statusChanged,[selfVerificationStatus](){
        //     auto status = selfVerificationStatus->status();
        //     qDebug() << "***********************************************";
        //     qDebug() << status;
        //     qDebug() << "***********************************************";  
        //     if(status == SelfVerificationStatus::Status::UnverifiedDevices) {
        //         // selfVerificationStatus->verifyUnverifiedDevices();
        //     }
        // });
        // QTimer::singleShot(5000, [selfVerificationStatus] {
        // //    selfVerificationStatus->verifyMasterKeyWithPassphrase("EsTi jVgL chr3 nu3D avQ3 Ld9Y f4th 9wiF Ctvx Xqu7 tEv7 Uo7o");
        // //    selfVerificationStatus->verifyMasterKey(); 
        // });
    });
    client->enableLogger(true,true);
    client->start();
    
    // QThread::sleep(10);
    // authentication->logout();
    return app.exec();     
}
