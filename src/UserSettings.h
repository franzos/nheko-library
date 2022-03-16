// SPDX-FileCopyrightText: 2017 Konstantinos Sideris <siderisk@auth.gr>
// SPDX-FileCopyrightText: 2021 Nheko Contributors
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QProcessEnvironment>
#include <QSettings>
#include <QSharedPointer>

#include <optional>

class Toggle;

class UserSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Presence presence READ presence WRITE setPresence NOTIFY presenceChanged)
    Q_PROPERTY(bool onlyShareKeysWithVerifiedUsers READ onlyShareKeysWithVerifiedUsers WRITE
                 setOnlyShareKeysWithVerifiedUsers NOTIFY onlyShareKeysWithVerifiedUsersChanged)
    Q_PROPERTY(bool shareKeysWithTrustedUsers READ shareKeysWithTrustedUsers WRITE
                 setShareKeysWithTrustedUsers NOTIFY shareKeysWithTrustedUsersChanged)
    Q_PROPERTY(bool useOnlineKeyBackup READ useOnlineKeyBackup WRITE setUseOnlineKeyBackup NOTIFY
                 useOnlineKeyBackupChanged)
    Q_PROPERTY(QString profile READ profile WRITE setProfile NOTIFY profileChanged)
    Q_PROPERTY(QString userId READ userId WRITE setUserId NOTIFY userIdChanged)
    Q_PROPERTY(QString accessToken READ accessToken WRITE setAccessToken NOTIFY accessTokenChanged)
    Q_PROPERTY(QString deviceId READ deviceId WRITE setDeviceId NOTIFY deviceIdChanged)
    Q_PROPERTY(QString homeserver READ homeserver WRITE setHomeserver NOTIFY homeserverChanged)
    Q_PROPERTY(bool disableCertificateValidation READ disableCertificateValidation WRITE
                 setDisableCertificateValidation NOTIFY disableCertificateValidationChanged)
    Q_PROPERTY(bool useStunServer READ useStunServer WRITE setUseStunServer NOTIFY useStunServerChanged)
    UserSettings();

public:
    static QSharedPointer<UserSettings> instance();
    static QSharedPointer<UserSettings> initialize(std::optional<QString> profile);

    QSettings *qsettings() { return &settings; }

    enum class Presence
    {
        AutomaticPresence,
        Online,
        Unavailable,
        Offline,
    };
    Q_ENUM(Presence)

    void save();
    void load(std::optional<QString> profile);
    void setPresence(Presence state);
    void setOnlyShareKeysWithVerifiedUsers(bool state);
    void setShareKeysWithTrustedUsers(bool state);
    void setUseOnlineKeyBackup(bool state);
    void setProfile(QString profile);
    void setUserId(QString userId);
    void setAccessToken(QString accessToken);
    void setDeviceId(QString deviceId);
    void setHomeserver(QString homeserver);
    void setDisableCertificateValidation(bool disabled);
    void setUseStunServer(bool state);

    Presence presence() const { return presence_; }
    bool shareKeysWithTrustedUsers() const { return shareKeysWithTrustedUsers_; }
    bool onlyShareKeysWithVerifiedUsers() const { return onlyShareKeysWithVerifiedUsers_; }
    bool useOnlineKeyBackup() const { return useOnlineKeyBackup_; }
    QString profile() const { return profile_; }
    QString userId() const { return userId_; }
    QString accessToken() const { return accessToken_; }
    QString deviceId() const { return deviceId_; }
    QString homeserver() const { return homeserver_; }
    bool disableCertificateValidation() const { return disableCertificateValidation_; }
    bool useStunServer() const { return useStunServer_; }

    QString secret(const QString &key){
        return secretsMap_[key];
    }
    void storeSecret(const QString &key, const QString &value);
    void deleteSecret(const QString &key);
    void clear(){
        settings.clear();
    }

signals:
    void presenceChanged(Presence state);
    void onlyShareKeysWithVerifiedUsersChanged(bool state);
    void shareKeysWithTrustedUsersChanged(bool state);
    void useOnlineKeyBackupChanged(bool state);
    void profileChanged(QString profile);
    void userIdChanged(QString userId);
    void accessTokenChanged(QString accessToken);
    void deviceIdChanged(QString deviceId);
    void homeserverChanged(QString homeserver);
    void disableCertificateValidationChanged(bool disabled);
    void useStunServerChanged(bool state);

private:
    bool shareKeysWithTrustedUsers_;
    bool onlyShareKeysWithVerifiedUsers_;
    bool useOnlineKeyBackup_;
    Presence presence_;
    bool disableCertificateValidation_ = true;
    QString profile_;
    QString userId_;
    QString accessToken_;
    QString deviceId_;
    QString homeserver_;
    bool useStunServer_;
    QSettings settings;
    QMap<QString, QString> secretsMap_;
    static QSharedPointer<UserSettings> instance_;
};
