#include <QtTest/QtTest>
#include <QEventLoop>
#include <iostream>

#include "../src/Client.h"

class ClientTest: public QObject
{
    Q_OBJECT
private:
    mtx::responses::Login loginInfo;
    std::string deviceName = "test";
    std::string userId = "@hamzeh_test01:pantherx.org";
    std::string password = "pQn3mDGsYR";
    std::string userId2 = "@hamzeh_test02:pantherx.org";
    std::string password2 = "5wn685g7mN";
    std::string serverAddress = "https://matrix.pantherx.org";   
    QEventLoop eventLoop;
    Client *client;
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
        client = Client::instance();
        UserSettings::instance()->clear();
        client->enableLogger(false);
        
    }

    void startProcess(){
        QObject::connect(client,  &Client::dropToLogin, [&](const std::string &msg){
            qDebug()<<"You are not logined yet";
            QVERIFY(1==1);
            eventLoop.quit();
        });
        client->start();
        eventLoop.exec();        
    }

    void clientLogin(){
        QObject::connect(client,  &Client::loginOk, [&](const  UserInformation &user){   
            qDebug()<<QString::fromStdString(user.userId);       
            QCOMPARE(user.userId,userId);
            client->start();
            eventLoop.quit();
        });

        QObject::connect(client,  &Client::loginErrorOccurred, [&](const std::string &out){
            qCritical() << QString::fromStdString(out);
            eventLoop.quit();
        });      
       client->loginWithPassword(deviceName, userId, password, serverAddress); 
       eventLoop.exec();  

       
        QObject::connect(client, &Client::initiateFinished,[&](){
            eventLoop.quit();
        });
        eventLoop.exec();      
    }

    void checkValidation(){
        if(client->hasValidUser()){
            QVERIFY(1==1);
        }else{
            QFAIL("Validation failed");
        }
    }

    void getUserInfo(){
        auto info = client->userInformation();
        QCOMPARE(info.userId,"@hamzeh_test01:pantherx.org");
    }
    

    void displayNameAndAvatar(){
        QObject::connect(client, &Client::userDisplayNameReady,[&](const std::string &name){
            qInfo() << QString::fromStdString(name);
            eventLoop.quit();
        });

        QObject::connect(client, &Client::userAvatarReady,[&](const std::string &avatar){
            qInfo() << QString::fromStdString(avatar);
            eventLoop.quit();
        });
        
        client->getProfileInfo();
        eventLoop.exec();
    }

    void createRoom(){
        mtx::requests::CreateRoom roomRqs;
        roomRqs.room_alias_name = GetRandomString(10).toStdString();
        roomRqs.is_direct = false;
        roomRqs.visibility = common::RoomVisibility::Public;
        roomRqs.name = GetRandomString(10).toStdString();
        roomRqs.topic = "test";
        connect(client, &Client::roomCreated,[&](const std::string roomId){
            inviteRoomId = roomId;
            eventLoop.quit();
        });

        connect(client, &Client::roomCreationFailed,[&](const std::string error){
            QFAIL(error.c_str());
            eventLoop.quit();
        });
        client->createRoom(roomRqs);
        eventLoop.exec();
    }

    void inviteRoom(){
        if(inviteRoomId.empty())
            QFAIL("room id is empty because of the previous test case (createRoom) failed");
        else {
            connect(client, &Client::userInvited,[&](const std::string room_id,const std::string user_id){
                QCOMPARE(inviteRoomId,room_id);
                QCOMPARE(userId2,user_id);
                eventLoop.quit();
            });

            connect(client, &Client::userInvitationFailed,[&](const std::string room_id,const std::string user_id, const std::string error){
                QFAIL((error + "(\"" + room_id + ", " + user_id + "\")").c_str());
                eventLoop.quit();
            });
            client->inviteUser(inviteRoomId,userId2, "test");
            eventLoop.exec();
        }
    }


   void roomList(){
        auto rooms = client->joinedRoomList();
        for(auto const &room: rooms){
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
        auto joinSignal = connect(client,&Client::joinedRoom,[&](const std::string &roomID){
            QCOMPARE(roomID,joinRoomId);
            eventLoop.quit();
        });
        auto joinErrorSignal = connect(client,&Client::joinRoomFailed,[&](const std::string &error){
            QFAIL(error.c_str());
            eventLoop.quit();
        });
        client->joinRoom(joinRoomId);
        eventLoop.exec();
        disconnect(joinSignal);
        disconnect(joinErrorSignal);
    }

    void leaveRoom(){
        if(joinRoomId.empty())
            QFAIL("room id is empty.");
        else {
            auto leftRoomSignal = connect(client, &Client::leftRoom,[&](const std::string room_id){
                QCOMPARE(joinRoomId,room_id);
                eventLoop.quit();
            });

            auto leftRoomErrorSignal = connect(client, &Client::roomLeaveFailed,[&](const std::string error){
                QFAIL(error.c_str());
                eventLoop.quit();
            });
            client->leaveRoom(joinRoomId);
            eventLoop.exec();
            disconnect(leftRoomSignal);
            disconnect(leftRoomErrorSignal);
        }
    }

    void deleteRoom(){
        if(inviteRoomId.empty())
            QFAIL("room id is empty because of the previous test case (createRoom) failed");
        else {
            connect(client, &Client::leftRoom,[&](const std::string room_id){
                QCOMPARE(inviteRoomId,room_id);
                eventLoop.quit();
            });

            connect(client, &Client::roomLeaveFailed,[&](const std::string error){
                QFAIL(error.c_str());
                eventLoop.quit();
            });
            client->leaveRoom(inviteRoomId);
            eventLoop.exec();
        }
    }

    void clientLogout(){
        connect(client, &Client::logoutOk,[&](){
            eventLoop.quit();
        });
        connect(client, &Client::logoutErrorOccurred,[&](const std::string error){
            QFAIL(error.c_str());
            eventLoop.quit();
        });
        client->logout();
        eventLoop.exec();
    }

    void cleanupTestCase(){
        QTimer::singleShot(4 * 1000, this, [&] {
            eventLoop.quit();
        });
        eventLoop.exec();
    }

};