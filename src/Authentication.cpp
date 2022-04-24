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
#include "CibaAuthentication.h"

Authentication::Authentication(QObject *parent): QObject(parent) {
    connect(this,&Authentication::cibaStatusChanged,this,&Authentication::loginCibaFlow);
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
        if (res.well_known) {
            http::client()->set_server(res.well_known->homeserver.base_url);
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



std::string Authentication::serverDiscovery(std::string userId){
    mtx::identifiers::User user;    
    try {
        user = mtx::identifiers::parse<mtx::identifiers::User>(userId);
    } catch (const std::exception &) {
        // showError(error_matrixid_label_,
        //           tr("You have entered an invalid Matrix ID  e.g @joe:matrix.org"));
        return " ";
    }

    QString homeServer = QString::fromStdString(user.hostname());
    http::client()->set_server(user.hostname());
    http::client()->well_known([this](const mtx::responses::WellKnown &res, mtx::http::RequestErr err) {             
        if (err) {
            if (err->status_code == 404) {
                nhlog::net()->info("Autodiscovery: No .well-known.");
                // checkHomeserverVersion();
                return;
            }

            if (!err->parse_error.empty()) {
                // emit versionErrorCb(tr("Autodiscovery failed. Received malformed response."));
                nhlog::net()->error("Autodiscovery failed. Received malformed response.");
                return;
            }

            // emit versionErrorCb(tr("Autodiscovery failed. Unknown error when "
            //                        "requesting .well-known."));
            nhlog::net()->error("Autodiscovery failed. Unknown error when "
                                "requesting .well-known. {} {}",
                                err->status_code,
                                err->error_code);
            
            return;
        }

        nhlog::net()->info("Autodiscovery: Discovered '" + res.homeserver.base_url + "'");
        //http::client()->set_server(res.homeserver.base_url);
        //homeServer = res.homeserver.base_url;
    });
    return homeServer.toStdString();
}

bool Authentication::loginWithCiba(QString username,QString server){
    cibaServer = server;
    ciba = new CibaAuthentication(server);
    auto res = ciba->availableLogin();
    if(res.status==200 || res.status==201){
        if(isCibaSupported(res.jsonRespnse)) {   
            auto idResp = ciba->loginRequest(username);
            if(idResp.status==200 || idResp.status==201) {
                QJsonDocument idJsonResponse = QJsonDocument::fromJson(idResp.jsonRespnse.toUtf8());
                QString requestId = "";
                QJsonObject jsonObj = idJsonResponse.object();
                if(jsonObj.contains("auth_req_id")) {
                    requestId = jsonObj["auth_req_id"].toString();
                    auto thread = new QThread();
                    auto context = new QObject() ;
                    context->moveToThread(thread);
                    QObject::connect(thread, &QThread::started, context, [&,requestId,username]() { 
                        bool isPenging = true;
                        while(isPenging){                                    
                            auto statusRsp = ciba->checkStatus(requestId);    
                            QJsonDocument statusResponse = QJsonDocument::fromJson(statusRsp.jsonRespnse.toUtf8());
                            QJsonObject jsonObj = statusResponse.object();
                            if(jsonObj.contains("access_token")){
                                QString token = jsonObj["access_token"].toString();
                                emit cibaStatusChanged(token,username); 
                                isPenging = false;
                            }else{
                                sleep(5);
                            }   
                        }        
                    });
                    thread->start();
                    //thread.quit();
                    // thread.wait();  
                    return true;                 
                }
            }
        } else {
            std::string msg = "Ciba is not supported";
            emit loginCibaErrorOccurred(msg);
        }
    }  
    std::string msg = "Connection or Json error";
    emit loginCibaErrorOccurred(msg);
    return false;  
}

void Authentication::loginCibaFlow(QString accessToken,QString username){
    auto res = ciba->checkRegistration(accessToken);
    if(res.status == 200 || res.status == 201){
        QJsonDocument jsonResponse = QJsonDocument::fromJson(res.jsonRespnse.toUtf8());
        QJsonObject jsonObj = jsonResponse.object();
        if(!jsonObj.contains("exists"))
           ciba->registeration(accessToken);
        auto loginRes = ciba->login(accessToken,username);
        if(loginRes.status == 200 || loginRes.status == 201){
            UserInformation userInfo;
            QJsonDocument userJson = QJsonDocument::fromJson(loginRes.jsonRespnse.toUtf8());
            QJsonObject jsonObj = userJson.object();
            userInfo.accessToken = jsonObj["access_token"].toString();
            userInfo.userId = jsonObj["user_id"].toString();
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

bool Authentication::isCibaSupported(QString data){
    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8());
    auto object = jsonResponse.object();
    QJsonValue value = object.value("flows");
    QJsonArray array = value.toArray();
    foreach (const QJsonValue & v, array) {   
        QString type =  v.toObject().value("type").toString();
        if (type.contains("cm.ciba_auth"))
            return true;
    }
    return false;
}