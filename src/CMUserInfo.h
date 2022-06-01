#pragma once

#include <QObject>
#include <QString>
#include "rest/HttpRequest.h"
#include "Authentication.h"


struct CMUserInformation{
    Q_GADGET
    public:
    QString username;
    QString usernameWithoutDomain;
    QString createdAt;
    QString email;
    QString phone;
    QString title;
    QString firstname;
    QString lastname;
    QString localizedTitle;
    QString localizedFirstName;
    QString localizedLastName;

    Q_PROPERTY(QString username                 MEMBER username)
    Q_PROPERTY(QString usernameWithoutDomain    MEMBER usernameWithoutDomain)
    Q_PROPERTY(QString createdAt                MEMBER createdAt)
    Q_PROPERTY(QString email                    MEMBER email)
    Q_PROPERTY(QString phone                    MEMBER phone)
    Q_PROPERTY(QString title                    MEMBER title)
    Q_PROPERTY(QString firstname                MEMBER firstname)
    Q_PROPERTY(QString lastname                 MEMBER lastname)
    Q_PROPERTY(QString localizedTitle           MEMBER localizedTitle)
    Q_PROPERTY(QString localizedFirstName       MEMBER localizedFirstName)
    Q_PROPERTY(QString localizedLastName        MEMBER localizedLastName)
};

class CMUserInfo: public QObject{
    Q_OBJECT
public:
    CMUserInfo();
    void update(const QString &accessToken);
signals:
    void userInfoUpdated(const CMUserInformation &info);
    void userInfoFailure(const QString &message);
private:
    HttpRequest     *_httpRequest;
};
