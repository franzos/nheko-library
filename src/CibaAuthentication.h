#pragma once

#include <QObject>
#include <QString>
#include <QTimer>


struct RestRequestResponse
{
    QString jsonRespnse;
    int status;
};

class CibaAuthentication: public QObject{
 Q_OBJECT

public:
    CibaAuthentication();
    void loginRequest(const QString &serverAddress, const QString &user, const QString &accessToken = "");
    RestRequestResponse checkRegistration(QString accessToken);
    RestRequestResponse registeration(QString accessToken);
    RestRequestResponse login(QString accessToken,QString user);  
    void cancel();

signals:
    void loginOk(const QString &accessToken,const QString &username);
    void loginError(const QString &message);

private:
    QString checkStatus(const QString &requestId);
    QString _serverAddress;  
    QString _requestId;
    QString _username;
    QTimer  *_checkStatusTimer;
};
