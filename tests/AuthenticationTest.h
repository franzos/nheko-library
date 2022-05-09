#include <QtTest/QtTest>
#include <QEventLoop>
#include "../src/Authentication.h"
#include "../src/Logging.h"



class AuthenticationTest: public QObject
{
    Q_OBJECT
    Authentication *loginTest;
    QEventLoop eventLoop;
    
private slots:

    void initTestCase(){
        // client = Client::instance();
        // UserSettings::instance()->clear();
        nhlog::init("matrix-client-library", true, true);   
    }
    void loginWithCorrectPassword(){
        loginTest = new Authentication();
        //QEventLoop eventLoop;
        QObject::connect(loginTest,  &Authentication::loginOk, [&](const mtx::responses::Login &res){
            QCOMPARE(res.user_id.localpart(),"fakhri_test01");            
            eventLoop.quit();
        });
        QObject::connect(loginTest,  &Authentication::loginErrorOccurred, [&](const std::string &out){
            QFAIL(out.c_str());
            eventLoop.quit();
        });      
         
        QObject::connect(loginTest,  &Authentication::logoutErrorOccurred, [&](const std::string &out){
            QFAIL(out.c_str());
            eventLoop.quit();
        });      


        std::string deviceName = "test";
        std::string userId = "@fakhri_test01:pantherx.org";
        std::string password = "a2bqy9iHU8";
        std::string serverAddress = "https://matrix.pantherx.org";   
        loginTest->loginWithPassword(deviceName, userId, password, serverAddress); 
        eventLoop.exec();  
         
    }
    
    void logout(){  
        QObject::connect(loginTest,  &Authentication::logoutOk, [&](){
            QVERIFY(1 == 1);
            eventLoop.quit();
        });
        loginTest->logout();        
        eventLoop.exec();
    }    

     void discovery(){      

        QObject::connect(loginTest,  &Authentication::serverChanged, [&](std::string server){
            QCOMPARE(server,"https://matrix.ones-now.com");
            eventLoop.quit();
        }); 

        QObject::connect(loginTest,  &Authentication::discoverryErrorOccurred, [&](std::string server){
            
            eventLoop.quit();
        }); 

        loginTest->serverDiscovery("@huser.test-703:ones-now.com");
        eventLoop.exec();     
    }

    void loginWithIncorrectPassword(){
        Authentication *loginTestWrong = new Authentication();
        QEventLoop eventLoop;
        QObject::connect(loginTestWrong,  &Authentication::loginOk, [&](const mtx::responses::Login &res){
            QFAIL(("Logined in invalid user "+res.user_id.localpart()).c_str() );
            eventLoop.quit();
        });

        QObject::connect(loginTestWrong,  &Authentication::loginErrorOccurred, [&](const std::string &out){            
            qDebug()<<"INVALID Password";       
            eventLoop.quit();
        });

        std::string deviceName = "test";
        std::string userId = "@fakhri_test01:pantherx.org";
        std::string password = "a2bqy9iHU88";
        std::string serverAddress = "https://matrix.pantherx.org";   
        loginTestWrong->loginWithPassword(deviceName, userId, password, serverAddress); 
        eventLoop.exec();        
    }
    
// void loginWithCorrectCiba(){
//         auto loginCibaTest = new Authentication();
//         //QEventLoop eventLoop;
//         QObject::connect(loginCibaTest,  &Authentication::loginCibaOk, [&](UserInformation res){            
//             eventLoop.quit();
//         });

 
//         QObject::connect(loginCibaTest,  &Authentication::loginCibaErrorOccurred, [&](const std::string &out){
//             QFAIL(out.c_str());
//             eventLoop.quit();
//         });              
      

//         QString userId = "ff.ss@pantherx.org";
//         QString server = "https://matrix.pantherx.dev";
//         loginCibaTest->loginWithCiba(userId,server); 
//         eventLoop.exec();  
         
//     }
       
};
