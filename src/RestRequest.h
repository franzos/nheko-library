#ifndef PX_TIME_TRACKING_RESTREQUEST_H
#define PX_TIME_TRACKING_RESTREQUEST_H

#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QUrlQuery>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QEventLoop>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QFile>
class RestRequest : public QObject{
    Q_OBJECT
    public:
        std::string qmapToString(const QMap<QString, QString> &map);
        
        QNetworkRequest setRequestHeaders(const QString &urlString, const QMap<QString, QString> &headers);

        int post    (const QString &url, 
                     const QMap<QString, QString> &headers, 
                     const QMap<QString, QString>& queryItems, 
                     const QMap<QString, QString> &data,
                     QString &response);
        int post    (const QString &url, 
                    const QMap<QString, QString> &headers, 
                     const QString &data,
                     QString &response);

        int postJson (const QString &url, 
                const QMap<QString, QString> &headers, 
                const QJsonDocument data, 
                QString &response);

        int get(const QString &url, const QMap<QString, QString> &headers, QString &response, bool debugFlag = true);
        int patch(const QString &url, const QMap<QString, QString> &headers, QString &response, bool debugFlag = true);
        int deleteObject(const QString &url, const QMap<QString, QString> &headers, QString &response, bool debugFlag=true);
        int postMultipart( const QString &url, 
                            const QMap<QString, QString> &headers, 
                            const QMap<QString, QString> &files, 
                            const QMap<QString, QString> &data);

};
#endif //PX_TIME_TRACKING_RESTREQUEST_H
