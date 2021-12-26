#include <QtTest/QtTest>
#include <QEventLoop>
#include "../src/Client.h"
#include <iostream>



class AuthenticationTest: public QObject
{
    Q_OBJECT
    Client *loginTest;
    QEventLoop eventLoop;
private slots:
    void loginWithCorrectPassword(){
         loginTest = Client::instance();
        //QEventLoop eventLoop;
        QObject::connect(loginTest,  &Client::loginReady, [&](const mtx::responses::Login &res){
            QCOMPARE(res.user_id.localpart(),"fakhri_test01");            
            eventLoop.quit();
        });
        QObject::connect(loginTest,  &Client::loginErrorOccurred, [&](const std::string &out){
            QFAIL(out.c_str());
            eventLoop.quit();
        });      
         
        QObject::connect(loginTest,  &Client::logoutErrorOccurred, [&](const std::string &out){
            QFAIL(out.c_str());
            eventLoop.quit();
        });      


        std::string deviceName = "test";
        std::string userId = "@fakhri_test01:pantherx.org";
        std::string password = "a2bqy9iHU8";
        std::string serverAddress = "https://matrix.pantherx.org";   
        // loginTest->serverDiscovery(userId);
        loginTest->loginWithPassword(deviceName, userId, password, serverAddress); 
        eventLoop.exec();        
    }
    // void discovery(){        
    //     std::string server = loginTest->serverDiscovery("@fakhri_test01:pantherx.org");
    //     QCOMPARE(server,"https://matrix.pantherx.org");  
    //     eventLoop.quit();     
    // }
    void logout(){        
        loginTest->logout(); 
        QObject::connect(loginTest,  &Client::logoutOk, [&](){
            QVERIFY(1 == 1);
            eventLoop.quit();
        });

     }

    void loginWithIncorrectPassword(){
    //     Client *loginTestWrong;
    //   loginTestWrong = Client::instance();
        //QEventLoop eventLoop;
        // QObject::connect(loginTest,  &Client::loginReady, [&](const mtx::responses::Login &res){
        //     QFAIL("Logined in invalid user");
        //     eventLoop.quit();
        // });
        // QObject::connect(loginTest,  &Client::loginErrorOccurred, [&](const std::string &out){            
        //    QCOMPARE(out,"Invalid password");
        //     eventLoop.quit();
        // });

        std::string deviceName = "test";
        std::string userId = "@fakhri_test01:pantherx.org";
        std::string password = "a2bqy9iHU8";
        std::string serverAddress = "https://matrix.pantherx.org";   
        loginTest->loginWithPassword(deviceName, userId, password, serverAddress); 
        eventLoop.exec();        
    }
    
};
