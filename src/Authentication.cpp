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
#include "RestRequest.h"

Authentication::Authentication(QObject *parent): 
    QObject(parent), _ciba(new CibaAuthentication()){
        connect(_ciba,&CibaAuthentication::loginOk,this,&Authentication::loginCibaFlow);
        connect(_ciba,&CibaAuthentication::loginError,[&](const QString &message){
            emit loginCibaErrorOccurred(message.toStdString());
        });
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

void Authentication::loginWithCiba(QString username,QString server,QString accessToken){
    cibaServer = server;
    std::string errMsg;
    auto opts = availableLogin(server);
    if(!opts.isEmpty()){
        if(isCibaSupported(opts)) {
            _ciba->loginRequest(server, username, accessToken);
            return;
        } else {
            errMsg = "Ciba is not supported";
        }
    } else {
        errMsg = "Connection or Json error";
    }
    emit loginCibaErrorOccurred(errMsg);
}

void Authentication::loginCibaFlow(QString accessToken,QString username){
    auto res = _ciba->checkRegistration(accessToken);
    if(res.status == 200 || res.status == 201){
        QJsonDocument jsonResponse = QJsonDocument::fromJson(res.jsonRespnse.toUtf8());
        QJsonObject jsonObj = jsonResponse.object();
        if(!jsonObj.contains("exists"))
           _ciba->registeration(accessToken);
        auto loginRes = _ciba->login(accessToken,username);
        if(loginRes.status == 200 || loginRes.status == 201){
            UserInformation userInfo;
            QJsonDocument userJson = QJsonDocument::fromJson(loginRes.jsonRespnse.toUtf8());
            QJsonObject jsonObj = userJson.object();
            userInfo.accessToken = jsonObj["access_token"].toString();
            userInfo.userId = jsonObj["user_id"].toString();
            userInfo.cmUserId = username;
            userInfo.homeServer = cibaServer;
            userInfo.deviceId = jsonObj["device_id"].toString();
            emit loginCibaOk(userInfo);
        } else {
            std::string msg = "Connection error";
            emit loginCibaErrorOccurred(msg); 
        }
    } else {
        std::string msg = "Connection error";
        emit loginCibaErrorOccurred(msg); 
    }
}

void Authentication::cancelCibaLogin(){
    _ciba->cancel();
}

QVariantMap Authentication::availableLogin(const QString &server){
    RestRequest restRequest;
    QVariantMap opt;
    QString urlAvailableLogin = server+"/_matrix/client/r0/login";
    QString response;
    int resCode = restRequest.get( urlAvailableLogin , {}, response);
    if(resCode==200 || resCode==201){
        QJsonDocument jsonResponse = QJsonDocument::fromJson(response.toUtf8());
        auto object = jsonResponse.object();
        QJsonValue value = object.value("flows");
        QJsonArray array = value.toArray();
        foreach (const QJsonValue & v, array) {   
            QString type =  v.toObject().value("type").toString();
            if (type.contains("cm.ciba_auth")) {
                opt.insert("CIBA","Available");
            }else if(type.contains("m.login.password")){
                opt.insert("PASSWORD","Available");
            }
        }  
    }
    return opt;
}

bool Authentication::isCibaSupported(const QVariantMap &loginOpts){
    for(auto const &o: loginOpts.toStdMap()){
        if((o.first=="CIBA")){
            return true;
        }
    }
    return false;
}