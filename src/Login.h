#ifndef PX_MATRIX_CLIENT_LIBRARY_H
#define PX_MATRIX_CLIENT_LIBRARY_H

#include <QObject>
#include "MatrixClient.h"

class Login{
 Q_OBJECT

public:
    void loginProcess(std::string deviceName, std::string userId, std::string password, std::string serverAddress);
    bool hasValidUser();
    mtx::responses::Login userInformation();


signals:
    void loginOk(const mtx::responses::Login &res);
    void errorOccurred(std::string &msg);


Private:


};


#endif //PX_MATRIX_CLIENT_LIBRARY_H