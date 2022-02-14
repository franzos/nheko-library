#ifndef MATRIX_CLIENT_LIBRARY_CIBA_AUTHENTICATION_H
#define MATRIX_CLIENT_LIBRARY_CIBA_AUTHENTICATION_H

#include <QObject>
#include <QString>

class CibaAuthentication: public QObject{
 Q_OBJECT

public:

    bool availableLogin();
    QString loginRequest(QString user);
    QString checkStatus(QString requestId);
    bool checkRegistration(QString accessToken);
    void registeration(QString accessToken);
    QString login(QString accessToken);    
    
};

#endif //MATRIX_CLIENT_LIBRARY_CIBA_AUTHENTICATION_H

