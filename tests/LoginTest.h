#include <QtTest/QtTest>
#include <QEventLoop>
#include "../src/Authentication.h"
#include <iostream>



class LoginTest: public QObject
{
    Q_OBJECT
private slots:
    void loginTurue(){
        Authentication *loginTest = new Authentication;
        QEventLoop eventLoop;
        QObject::connect(loginTest,  &Authentication::loginOk, [&](const mtx::responses::Login &res){
            QCOMPARE(res.user_id.localpart(),"hamzeh_test02");
             eventLoop.quit();
        });
        QObject::connect(loginTest,  &Authentication::errorOccurred, [&](const std::string &out){
            QFAIL(out.c_str());
            eventLoop.quit();
        });

        std::string deviceName = "test";
        std::string userId = "@hamzeh_test02:pantherx.org";
        std::string password = "5wn685g7mN";
        std::string serverAddress = "https://matrix.pantherx.org";   
        loginTest->loginWithUsername(deviceName, userId, password, serverAddress); 
        eventLoop.exec();        
    }
    void loginFaile(){
        Authentication *loginTest = new Authentication;
        QEventLoop eventLoop;
        QObject::connect(loginTest,  &Authentication::loginOk, [&](const mtx::responses::Login &res){
            QFAIL("Logined in invalid user");
            eventLoop.quit();
        });
        QObject::connect(loginTest,  &Authentication::errorOccurred, [&](const std::string &out){            
            QCOMPARE(out,"Invalid password");
            eventLoop.quit();
        });

        std::string deviceName = "test";
        std::string userId = "@hamzeh_test02:pantherx.org";
        std::string password = "5wn685g7mNmm";
        std::string serverAddress = "https://matrix.pantherx.org";   
        loginTest->loginWithUsername(deviceName, userId, password, serverAddress); 
        eventLoop.exec();        
    }
    
};
