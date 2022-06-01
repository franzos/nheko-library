#pragma once

#include <QObject>
#include <QString>

struct UserInformation{
    Q_GADGET
    public:
    QString userId;
    QString cmUserId;
    QString accessToken;
    QString deviceId;
    QString homeServer;
    QString displayName;
    QString avatarUrl;

    Q_PROPERTY(QString userId MEMBER userId)
    Q_PROPERTY(QString cmUserId MEMBER cmUserId)
    Q_PROPERTY(QString accessToken MEMBER accessToken)
    Q_PROPERTY(QString deviceId MEMBER deviceId)
    Q_PROPERTY(QString homeServer MEMBER homeServer)
    Q_PROPERTY(QString displayName MEMBER displayName)
    Q_PROPERTY(QString avatarUrl MEMBER avatarUrl)
};
