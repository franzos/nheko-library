#ifndef PX_MATRIX_CLIENT_LIBRARY_H
#define PX_MATRIX_CLIENT_LIBRARY_H

#include <QObject>
#include <mtx/identifiers.hpp>
#include <mtx/responses/login.hpp>
#include <mtx/requests.hpp>
#include "MatrixClient.h"
#include "UserInformation.h"
#include "Features.h"
#if CIBA_AUTHENTICATION
#include <px-auth-lib-cpp/Authentication.h>   
#else
namespace PX::AUTH{
Q_NAMESPACE
enum class LOGIN_TYPE{
    UNKNOWN, // For compatibility with px-auth-library-cpp Enum
    PASSWORD,
};
Q_ENUM_NS(LOGIN_TYPE)
}
#endif 

class Authentication: public QObject{
 Q_OBJECT

public:
    Authentication(QObject *parent = nullptr);
    void loginWithPassword(std::string deviceName, std::string userId, std::string password, std::string serverAddress);    
    void logout();
    void serverDiscovery(std::string hostName);

signals:
    void loginOk(const mtx::responses::Login &res);
    void loginErrorOccurred(const std::string &msg);
    void logoutOk();    
    void logoutErrorOccurred(const std::string &msg);
    void serverChanged(const std::string &homeserver);
    void discoveryErrorOccurred(const std::string &err);
};


#endif //PX_MATRIX_CLIENT_LIBRARY_H