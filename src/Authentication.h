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
    void loginWithCiba(QString username,QString server,QString accessToken);
    void serverDiscovery(std::string hostName);
    QVariantMap availableLogin(const QString &server);

signals:
    void loginOk(const mtx::responses::Login &res);
    void loginErrorOccurred(const std::string &msg);
    void logoutOk();    
    void logoutErrorOccurred(const std::string &msg);
    void loginCibaOk(UserInformation userInfo);
    void loginCibaErrorOccurred(const std::string &msg);
    void serverChanged(const std::string &homeserver);
    void discoveryErrorOccurred(const std::string &err);

private slots:
    void loginCibaFlow(QString accessToken,QString username);
    
private:
    bool isCibaSupported(const QVariantMap &loginOpts);
    QString cibaServer;
    CibaAuthentication *_ciba; 
};


#endif //PX_MATRIX_CLIENT_LIBRARY_H