#include "RestRequest.h"
#include <QFileInfo>
#include <QDebug>

std::string RestRequest::qmapToString(const QMap<QString, QString> &map){
    QString result;
    for (auto i = map.begin(); i != map.end(); ++i){
        result += "      - " + i.key() + " : " + i.value() + "\n";
    }
    return result.toStdString();
}
        
QNetworkRequest RestRequest::setRequestHeaders(const QString &urlString, const QMap<QString, QString> &headers) {
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

int RestRequest::post    (const QString &url, 
                const QMap<QString, QString> &headers, 
                const QMap<QString, QString>& queryItems, 
                const QMap<QString, QString> &data,
                QString &response) {
    // LOG
    std::string debugTitle = " * POST " + url.toStdString() + "\n";
    std::string debug = debugTitle;
    if(headers.size())      debug +="   + Headers   :\n" + qmapToString(headers);
    if(queryItems.size())   debug +="   + Items     :\n" + qmapToString(queryItems);
    if(data.size())         debug +="   + Data      :\n" + qmapToString(data);
    debug += "   + " + QSslSocket::sslLibraryVersionString().toStdString();
    qDebug()<<QString::fromStdString(debug);

    QEventLoop loop;
    QNetworkAccessManager manager;
    int httpCode = 0;
    QObject::connect(&manager, &QNetworkAccessManager::finished, [&](QNetworkReply *reply) {
        QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if (statusCode.isValid())
            httpCode = statusCode.toInt();
        response = QString::fromUtf8(reply->readAll());
        QString reason = reply->attribute( QNetworkRequest::HttpReasonPhraseAttribute ).toString();
        std::string dbg = " < " + debugTitle + response.toStdString() + " (" + reason.toStdString() + ":" + std::to_string(httpCode) + ")";
        if(reply->error() != QNetworkReply::NoError) 
            qDebug()<<QString::fromStdString(dbg);
        else
            qDebug()<<QString::fromStdString(dbg);
        loop.quit();
    });

    QNetworkRequest request = setRequestHeaders(url, headers);

    QUrlQuery query;
    for (auto i = queryItems.begin(); i != queryItems.end(); ++i){
        query.addQueryItem(i.key(), i.value());
    }

    QJsonObject obj;
    for (auto i = data.begin(); i != data.end(); ++i)
        obj[i.key()] = i.value();
    QJsonDocument doc(obj);
    QByteArray byteArray = doc.toJson();
    if(queryItems.size())
        manager.post(request, query.toString().toUtf8());
    else 
        manager.post(request, byteArray);
    loop.exec();
    return httpCode;
}

 int RestRequest::post    (const QString &url, 
                     const QMap<QString, QString> &headers, 
                     const QString &data,
                     QString &response){
        // LOG
    std::string debugTitle = " * POST " + url.toStdString() + "\n";
    std::string debug = debugTitle;
    if(headers.size())      debug +="   + Headers   :\n" + qmapToString(headers);
    debug +="   + Data      :\n" + data.toStdString();
    debug += "   + " + QSslSocket::sslLibraryVersionString().toStdString();
    qDebug()<<QString::fromStdString(debug);

     QEventLoop loop;
    QNetworkAccessManager manager;
    int httpCode = 0;
    QObject::connect(&manager, &QNetworkAccessManager::finished, [&](QNetworkReply *reply) {
        QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if (statusCode.isValid())
            httpCode = statusCode.toInt();
        response = QString::fromUtf8(reply->readAll());
        QString reason = reply->attribute( QNetworkRequest::HttpReasonPhraseAttribute ).toString();
        std::string dbg = " < " + debugTitle + response.toStdString() + " (" + reason.toStdString() + ":" + std::to_string(httpCode) + ")";
        if(reply->error() != QNetworkReply::NoError) 
            qDebug()<<QString::fromStdString(dbg);
        else
            qDebug()<<QString::fromStdString(dbg);
        loop.quit();
    });

    QNetworkRequest request = setRequestHeaders(url, headers);

    manager.post(request, data.toUtf8());

    loop.exec();
    return httpCode;
    }


int RestRequest::postJson    (const QString &url, 
                const QMap<QString, QString> &headers, 
                const QJsonDocument jData, 
                QString &response) {
    // LOG
    std::string debugTitle = " * POST " + url.toStdString() + "\n";
    std::string debug = debugTitle;
    if(headers.size())      debug +="   + Headers   :\n" + qmapToString(headers);
    debug +="   + Data     :\n" + jData.toJson().toStdString();
    qDebug()<<QString::fromStdString(debug);

    QEventLoop loop;
    QNetworkAccessManager manager;
    int httpCode = 0;
    QObject::connect(&manager, &QNetworkAccessManager::finished, [&](QNetworkReply *reply) {
        QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if (statusCode.isValid())
            httpCode = statusCode.toInt();
        response = QString::fromUtf8(reply->readAll());
        QString reason = reply->attribute( QNetworkRequest::HttpReasonPhraseAttribute ).toString();
        std::string dbg = " < " + debugTitle + response.toStdString() + " (" + reason.toStdString() + ":" + std::to_string(httpCode) + ")";
        if(reply->error() != QNetworkReply::NoError) 
            qDebug()<<QString::fromStdString(dbg);
        else
            qDebug()<<QString::fromStdString(dbg);
        loop.quit();
    });

    QNetworkRequest request = setRequestHeaders(url, headers);

    
    QByteArray byteArray = jData.toJson();
    
    manager.post(request, byteArray);
    loop.exec();
    return httpCode;
}




int RestRequest::get(const QString &url, const QMap<QString, QString> &headers, QString &response, bool debugFlag){
    // LOG
    std::string debugTitle = " * GET " + url.toStdString() + "\n";
    std::string debug = debugTitle;
    if(headers.size())      debug +="   + Headers   :\n" + qmapToString(headers);
    debug += "   + " + QSslSocket::sslLibraryVersionString().toStdString();
    if(debugFlag) 
        qDebug()<<QString::fromStdString(debug);

    QEventLoop loop;
    QNetworkAccessManager manager;
    int httpCode = 0;
    QObject::connect(&manager, &QNetworkAccessManager::finished, [&](QNetworkReply *reply) {
        QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if (statusCode.isValid())
            httpCode = statusCode.toInt();
        response = QString::fromUtf8(reply->readAll());
        std::string dbg = " < " + debugTitle + response.toStdString() + " (" + std::to_string(httpCode) + ")";
        
        if(reply->error() != QNetworkReply::NoError)
            qDebug()<<QString::fromStdString(dbg);
        else
            if(debugFlag) 
                qDebug()<<QString::fromStdString(dbg);
        loop.quit();
    });

    
    QNetworkRequest request = setRequestHeaders(url, headers);
    

    manager.get(request);
    loop.exec();
    return httpCode;
}

int RestRequest::postMultipart( const QString &url, 
                    const QMap<QString, QString> &headers, 
                    const QMap<QString, QString> &files, 
                    const QMap<QString, QString> &data){
    // LOG
    std::string debugTitle = " * POST multipart " + url.toStdString() + "\n";
    std::string debug  = debugTitle;
    if(headers.size())      debug +="   + Headers   :\n" + qmapToString(headers);
    if(files.size())        debug +="   + form-data :\n" + qmapToString(files);
    if(data.size())         debug +="   + form-data :\n" + qmapToString(data);
    debug += "   + " + QSslSocket::sslLibraryVersionString().toStdString();
    qDebug()<<QString::fromStdString(debug);

    QEventLoop loop;
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    for (auto i = files.begin(); i != files.end(); ++i){
        QFile *file = new QFile(i.value());
        if(!file->exists())
           qDebug()<<QString::fromStdString("The " + i.key().toStdString() + " image file not exist: " + i.value().toStdString());
        else {
            QFileInfo fileInfo(i.value());
            QHttpPart imagePart;
            imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/jpeg"));
            imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, 
                                QVariant(QString("form-data; name=\"" + i.key() + "\"; "
                                "filename=\"%1\"").arg(fileInfo.fileName())));
            file->open(QIODevice::ReadOnly);
            imagePart.setBodyDevice(file);
            file->setParent(multiPart); // we cannot delete the file now, so delete it with the multiPart
            multiPart->append(imagePart);
        }
    }

    for (auto i = data.begin(); i != data.end(); ++i){
        QHttpPart textPart;
        textPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"" + QByteArray::fromStdString(i.key().toStdString()) + "\""));
        textPart.setBody(QByteArray::fromStdString(i.value().toStdString()));
        multiPart->append(textPart);
    }

    QNetworkRequest request = setRequestHeaders(url, headers);
    QNetworkAccessManager manager;
    int httpCode;
    QObject::connect(&manager, &QNetworkAccessManager::finished, [&](QNetworkReply *reply) {
        QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if (statusCode.isValid())
            httpCode = statusCode.toInt();
        std::string dbg = " < " + debugTitle + QString::fromUtf8(reply->readAll()).toStdString() + " (" + std::to_string(httpCode) + ")";
        if(reply->error() != QNetworkReply::NoError) {
            qDebug()<<QString::fromStdString(dbg);
        } else
            qDebug()<<QString::fromStdString(dbg);
        loop.quit();
    });
    QNetworkReply *reply = manager.post(request, multiPart);
    multiPart->setParent(reply); // delete the multiPart with the reply
    loop.exec();
    return httpCode;
}


