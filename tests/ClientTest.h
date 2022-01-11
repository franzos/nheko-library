#include <QtTest/QtTest>
#include <QEventLoop>
#include <iostream>

#include "../src/Client.h"

class ClientTest: public QObject
{
    Q_OBJECT
private:
    mtx::responses::Login loginInfo;
    QString deviceName = "test";
    QString userId = "@fakhri_test01:pantherx.org";
    QString password = "a2bqy9iHU8";
    QString userId2 = "@hamzeh_test02:pantherx.org";
    QString password2 = "5wn685g7mN";
    QString serverAddress = "https://matrix.pantherx.org";   
    QEventLoop eventLoop;
    Client *client;
    bool isInitialSynced, isUpdated;
    QString inviteRoomId;
    QString joinRoomId = "!fCNlplLEJIMZawGUdE:pantherx.org";

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
        client->enableLogger(false,false);
        isInitialSynced = false;
        isUpdated=false;

        QObject::connect(client,  &Client::initialSync, [&](const mtx::responses::Sync &sync){  
            (void)sync; 
            qDebug()<<"Initial Sync is done";  
            isInitialSynced = true;               
        });

        QObject::connect(client,  &Client::newUpdated, [&](const mtx::responses::Sync &sync){   
            (void)sync;
            qDebug()<<"Update Sync is done";  
            isUpdated = true;               
        });
    }

    void startProcess(){
        QObject::connect(client,  &Client::dropToLogin, [&](const QString &msg){
            qDebug() << msg;
            QVERIFY(1==1);
            eventLoop.quit();
        });
        client->start();
        eventLoop.exec();        
    }

    void clientLogin(){
       
        
        QObject::connect(client,  &Client::loginOk, [&](const  UserInformation &user){   
            qDebug()<<user.userId;       
            QCOMPARE(user.userId,userId);
            client->start();
            eventLoop.quit();
        });

        QObject::connect(client,  &Client::loginErrorOccurred, [&](const QString &out){
            qCritical() << out;
            qDebug()<<out;   
            eventLoop.quit();
        });   

         try
        {
           client->loginWithPassword(deviceName, userId, password, serverAddress); 
            eventLoop.exec();  
            QObject::connect(client, &Client::initiateFinished,[&](){
            eventLoop.quit();
        });
        eventLoop.exec();    
        }
        catch(const std::exception& e)
        {            
            qDebug()<<" Loged in has error"<<e.what();
        }   
              
         
    }

    void initialSync(){
        QCOMPARE(isInitialSynced,true);
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
        QCOMPARE(info.userId,"@fakhri_test01:pantherx.org");
    }
    

    void displayNameAndAvatar(){
        QObject::connect(client, &Client::userDisplayNameReady,[&](const QString &name){
            qInfo() << name;
            eventLoop.quit();
        });

        QObject::connect(client, &Client::userAvatarReady,[&](const QString &avatar){
            qInfo() << avatar;
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
        connect(client, &Client::roomCreated,[&](const QString roomId){
            inviteRoomId = roomId;
            eventLoop.quit();
        });

        connect(client, &Client::roomCreationFailed,[&](const QString error){
            QFAIL(error.toStdString().c_str());
            eventLoop.quit();
        });
        client->createRoom(roomRqs);
        eventLoop.exec();
    }

    void inviteRoom(){
        if(inviteRoomId.isEmpty())
            QFAIL("room id is empty because of the previous test case (createRoom) failed");
        else {
            connect(client, &Client::userInvited,[&](const QString room_id,const QString user_id){
                QCOMPARE(inviteRoomId,room_id);
                QCOMPARE(userId2,user_id);
                eventLoop.quit();
            });

            connect(client, &Client::userInvitationFailed,[&](const QString room_id,const QString user_id, const QString error){
                QFAIL((error + "(\"" + room_id + ", " + user_id + "\")").toStdString().c_str());
                eventLoop.quit();
            });
            client->inviteUser(inviteRoomId,userId2, "test");
            eventLoop.exec();
        }
    }


   void roomList(){
        auto rooms = client->joinedRoomList();
        for(auto const &room: rooms.toStdMap()){
            if(room.first == inviteRoomId) {
                return;
            }
        }
        QFAIL("Created room in the previous step not found!");
    }

    void joinRoom(){
        // join
        if(joinRoomId.isEmpty())
            QFAIL("room id is empty.");
        auto joinSignal = connect(client,&Client::joinedRoom,[&](const QString &roomID){
            QCOMPARE(roomID,joinRoomId);
            eventLoop.quit();
        });
        auto joinErrorSignal = connect(client,&Client::joinRoomFailed,[&](const QString &error){
            QFAIL(error.toStdString().c_str());
            eventLoop.quit();
        });
        client->joinRoom(joinRoomId);
        eventLoop.exec();
        disconnect(joinSignal);
        disconnect(joinErrorSignal);
    }

    void updateSync(){
        QCOMPARE(isUpdated,true);
    }

    void leaveRoom(){
        if(joinRoomId.isEmpty())
            QFAIL("room id is empty.");
        else {
            auto leftRoomSignal = connect(client, &Client::leftRoom,[&](const QString room_id){
                QCOMPARE(joinRoomId,room_id);
                eventLoop.quit();
            });

            auto leftRoomErrorSignal = connect(client, &Client::roomLeaveFailed,[&](const QString &error){
                QFAIL(error.toStdString().c_str());
                eventLoop.quit();
            });
            client->leaveRoom(joinRoomId);
            eventLoop.exec();
            disconnect(leftRoomSignal);
            disconnect(leftRoomErrorSignal);
        }
    }

    void deleteRoom(){
        if(inviteRoomId.isEmpty())
            QFAIL("room id is empty because of the previous test case (createRoom) failed");
        else {
            connect(client, &Client::leftRoom,[&](const QString room_id){
                QCOMPARE(inviteRoomId,room_id);
                eventLoop.quit();
            });

            connect(client, &Client::roomLeaveFailed,[&](const QString &error){
                QFAIL(error.toStdString().c_str());
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
        connect(client, &Client::logoutErrorOccurred,[&](const QString error){
            QFAIL(error.toStdString().c_str());
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
