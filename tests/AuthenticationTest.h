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
        QObject::connect(loginTest,  &Authentication::serverChanged, [&](const std::string &server){
            QCOMPARE(server,"https://matrix.pantherx.org");
            eventLoop.quit();
        }); 

        QObject::connect(loginTest,  &Authentication::discoveryErrorOccurred, [&](const std::string error){
            qDebug() << QString::fromStdString(error);
            eventLoop.quit();
        }); 

        std::string userId ="@fakhri_test01:pantherx.org";

         mtx::identifiers::User user;    
        // try {
            user = mtx::identifiers::parse<mtx::identifiers::User>(userId);
        // } catch (const std::exception &) {                
        //     emit discoveryErrorOccurred("You have entered an invalid Matrix ID");
        // }
        loginTest->serverDiscovery(user.hostname());
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
            qDebug()<<"INVALID Password" << QString::fromStdString(out);       
            eventLoop.quit();
        });

        std::string deviceName = "test";
        std::string userId = "@fakhri_test01:pantherx.org";
        std::string password = "a2bqy9iHU88";
        std::string serverAddress = "https://matrix.pantherx.org";   
        loginTestWrong->loginWithPassword(deviceName, userId, password, serverAddress); 
        eventLoop.exec();        
    }       
};
