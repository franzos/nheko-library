#ifndef PX_MATRIX_CLIENT_LIBRARY_H
#define PX_MATRIX_CLIENT_LIBRARY_H

#include <QObject>
#include <mtx/identifiers.hpp>
#include <mtx/responses/login.hpp>
#include <mtx/requests.hpp>
#include "MatrixClient.h"
#include "CibaAuthentication.h"
#include "UserInformation.h"

class Authentication: public QObject{
 Q_OBJECT

public:
    Authentication(QObject *parent = nullptr);
    void loginWithPassword(std::string deviceName, std::string userId, std::string password, std::string serverAddress);    
    void logout();
    bool loginWithCiba(QString username,QString server);
    void serverDiscovery(std::string userId);

signals:
    void loginOk(const mtx::responses::Login &res);
    void loginErrorOccurred(std::string &msg);
    void logoutErrorOccurred(std::string &msg);
    void logoutOk();    
    void loginCibaOk(UserInformation userInfo);
    void loginCibaErrorOccurred(std::string &msg);
    void cibaStatusChanged(QString accessToken,QString username);
    void serverChanged(std::string homeserver);

private slots:
    void loginCibaFlow(QString accessToken,QString username);
private:
    CibaAuthentication *ciba; 
    bool isCibaSupported(QString data);
    QString cibaServer;
};


#endif //PX_MATRIX_CLIENT_LIBRARY_H