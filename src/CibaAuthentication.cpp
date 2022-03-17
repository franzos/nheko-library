#include "CibaAuthentication.h"
#include <QJsonArray>
#include <QMap>
#include "RestRequest.h"

CibaAuthentication::CibaAuthentication(QString server){
    serverAddress = server;
}

RestRequestResponse CibaAuthentication::availableLogin(){
    RestRequestResponse output;
    RestRequest restRequest;
    QMap<QString, QString> headers;
    QString urlAvailableLogin = serverAddress+"/_matrix/client/r0/login";
    output.status = restRequest.get( urlAvailableLogin , headers, output.jsonRespnse);
    return output;
}

RestRequestResponse CibaAuthentication::loginRequest(QString user){
    RestRequest restRequest;
    RestRequestResponse output;
    QMap<QString, QString> headers;
    QMap<QString, QString> data;
    QMap<QString, QString> items;
    headers.insert("Content-Type", "application/json");   
    data.insert("login_hint_token", user);       
    QString urlLoginRequest = serverAddress+"/_synapse/client/cm_login/ciba";
    output.status = restRequest.post( urlLoginRequest , headers,items, data, output.jsonRespnse);
    return output;
}

RestRequestResponse CibaAuthentication::checkStatus(QString requestId){
    RestRequest restRequest;
    RestRequestResponse output;
    QMap<QString, QString> headers;
    QString urlCheckStatus = serverAddress+"/_synapse/client/cm_login/ciba_status/"+requestId;
    output.status= restRequest.get(urlCheckStatus,headers,output.jsonRespnse);
    return output;
}

RestRequestResponse CibaAuthentication::checkRegistration(QString accessToken){
    RestRequest restRequest;
    RestRequestResponse output;
    QMap<QString, QString> headers;
    QString urlRegistration = serverAddress+"/_synapse/client/cm_login/exists";
    headers.insert("Content-Type", "application/json");   
    headers.insert("Authorization", "Bearer " + accessToken);  
    output.status = restRequest.get(urlRegistration,headers,output.jsonRespnse);    
    return output;
}

RestRequestResponse CibaAuthentication::registeration(QString accessToken){
    RestRequest restRequest;
    RestRequestResponse output;
    QMap<QString, QString> headers;
    QString urlRegistration = serverAddress+"/_synapse/client/cm_login/register";
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
      
    QString urlLoginRequest = serverAddress+"/_matrix/client/r0/login";
    output.status = restRequest.post( urlLoginRequest , headers, data, output.jsonRespnse);
    return output;
}