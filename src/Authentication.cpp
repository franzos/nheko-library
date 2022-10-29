#include "Authentication.h"
#include <QDebug>
#include <iostream>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QThread>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include "Utils.h"

Authentication::Authentication(QObject *parent): 
    QObject(parent){
}

void Authentication::loginWithPassword(std::string deviceName, std::string userId, std::string password, std::string serverAddress){
    if((userId[0] != '@') || (userId.find(':') == std::string::npos) ){
        std::string s ="The UerId format is wrong ";
        emit loginErrorOccurred(s);
        return;
    }
    http::client()->set_server(serverAddress);
    mtx::identifiers::User user;
    user = mtx::identifiers::parse<mtx::identifiers::User>(userId);        
    http::client()->login(          
        user.localpart(),
        password,
        deviceName,
        [this](const mtx::responses::Login &res, mtx::http::RequestErr err) {
            if (err) {
                auto s = utils::httpMtxErrorToString(err).toStdString();
                emit loginErrorOccurred(s);
                return;
            }
            emit loginOk(res);
    });

}

void Authentication::logout(){
    http::client()->logout([this](const mtx::responses::Logout &, mtx::http::RequestErr err) {
        if (err) {
            auto s = utils::httpMtxErrorToString(err).toStdString();     
            emit logoutErrorOccurred(s);
            return;
        }        
        emit logoutOk();
    });
}

void Authentication::serverDiscovery(std::string hostName){   
    http::client()->set_server(hostName);
    http::client()->well_known([this,hostName](const mtx::responses::WellKnown &res, mtx::http::RequestErr err) {             
        if (err) {
            auto s = utils::httpMtxErrorToString(err).toStdString();     
            emit discoveryErrorOccurred(s);
            nhlog::net()->error("Autodiscovery failed. " + s);  
        } else{
            nhlog::net()->info("Autodiscovery: Discovered '" + res.homeserver.base_url + "'");
            http::client()->set_server(res.homeserver.base_url);
            emit serverChanged(res.homeserver.base_url);
        }
    });
}
