// Copyright © 2020 Hamzeh Nasajpour <h.nasajpour@pantherx.org> | PantherX.DEV
#include <QFileInfo>
#include <QDebug>

#include "HttpRequest.h"

HttpRequest::HttpRequest(QObject *parent) : QObject(parent){
    connect(&_manager, &QNetworkAccessManager::finished, this , &HttpRequest::networkReplyHandler);
}

QString HttpRequest::qmapToString(const QMap<QString, QString> &map){
    QString result;
    for (auto i = map.begin(); i != map.end(); ++i){
        result += "      - " + i.key() + " : " + i.value() + "\n";
    }
    return result;
}
        
QNetworkRequest HttpRequest::setRequestHeaders(const QString &urlString, const QMap<QString, QString> &headers) {
    QNetworkRequest request;
    QUrl url {urlString};
    request.setUrl(url);
    QMapIterator<QString, QString> headersIterator(headers);
    while (headersIterator.hasNext()) {
        headersIterator.next();
        request.setRawHeader(
            QByteArray::fromStdString(headersIterator.key().toStdString()),
            QByteArray::fromStdString(headersIterator.value().toStdString()));
    }
    return request;
}

void HttpRequest::networkReplyHandler(QNetworkReply *reply){
    int httpCode = 0;
    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (statusCode.isValid())
        httpCode = statusCode.toInt();
    QString res = QString::fromUtf8(reply->readAll());
    if(_debugMode){
        QString reason = reply->attribute( QNetworkRequest::HttpReasonPhraseAttribute ).toString();
        QString dbg = " < " + _queryString + res + " (" + reason + ":" + QString::number(httpCode) + ")";
        if(reply->error() != QNetworkReply::NoError) 
            qWarning() << dbg;
        else
            qDebug() << dbg;
    }
    emit response(httpCode, res); 
}

void HttpRequest::post    (const QString &url, 
                const QMap<QString, QString> &headers, 
                const QMap<QString, QString>& queryItems, 
                const QString &data, bool debug) {
    _debugMode = debug;
    if(_debugMode){
        // LOG
        _queryString = " * POST " + url + "\n";
        if(headers.size())      _queryString +="   + Headers   :\n" + qmapToString(headers);
        if(queryItems.size())   _queryString +="   + Items     :\n" + qmapToString(queryItems);
        if(data.size())         _queryString +="   + Data      :\n" + data;
        _queryString += "   + " + QSslSocket::sslLibraryVersionString() + "\n";
    }
    QNetworkRequest request = setRequestHeaders(url, headers);

    QUrlQuery query;
    for (auto i = queryItems.begin(); i != queryItems.end(); ++i){
        query.addQueryItem(i.key(), i.value());
    }
    if(queryItems.size())
        _manager.post(request, query.toString().toUtf8());
    else
        _manager.post(request, data.toUtf8());
}

void HttpRequest::patch (const QString &url, 
                const QMap<QString, QString> &headers, 
                const QMap<QString, QString>& queryItems, 
                const QString &data,
                bool debug) {
    _debugMode = debug;
    if(_debugMode){
        // LOG
        _queryString = " * PATCH " + url + "\n";
        if(headers.size())      _queryString +="   + Headers   :\n" + qmapToString(headers);
        if(queryItems.size())   _queryString +="   + Items     :\n" + qmapToString(queryItems);
        if(data.size())         _queryString +="   + Data      :\n" + data;
        _queryString += "   + " + QSslSocket::sslLibraryVersionString() + "\n";
    }

    QNetworkRequest request = setRequestHeaders(url, headers);
    QUrlQuery query;
    for (auto i = queryItems.begin(); i != queryItems.end(); ++i){
        query.addQueryItem(i.key(), i.value());
    }
    if(queryItems.size())
        _manager.sendCustomRequest(request,"PATCH",query.toString().toUtf8()); 
    else
        _manager.sendCustomRequest(request,"PATCH",data.toUtf8());
}

void HttpRequest::get(const QString &url, const QMap<QString, QString> &headers, bool debug){
    _debugMode = debug;
    if(_debugMode){
        // LOG
        _queryString = " * GET " + url + "\n";
        if(headers.size())      _queryString +="   + Headers   :\n" + qmapToString(headers);
        _queryString += "   + " + QSslSocket::sslLibraryVersionString() + "\n";
    }

    QNetworkRequest request = setRequestHeaders(url, headers);

    _manager.get(request);
}