int RestRequest::patch(const QString &url, const QMap<QString, QString> &headers, QString &response, bool debugFlag){
    // LOG
    std::string debugTitle = " * PATCH " + url.toStdString() + "\n";
    std::string debug = debugTitle;
    if(headers.size())      debug +="   + Headers   :\n" + qmapToString(headers);
    debug += "   + " + QSslSocket::sslLibraryVersionString().toStdString();
    if(debugFlag) qDebug()<<QString::fromStdString(debug);

    QEventLoop loop;
    QNetworkAccessManager manager;
    int httpCode = 0;
    QObject::connect(&manager, &QNetworkAccessManager::finished, [&](QNetworkReply *reply) {
        QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if (statusCode.isValid())
            httpCode = statusCode.toInt();
        response = QString::fromUtf8(reply->readAll());
        std::string dbg = " < " + debugTitle + response.toStdString() + " (" + std::to_string(httpCode) + ")";
        
        if(reply->error() != QNetworkReply::NoError)
            qDebug()<<QString::fromStdString(dbg);
        else
            if(debugFlag) 
                qDebug()<<QString::fromStdString(dbg);
        loop.quit();
    });

    
    QNetworkRequest request = setRequestHeaders(url, headers);
    

    manager.sendCustomRequest(request,"PATCH"); 
    loop.exec();
    return httpCode;
}




