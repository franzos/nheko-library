// Copyright © 2020 Hamzeh Nasajpour <h.nasajpour@pantherx.org> | PantherX.DEV

#pragma once

#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QUrlQuery>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QFile>

class HttpRequest : public QObject{
    Q_OBJECT
public:
    HttpRequest (QObject *parent = nullptr);
    QString qmapToString(const QMap<QString, QString> &map);
    
    QNetworkRequest setRequestHeaders(const QString &urlString, const QMap<QString, QString> &headers);

    void post      (const QString &url, 
                    const QMap<QString, QString> &headers, 
                    const QMap<QString, QString>& queryItems, 
                    const QString &data, bool debug = false);

    void patch      (const QString &url, 
                    const QMap<QString, QString> &headers, 
                    const QMap<QString, QString>& queryItems, 
                    const QString &data, bool debug = false);

    void get(const QString &url, const QMap<QString, QString> &headers, bool debug = false);

private slots:
    void networkReplyHandler(QNetworkReply *reply);

signals:
    void response (int code, const QString &response);

private:
    QNetworkAccessManager _manager;
    QString _queryString;
    bool    _debugMode;
};