// SPDX-FileCopyrightText: 2017 Konstantinos Sideris <siderisk@auth.gr>
// SPDX-FileCopyrightText: 2021 Nheko Contributors
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QString>
#include <QTextStream>
#include "Cache.h"
#include "MatrixClient.h"
#include "UserSettingsPage.h"
#include "Utils.h"
#include "encryption/Olm.h"

#include "config/nheko.h"

QSharedPointer<UserSettings> UserSettings::instance_;

UserSettings::UserSettings()
{
    connect(
      QCoreApplication::instance(), &QCoreApplication::aboutToQuit, []() { instance_.clear(); });
}

QSharedPointer<UserSettings>
UserSettings::instance()
{
    return instance_;
}

void
UserSettings::initialize(std::optional<QString> profile)
{
    instance_.reset(new UserSettings());
    instance_->load(profile);
}

void
UserSettings::load(std::optional<QString> profile)
{
    auto tempPresence     = settings.value("user/presence", "").toString().toStdString();
    // auto presenceValue    = QMetaEnum::fromType<Presence>().keyToValue(tempPresence.c_str());
    // if (presenceValue < 0)
    //     presenceValue = 0;
    // presence_               = static_cast<Presence>(presenceValue);
    if (profile) // set to "" if it's the default to maintain compatibility
        profile_ = (*profile == "default") ? "" : *profile;
    else
        profile_ = settings.value("user/currentProfile", "").toString();

    QString prefix = (profile_ != "" && profile_ != "default") ? "profile/" + profile_ + "/" : "";
    accessToken_   = settings.value(prefix + "auth/access_token", "").toString();
    homeserver_    = settings.value(prefix + "auth/home_server", "").toString();
    userId_        = settings.value(prefix + "auth/user_id", "").toString();
    deviceId_      = settings.value(prefix + "auth/device_id", "").toString();

    shareKeysWithTrustedUsers_ =
      settings.value(prefix + "user/automatically_share_keys_with_trusted_users", false).toBool();
    onlyShareKeysWithVerifiedUsers_ =
      settings.value(prefix + "user/only_share_keys_with_verified_users", false).toBool();
    useOnlineKeyBackup_ = settings.value(prefix + "user/online_key_backup", false).toBool();

    disableCertificateValidation_ =
      settings.value("disable_certificate_validation", false).toBool();
}

void
UserSettings::setPresence(Presence state)
{
    if (state == presence_)
        return;
    presence_ = state;
    emit presenceChanged(state);
    save();
}

void
UserSettings::setOnlyShareKeysWithVerifiedUsers(bool shareKeys)
{
    if (shareKeys == onlyShareKeysWithVerifiedUsers_)
        return;

    onlyShareKeysWithVerifiedUsers_ = shareKeys;
    emit onlyShareKeysWithVerifiedUsersChanged(shareKeys);
    save();
}

void
UserSettings::setShareKeysWithTrustedUsers(bool shareKeys)
{
    if (shareKeys == shareKeysWithTrustedUsers_)
        return;

    shareKeysWithTrustedUsers_ = shareKeys;
    emit shareKeysWithTrustedUsersChanged(shareKeys);
    save();
}

void
UserSettings::setUseOnlineKeyBackup(bool useBackup)
{
    if (useBackup == useOnlineKeyBackup_)
        return;

    useOnlineKeyBackup_ = useBackup;
    emit useOnlineKeyBackupChanged(useBackup);
    save();
}

void
UserSettings::setProfile(QString profile)
{
    if (profile == profile_)
        return;
    profile_ = profile;
    emit profileChanged(profile_);
    save();
}

void
UserSettings::setUserId(QString userId)
{
    if (userId == userId_)
        return;
    userId_ = userId;
    emit userIdChanged(userId_);
    save();
}

void
UserSettings::setAccessToken(QString accessToken)
{
    if (accessToken == accessToken_)
        return;
    accessToken_ = accessToken;
    emit accessTokenChanged(accessToken_);
    save();
}

void
UserSettings::setDeviceId(QString deviceId)
{
    if (deviceId == deviceId_)
        return;
    deviceId_ = deviceId;
    emit deviceIdChanged(deviceId_);
    save();
}

void
UserSettings::setHomeserver(QString homeserver)
{
    if (homeserver == homeserver_)
        return;
    homeserver_ = homeserver;
    emit homeserverChanged(homeserver_);
    save();
}

void
UserSettings::setDisableCertificateValidation(bool disabled)
{
    if (disabled == disableCertificateValidation_)
        return;
    disableCertificateValidation_ = disabled;
    http::client()->verify_certificates(!disabled);
    emit disableCertificateValidationChanged(disabled);
}

void
UserSettings::save()
{
    settings.beginGroup("user");
    settings.setValue("currentProfile", profile_);
    settings.endGroup(); // user

    QString prefix = (profile_ != "" && profile_ != "default") ? "profile/" + profile_ + "/" : "";
    settings.setValue(prefix + "auth/access_token", accessToken_);
    settings.setValue(prefix + "auth/home_server", homeserver_);
    settings.setValue(prefix + "auth/user_id", userId_);
    settings.setValue(prefix + "auth/device_id", deviceId_);

    settings.setValue(prefix + "user/automatically_share_keys_with_trusted_users",
                      shareKeysWithTrustedUsers_);
    settings.setValue(prefix + "user/only_share_keys_with_verified_users",
                      onlyShareKeysWithVerifiedUsers_);
    settings.setValue(prefix + "user/online_key_backup", useOnlineKeyBackup_);

    settings.setValue("disable_certificate_validation", disableCertificateValidation_);

    settings.sync();
}
