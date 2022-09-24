#include "CibaAuthentication.h"
#include <QJsonArray>
#include <QMap>
#include <QThread>
#include <unistd.h>
#include "RestRequest.h"
#include "Logging.h"

#define CHECK_STATUS_TIMEOUT    5000

CibaAuthentication::CibaAuthentication():
    _checkStatusTimer(new QTimer(this)){
        connect(_checkStatusTimer, &QTimer::timeout, [&](){
            auto accessToken = checkStatus(_requestId);    
            if(!accessToken.isEmpty()){
                nhlog::net()->info("CIBA login done for " + _username.toStdString());
                _checkStatusTimer->stop();
                emit loginOk(accessToken, _username);
            }
        });
}

void CibaAuthentication::loginRequest(const QString &serverAddress, const QString &username, const QString &accessToken){
    RestRequest restRequest;
    QString requestId = "";
    QMap<QString, QString> headers;
    QMap<QString, QString> data;
    headers.insert("Content-Type", "application/json");   
    data.insert("login_hint_token", username);       
    _serverAddress = serverAddress;
    if(!accessToken.isEmpty()){
        emit loginOk(accessToken, username);
        return;
    }
    QString urlLoginRequest = _serverAddress+"/_synapse/client/cm_login/ciba";
    QString response;
    int resCode = restRequest.post( urlLoginRequest , headers,{}, data, response);
    if(resCode==200 || resCode==201) {
        QJsonDocument idJsonResponse = QJsonDocument::fromJson(response.toUtf8());
        QJsonObject jsonObj = idJsonResponse.object();
        if(jsonObj.contains("auth_req_id")) {
            requestId = jsonObj["auth_req_id"].toString();
            if(!requestId.isEmpty()) {
                _username  = username;
                _requestId = requestId;
                _checkStatusTimer->start(CHECK_STATUS_TIMEOUT);
                return;  
            }
        }
        response = "Response error (" + response + ")";
    }
    emit loginError(response);
}

QString CibaAuthentication::checkStatus(const QString &requestId){
    RestRequest restRequest;
    QString response;
    QMap<QString, QString> headers;
    QString urlCheckStatus = _serverAddress+"/_synapse/client/cm_login/ciba_status/"+requestId;
    restRequest.get(urlCheckStatus,headers,response);
    QJsonDocument statusResponse = QJsonDocument::fromJson(response.toUtf8());
    QJsonObject jsonObj = statusResponse.object();
    if(jsonObj.contains("access_token")){
        return jsonObj["access_token"].toString();
    }
    return "";
}

RestRequestResponse CibaAuthentication::checkRegistration(QString accessToken){
    RestRequest restRequest;
    RestRequestResponse output;
    QMap<QString, QString> headers;
    QString urlRegistration = _serverAddress+"/_synapse/client/cm_login/exists";
    headers.insert("Content-Type", "application/json");   
    headers.insert("Authorization", "Bearer " + accessToken);  
    output.status = restRequest.get(urlRegistration,headers,output.jsonRespnse);    
    return output;
}

RestRequestResponse CibaAuthentication::registeration(QString accessToken){
    RestRequest restRequest;
    RestRequestResponse output;
    QMap<QString, QString> headers;
    QString urlRegistration = _serverAddress+"/_synapse/client/cm_login/register";
    headers.insert("Content-Type", "application/json");   
    headers.insert("Authorization", "Bearer " + accessToken);    
    output.status= restRequest.get(urlRegistration,headers,output.jsonRespnse);
    return output;
}

RestRequestResponse CibaAuthentication::login(QString accessToken,QString user){
    RestRequest restRequest;
    RestRequestResponse output;
    QMap<QString, QString> headers;
    QMap<QString, QString> items;
    headers.insert("Content-Type", "application/json");  
    QString data = "{\"type\":\"cm.ciba_auth\",\"identifier\":{\"type\":\"m.id.user\",\"user\":\""+user+"\"},\"token\":\""+accessToken+"\"}";
      
    QString urlLoginRequest = _serverAddress+"/_matrix/client/r0/login";
    output.status = restRequest.post( urlLoginRequest , headers, data, output.jsonRespnse);
    return output;
}

void CibaAuthentication::cancel(){
    _checkStatusTimer->stop();
}
