#ifndef PX_MATRIX_CLIENT_LIBRARY_H
#define PX_MATRIX_CLIENT_LIBRARY_H

#include <QObject>
#include <mtx/identifiers.hpp>
#include <mtx/responses/login.hpp>
#include <mtx/requests.hpp>
#include "MatrixClient.h"
#include "CibaAuthentication.h"

struct UserInformation{
    QString userId;
    QString accessToken;
    QString deviceId;
    QString homeServer;
} ;

class Authentication: public QObject{
 Q_OBJECT

public:
    void loginWithPassword(std::string deviceName, std::string userId, std::string password, std::string serverAddress);    
    void logout();
    bool loginWithCiba(QString username,QString server);
    std::string serverDiscovery(std::string userId);

signals:
    void loginOk(const mtx::responses::Login &res);
    void loginErrorOccurred(std::string &msg);
    void logoutErrorOccurred(std::string &msg);
    void logoutOk();    
    void loginCibaOk(UserInformation userInfo);
    void loginCibaErrorOccurred(std::string &msg);
    void cibaStatusChanged(QString accessToken,QString username);

private slots:
    void loginCibaFlow(QString accessToken,QString username);
private:
   CibaAuthentication *ciba; 
   bool isCibaSupported(QString data);
};


#endif //PX_MATRIX_CLIENT_LIBRARY_H