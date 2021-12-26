#include <QtTest/QtTest>
#include <QEventLoop>
#include <iostream>

#include "../src/PXMatrixClient.h"

class RoomTest: public QObject
{
    Q_OBJECT
private:
    mtx::responses::Login loginInfoUser1;
    std::string deviceName = "test";
    std::string userId1 = "@hamzeh_test01:pantherx.org";
    std::string password1 = "pQn3mDGsYR";
    std::string userId2 = "@hamzeh_test02:pantherx.org";
    std::string password2 = "5wn685g7mN";
    std::string serverAddress = "https://matrix.pantherx.org";   
    
    QEventLoop eventLoop;
    Chat        *chatUser1 = nullptr;
    Authentication *authUser1;
    std::string inviteRoomId;
    std::string joinRoomId = "!fCNlplLEJIMZawGUdE:pantherx.org";

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
        authUser1 = px::mtx_client::authentication();
        chatUser1 = px::mtx_client::chat();
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
        // join
        if(joinRoomId.empty())
            QFAIL("room id is empty.");
        auto joinSignal = connect(chatUser1,&Chat::joinedRoom,[&](const std::string &roomID){
            QCOMPARE(roomID,joinRoomId);
            eventLoop.quit();
        });
        auto joinErrorSignal = connect(chatUser1,&Chat::joinRoomFailed,[&](const std::string &error){
            QFAIL(error.c_str());
            eventLoop.quit();
        });
        chatUser1->joinRoom(joinRoomId);
        eventLoop.exec();
        disconnect(joinSignal);
        disconnect(joinErrorSignal);
    }

    void leaveRoom(){
        if(joinRoomId.empty())
            QFAIL("room id is empty.");
        else {
            auto leftRoomSignal = connect(chatUser1, &Chat::leftRoom,[&](const std::string room_id){
                QCOMPARE(joinRoomId,room_id);
                eventLoop.quit();
            });

            auto leftRoomErrorSignal = connect(chatUser1, &Chat::roomLeaveFailed,[&](const std::string error){
                QFAIL(error.c_str());
                eventLoop.quit();
            });
            chatUser1->leaveRoom(joinRoomId);
            eventLoop.exec();
            disconnect(leftRoomSignal);
            disconnect(leftRoomErrorSignal);
        }
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
        connect(authUser1, &Authentication::logoutOk,[&](){
            eventLoop.quit();
        });
        connect(authUser1, &Authentication::logoutErrorOccurred,[&](const std::string error){
            QFAIL(error.c_str());
            eventLoop.quit();
        });
        authUser1->logout();
        eventLoop.exec();
    }
};