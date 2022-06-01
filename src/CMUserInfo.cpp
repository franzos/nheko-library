#include "CMUserInfo.h"
#include <QJsonArray>
#include <QMap>

CMUserInfo::CMUserInfo():
    _httpRequest(new HttpRequest(this)){
    connect(_httpRequest, &HttpRequest::response, [&](int code, const QString &response){
        if(code>=200 && code <300){
            QJsonDocument idJsonResponse = QJsonDocument::fromJson(response.toUtf8());
            QJsonObject jsonObj = idJsonResponse.object();
            if( jsonObj.contains("createdAt")               && jsonObj.contains("username") && 
                jsonObj.contains("usernameWithoutDomain")   && jsonObj.contains("email") && 
                jsonObj.contains("phone")                   && jsonObj.contains("title") && 
                jsonObj.contains("firstname")               && jsonObj.contains("lastname")){
            
            CMUserInformation info;
            info.createdAt = jsonObj["createdAt"].toString();
            info.createdAt = jsonObj["username"].toString();
            info.createdAt = jsonObj["usernameWithoutDomain"].toString();
            info.createdAt = jsonObj["email"].toString();
            info.createdAt = jsonObj["phone"].toString();
            info.createdAt = jsonObj["title"].toString();
            info.createdAt = jsonObj["firstname"].toString();
            info.createdAt = jsonObj["lastname"].toString();
            auto additionalInfoJson = jsonObj["additionalInfo"].toObject();
            if( additionalInfoJson.contains("localizedTitle")       && 
                additionalInfoJson.contains("localizedFirstName")   && 
                additionalInfoJson.contains("localizedLastName")){
                    info.localizedTitle     = additionalInfoJson["localizedTitle"].toString();
                    info.localizedFirstName = additionalInfoJson["localizedFirstName"].toString();
                    info.localizedLastName  = additionalInfoJson["localizedLastName"].toString();
                }
            emit userInfoUpdated(info);
            return;
            } 
        }
        emit userInfoFailure(response);
    });
}

void CMUserInfo::update(const QString &accessToken){
    QMap<QString, QString> headers;
    headers.insert("Content-Type", "application/json");
    headers.insert("Accept", "application/json");
    headers.insert("Authorization", "Bearer " + accessToken);
    _httpRequest->get("https://identity.pantherx.org/users/me",headers, true);
}