int RestRequest::deleteObject(const QString &url, const QMap<QString, QString> &headers, QString &response, bool debugFlag){
    // LOG
    std::string debugTitle = " * DELETE " + url.toStdString() + "\n";
    std::string debug = debugTitle;
    if(headers.size())      debug +="   + Headers   :\n" + qmapToString(headers);
    debug += "   + " + QSslSocket::sslLibraryVersionString().toStdString();
    if(debugFlag) 
        qDebug()<<QString::fromStdString(debug);

    QEventLoop loop;
    QNetworkAccessManager manager;
    int httpCode = 0;
    QObject::connect(&manager, &QNetworkAccessManager::finished, [&](QNetworkReply *reply) {
        QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if (statusCode.isValid())
            httpCode = statusCode.toInt();
        response = QString::fromUtf8(reply->readAll());
        std::string dbg = " < " + debugTitle + response.toStdString() + " (" + std::to_string(httpCode) + ")";
        
        if(reply->error() != QNetworkReply::NoError)
            qDebug()<<QString::fromStdString(dbg);
        else
            if(debugFlag) 
                qDebug()<<QString::fromStdString(dbg);
        loop.quit();
    });

    
    QNetworkRequest request = setRequestHeaders(url, headers);
    

    manager.sendCustomRequest(request,"DELETE"); 
    loop.exec();
    return httpCode;
}



