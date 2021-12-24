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
    Chat        *chatUser2 = nullptr;
    std::string inviteRoomId;
    std::string joinRoomId;

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
            loginInfoUser1 = res;
            eventLoop.quit();
        });
        QObject::connect(loginTest,  &Authentication::errorOccurred, [&](const std::string &out){
            qCritical() << QString::fromStdString(out);
            eventLoop.quit();
        });

        if(loginTest->hasValidUser()){
            loginInfoUser1 = loginTest->userInformation();
        } else {
            loginTest->loginWithPassword(deviceName, userId1, password1, serverAddress); 
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
        QFAIL("TODO (Logout user1, Login user2, roomList, createRoom, join)");
        // if(joinRoomId.empty())
        //     QFAIL("room id is empty because of the previous test case (createRoom) failed");
        // connect(chatUser1,&Chat::joinedRoom,[&](const std::string &roomID){
        //     QCOMPARE(roomID,joinRoomId);
        //     eventLoop.quit();
        // });
        // connect(chatUser1,&Chat::joinRoomFailed,[&](const std::string &error){
        //     QFAIL(error.c_str());
        //     eventLoop.quit();
        // });
        // chatUser1->joinedRoom(joinRoomId);
        // eventLoop.exec();
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
        QFAIL("TODO (logout)");
    }
};