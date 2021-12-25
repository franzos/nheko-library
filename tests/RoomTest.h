#include <QtTest/QtTest>
#include <QEventLoop>
#include <iostream>

#include "../src/PXMatrixClient.h"

class RoomTest: public QObject
{
    Q_OBJECT
private:
    mtx::responses::Login loginInfoUser1;
    mtx::responses::Login loginInfoUser2;
    std::string deviceName = "test";
    std::string userId1 = "@hamzeh_test01:pantherx.org";
    std::string password1 = "pQn3mDGsYR";
    std::string userId2 = "@hamzeh_test02:pantherx.org";
    std::string password2 = "5wn685g7mN";
    std::string serverAddress = "https://matrix.pantherx.org";   
    
    QEventLoop eventLoop;
    Chat        *chatUser1 = nullptr;
    Chat        *chatUser2 = nullptr;
    Authentication *authUser1;
    Authentication *authUser2;
    std::string inviteRoomId;

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
        authUser1 = px::mtx_client::authentication();
        QObject::connect(authUser1,  &Authentication::loginOk, [&](const mtx::responses::Login &res){
            loginInfoUser1 = res;
            eventLoop.quit();
        });
        QObject::connect(authUser1,  &Authentication::loginErrorOccurred, [&](const std::string &out){
            qCritical() << QString::fromStdString(out);
            eventLoop.quit();
        });

        if(authUser1->hasValidUser()){
            loginInfoUser1 = authUser1->userInformation();
        } else {
            authUser1->loginWithPassword(deviceName, userId1, password1, serverAddress); 
            eventLoop.exec();
        }
        chatUser1 = px::mtx_client::chat();
        chatUser1->initialize( loginInfoUser1.user_id.to_string(),
                    serverAddress,
                    loginInfoUser1.access_token);
    }

    void createRoom(){
        mtx::requests::CreateRoom roomRqs;
        roomRqs.room_alias_name = GetRandomString(10).toStdString();
        roomRqs.is_direct = false;
        roomRqs.visibility = common::RoomVisibility::Public;
        roomRqs.name = GetRandomString(10).toStdString();
        roomRqs.topic = "test";
        connect(chatUser1, &Chat::roomCreated,[&](const std::string roomId){
            inviteRoomId = roomId;
            eventLoop.quit();
        });

        connect(chatUser1, &Chat::roomCreationFailed,[&](const std::string error){
            QFAIL(error.c_str());
            eventLoop.quit();
        });
        chatUser1->createRoom(roomRqs);
        eventLoop.exec();
    }

    void inviteRoom(){
        if(inviteRoomId.empty())
            QFAIL("room id is empty because of the previous test case (createRoom) failed");
        else {
            connect(chatUser1, &Chat::userInvited,[&](const std::string room_id,const std::string user_id){
                QCOMPARE(inviteRoomId,room_id);
                QCOMPARE(userId2,user_id);
                eventLoop.quit();
            });

            connect(chatUser1, &Chat::userInvitationFailed,[&](const std::string room_id,const std::string user_id, const std::string error){
                QFAIL((error + "(\"" + room_id + ", " + user_id + "\")").c_str());
                eventLoop.quit();
            });
            chatUser1->inviteUser(inviteRoomId,userId2, "test");
            eventLoop.exec();
        }
    }

    void roomList(){
        auto rooms = chatUser1->joinedRoomList();
        for(auto const room: rooms){
            if(room.first.toStdString() == inviteRoomId) {
                return;
            }
        }
        QFAIL("Created room in the previous step not found!");
    }

    void joinRoom(){
        // logout user1
        connect(authUser1, &Authentication::logoutOk,[&](){
            eventLoop.quit();
        });
        connect(authUser1, &Authentication::logoutErrorOccurred,[&](const std::string error){
            QFAIL(error.c_str());
            eventLoop.quit();
        });
        authUser1->logout();
        eventLoop.exec();
        // login as user2
        authUser2 = px::mtx_client::authentication();
        QObject::connect(authUser2,  &Authentication::loginOk, [&](const mtx::responses::Login &res){
            loginInfoUser2 = res;
            eventLoop.quit();
        });
        QObject::connect(authUser2,  &Authentication::loginErrorOccurred, [&](const std::string &err){
            QFAIL(err.c_str());
            eventLoop.quit();
        });

        if(authUser2->hasValidUser()){
            loginInfoUser2 = authUser2->userInformation();
        } else {
            authUser2->loginWithPassword(deviceName, userId2, password2, serverAddress); 
            eventLoop.exec();
        }
        chatUser2 = px::mtx_client::chat();
        chatUser2->initialize( loginInfoUser2.user_id.to_string(),
                    serverAddress,
                    loginInfoUser2.access_token);
        // join
        if(inviteRoomId.empty())
            QFAIL("room id is empty because of the previous test case (createRoom) failed");
        connect(chatUser2,&Chat::joinedRoom,[&](const std::string &roomID){
            QCOMPARE(roomID,inviteRoomId);
            eventLoop.quit();
        });
        connect(chatUser2,&Chat::joinRoomFailed,[&](const std::string &error){
            QFAIL(error.c_str());
            eventLoop.quit();
        });
        chatUser2->joinedRoom(inviteRoomId);
        eventLoop.exec();
    }

    void deleteRoom(){
        if(inviteRoomId.empty())
            QFAIL("room id is empty because of the previous test case (createRoom) failed");
        else {
            connect(chatUser1, &Chat::leftRoom,[&](const std::string room_id){
                QCOMPARE(inviteRoomId,room_id);
                eventLoop.quit();
            });

            connect(chatUser1, &Chat::roomLeaveFailed,[&](const std::string error){
                QFAIL(error.c_str());
                eventLoop.quit();
            });
            chatUser1->leaveRoom(inviteRoomId);
            eventLoop.exec();
        }
    }

    void cleanupTestCase(){
        connect(authUser2, &Authentication::logoutOk,[&](){
            eventLoop.quit();
        });
        connect(authUser2, &Authentication::logoutErrorOccurred,[&](const std::string error){
            QFAIL(error.c_str());
            eventLoop.quit();
        });
        authUser2->logout();
        eventLoop.exec();
    }
};