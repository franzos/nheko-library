#include <QtTest/QtTest>
#include <QEventLoop>
#include <iostream>

#include "../src/PXMatrixClient.h"

class RoomTest: public QObject
{
    Q_OBJECT
private:
    mtx::responses::Login loginInfo;
    std::string deviceName = "test";
    std::string userId = "@hamzeh_test01:pantherx.org";
    std::string password = "pQn3mDGsYR";
    std::string serverAddress = "https://matrix.pantherx.org";   
    std::string inviteUserId = "@hamzeh_test02:pantherx.org";
    QEventLoop eventLoop;
    Chat        *chat = nullptr;
    std::string roomID; 

    QString GetRandomString(int len) {
        const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
        const int randomStringLength = len; // assuming you want random strings of 12 characters

        QString randomString;
        for(int i=0; i<randomStringLength; ++i)
        {
            int index = QRandomGenerator::global()->generate() % possibleCharacters.length();
            QChar nextChar = possibleCharacters.at(index);
            randomString.append(nextChar);
        }
        return randomString;
    }

private slots:
    void initTestCase(){
        px::mtx_client::init();
        auto loginTest = px::mtx_client::authentication();
        QObject::connect(loginTest,  &Authentication::loginOk, [&](const mtx::responses::Login &res){
            loginInfo = res;
            eventLoop.quit();
        });
        QObject::connect(loginTest,  &Authentication::errorOccurred, [&](const std::string &out){
            qCritical() << QString::fromStdString(out);
            eventLoop.quit();
        });

        if(loginTest->hasValidUser()){
            loginInfo = loginTest->userInformation();
            qDebug() << "has valid";
        } else {
            loginTest->loginWithPassword(deviceName, userId, password, serverAddress); 
            eventLoop.exec();
        }
        chat = px::mtx_client::chat();
        chat->initialize( loginInfo.user_id.to_string(),
                    serverAddress,
                    loginInfo.access_token);
    }
    void roomList(){
        QFAIL("TODO");
    }

    void createRoom(){
        mtx::requests::CreateRoom roomRqs;
        roomRqs.room_alias_name = GetRandomString(10).toStdString();
        roomRqs.is_direct = false;
        roomRqs.visibility = common::RoomVisibility::Public;
        roomRqs.name = GetRandomString(10).toStdString();
        roomRqs.topic = "test";
        connect(chat, &Chat::roomCreated,[&](const std::string roomId){
            qDebug() << "Room ID:" << QString::fromStdString(roomId);
            roomID = roomId;
            eventLoop.quit();
        });

        connect(chat, &Chat::roomCreationFailed,[&](const std::string error){
            QFAIL(error.c_str());
            eventLoop.quit();
        });
        chat->createRoom(roomRqs);
        eventLoop.exec();
    }

    void inviteRoom(){
        if(roomID.empty())
            QFAIL("room id is empty because of the previous test case (createRoom) failed");
        else {
            connect(chat, &Chat::userInvited,[&](const std::string room_id,const std::string user_id){
                QCOMPARE(roomID,room_id);
                QCOMPARE("@hamzeh_test02:pantherx.org",user_id);
                eventLoop.quit();
            });

            connect(chat, &Chat::userInvitationFailed,[&](const std::string room_id,const std::string user_id, const std::string error){
                QFAIL(error.c_str());
                eventLoop.quit();
            });
            chat->inviteUser(roomID,inviteUserId, "test");
            eventLoop.exec();
        }
    }

    void deleteRoom(){
        if(roomID.empty())
            QFAIL("room id is empty because of the previous test case (createRoom) failed");
        else {
            connect(chat, &Chat::leftRoom,[&](const std::string room_id){
                QCOMPARE(roomID,room_id);
                eventLoop.quit();
            });

            connect(chat, &Chat::roomLeaveFailed,[&](const std::string error){
                QFAIL(error.c_str());
                eventLoop.quit();
            });
            chat->leaveRoom(roomID);
            eventLoop.exec();
        }
    }

    void joinRoom(){
        QFAIL("TODO");
    }

    void leaveRoom(){
        QFAIL("TODO");
    }
};