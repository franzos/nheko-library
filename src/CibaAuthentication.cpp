#include "CibaAuthentication.h"
#include <QJsonArray>
#include <QMap>
#include "RestRequest.h"

RestRequestResponse CibaAuthentication::availableLogin(){
    RestRequestResponse output;
    RestRequest restRequest;
    QMap<QString, QString> headers;
    QString urlAvailableLogin = "https://matrix.pantherx.dev/_matrix/client/r0/login";
    output.status = restRequest.get( urlAvailableLogin , headers, output.jsonRespnse);
    // QJsonDocument jsonResponse = QJsonDocument::fromJson(response.toUtf8());
    // if(jsonResponse.isArray()){
    //     QJsonArray jsonArray = jsonResponse.array();
    //     foreach (const QJsonValue & value, jsonArray) {
    //         QJsonObject obj = value.toObject();
    //         QString type = obj["type"].toString();
    //         if (type.contains("cm.ciba_auth"))
    //         {
    //             return true;
    //         }
    //     }           
    // }
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
    QString urlLoginRequest = "https://matrix.pantherx.dev/_synapse/client/cm_login/ciba";
    output.status = restRequest.post( urlLoginRequest , headers,items, data, output.jsonRespnse);
    // QJsonDocument jsonResponse = QJsonDocument::fromJson(response.toUtf8());
    // QString requestId = "";
    // QJsonObject jsonObj = jsonResponse.object();
    // if(jsonObj.contains("auth_req_id"))
    //     requestId = jsonObj["auth_req_id"].toString();
    return output;
}

RestRequestResponse CibaAuthentication::checkStatus(QString requestId){
    RestRequest restRequest;
    RestRequestResponse output;
    QMap<QString, QString> headers;
    QString urlCheckStatus = "https://matrix.pantherx.dev/_synapse/client/cm_login/ciba_status/"+requestId;
    output.status= restRequest.get(urlCheckStatus,headers,output.jsonRespnse);
    // QJsonDocument jsonResponse = QJsonDocument::fromJson(response.toUtf8());
    // QJsonObject jsonObj = jsonResponse.object();
    // if(jsonObj.contains("access_token")){
    //    return jsonObj["access_token"].toString();
    // }else{
    //     return "pending";
    // }
    return output;
}

RestRequestResponse CibaAuthentication::checkRegistration(QString accessToken){
    RestRequest restRequest;
    RestRequestResponse output;
    QMap<QString, QString> headers;
    QString urlRegistration = "https://matrix.pantherx.dev/_synapse/client/cm_login/exists";
    headers.insert("Content-Type", "application/json");   
    headers.insert("Authorization", "Bearer " + accessToken);  
    output.status = restRequest.get(urlRegistration,headers,output.jsonRespnse);
    // QJsonDocument jsonResponse = QJsonDocument::fromJson(response.toUtf8());
    // QJsonObject jsonObj = jsonResponse.object();
    // if(jsonObj.contains("exists")){
    //    return true;
    // }
    
    return output;
}

RestRequestResponse CibaAuthentication::registeration(QString accessToken){
    RestRequest restRequest;
    RestRequestResponse output;
    QMap<QString, QString> headers;
    QString urlRegistration = "https://matrix.pantherx.dev/_synapse/client/cm_login/register";
    headers.insert("Content-Type", "application/json");   
    headers.insert("Authorization", "Bearer " + accessToken);    
   output.status= restRequest.get(urlRegistration,headers,output.jsonRespnse);
   return output;
}

RestRequestResponse CibaAuthentication::login(QString accessToken,QString user){
    RestRequest restRequest;
    RestRequestResponse output;
    QMap<QString, QString> headers;
    QMap<QString, QString> data;
    QMap<QString, QString> items;
    headers.insert("Content-Type", "application/json");   
    data.insert("type","cm.ciba_auth");       
    data.insert("token",accessToken); 
    QString identifire = "{\"type\":\"m.id.user\",\"user\":\""+user+"\"}";
    data.insert("identifier",identifire);      
    QString urlLoginRequest = "https://matrix.pantherx.dev/_matrix/client/r0/login";
    output.status = restRequest.post( urlLoginRequest , headers,items, data, output.jsonRespnse);
    return output;
}