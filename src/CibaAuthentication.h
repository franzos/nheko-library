#ifndef MATRIX_CLIENT_LIBRARY_CIBA_AUTHENTICATION_H
#define MATRIX_CLIENT_LIBRARY_CIBA_AUTHENTICATION_H

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
    CibaAuthentication(QString server);
    RestRequestResponse availableLogin();
    RestRequestResponse loginRequest(QString user);
    RestRequestResponse checkStatus(QString requestId);
    RestRequestResponse checkRegistration(QString accessToken);
    RestRequestResponse registeration(QString accessToken);
    RestRequestResponse login(QString accessToken,QString user);  
private:
    QString serverAddress;
    
    
};

#endif //MATRIX_CLIENT_LIBRARY_CIBA_AUTHENTICATION_H

