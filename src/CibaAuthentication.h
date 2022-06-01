#pragma once

#include <QObject>
#include <QString>


struct RestRequestResponse
{
    QString jsonRespnse;
    int status;
};

class CibaAuthentication: public QObject{
 Q_OBJECT

public:
    CibaAuthentication();
    bool loginRequest(const QString &serverAddress, const QString &user);
    RestRequestResponse checkRegistration(QString accessToken);
    RestRequestResponse registeration(QString accessToken);
    RestRequestResponse login(QString accessToken,QString user);  
signals:
    void loginOk(const QString &accessToken,const QString &username);
    void loginError(const QString &message);

private:
    QString checkStatus(const QString &requestId);
    QString serverAddress;  
};
