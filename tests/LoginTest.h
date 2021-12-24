#include <QtTest/QtTest>
#include <QEventLoop>
#include "../src/Authentication.h"
#include <iostream>



class LoginTest: public QObject
{
    Q_OBJECT
    Authentication *loginTest;
    QEventLoop eventLoop;
private slots:
    void loginWithCorrectPassword(){
         loginTest = new Authentication;
        //QEventLoop eventLoop;
        QObject::connect(loginTest,  &Authentication::loginOk, [&](const mtx::responses::Login &res){
            QCOMPARE(res.user_id.localpart(),"fakhri_test01");            
            eventLoop.quit();
        });
        QObject::connect(loginTest,  &Authentication::errorOccurred, [&](const std::string &out){
            QFAIL(out.c_str());
            eventLoop.quit();
        });        


        std::string deviceName = "test";
        std::string userId = "@fakhri_test01:pantherx.org";
        std::string password = "a2bqy9iHU8";
        std::string serverAddress = "https://matrix.pantherx.org";   
        loginTest->serverDiscovery(userId);
        loginTest->loginWithPassword(deviceName, userId, password, serverAddress); 
        eventLoop.exec();        
    }
     void logout(){        
        loginTest->logout(); 
        QObject::connect(loginTest,  &Authentication::logoutOk, [&](){
            QVERIFY(1 == 1);
            eventLoop.quit();
        });

     }

    void loginWithIncorrectPassword(){
        Authentication *loginTestWrong = new Authentication;
        //QEventLoop eventLoop;
        QObject::connect(loginTestWrong,  &Authentication::loginOk, [&](const mtx::responses::Login &res){
            QFAIL("Logined in invalid user");
            eventLoop.quit();
        });
        QObject::connect(loginTestWrong,  &Authentication::errorOccurred, [&](const std::string &out){            
            QCOMPARE(out,"Invalid password");
            eventLoop.quit();
        });

        std::string deviceName = "test";
        std::string userId = "@fakhri_test01:pantherx.org";
        std::string password = "a2bqy9iHU888";
        std::string serverAddress = "https://matrix.pantherx.org";   
        loginTest->loginWithPassword(deviceName, userId, password, serverAddress); 
        eventLoop.exec();        
    }
    
};
