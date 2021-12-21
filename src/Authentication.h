#ifndef PX_MATRIX_CLIENT_LIBRARY_H
#define PX_MATRIX_CLIENT_LIBRARY_H

#include <QObject>
#include <mtx/identifiers.hpp>
#include <mtx/responses/login.hpp>
#include <mtx/requests.hpp>
#include "MatrixClient.h"

class Authentication: public QObject{
 Q_OBJECT

public:
    void loginWithUsername(std::string deviceName, std::string userId, std::string password, std::string serverAddress);
    bool hasValidUser();
    mtx::responses::Login userInformation();

signals:
    void loginOk(const mtx::responses::Login &res);
    void errorOccurred(std::string &msg);
    
};


#endif //PX_MATRIX_CLIENT_LIBRARY_H