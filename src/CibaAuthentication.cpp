#include "CibaAuthentication.h"
#include <QJsonArray>
#include <QMap>
#include "RestRequest.h"

bool availableLogin(){
    RestRequest restRequest;
    QMap<QString, QString> headers;
    QString response;         
    QString urlGetSync = "https://matrix.pantherx.dev/_matrix/client/r0/login";
    auto result = restRequest.get( urlGetSync , headers, response);
    QJsonDocument jsonResponse = QJsonDocument::fromJson(response.toUtf8());
    if(jsonResponse.isArray()){
        QJsonArray jsonArray = jsonResponse.array();
        foreach (const QJsonValue & value, jsonArray) {
            QJsonObject obj = value.toObject();
            QString type = obj["type"].toString();
            if (type.contains("cm.ciba_auth"))
            {
                return true;
            }
        }           
    }
return false;
}