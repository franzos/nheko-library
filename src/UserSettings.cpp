// SPDX-FileCopyrightText: 2017 Konstantinos Sideris <siderisk@auth.gr>
// SPDX-FileCopyrightText: 2021 Nheko Contributors
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QStandardPaths>
#include <QString>
#include <QTextStream>
#include <QCoreApplication>
#include <QDebug>
#include "Cache.h"
#include "MatrixClient.h"
#include "UserSettings.h"
#include "Utils.h"
#include "encryption/Olm.h"


QSharedPointer<UserSettings> UserSettings::instance_;

UserSettings::UserSettings()
{
    
}

QSharedPointer<UserSettings>
UserSettings::instance()
{
    return instance_;
}

QSharedPointer<UserSettings>
UserSettings::initialize(std::optional<QString> profile)
{
    // requirement of UserSettings
    if(QCoreApplication::organizationName().isEmpty())
        QCoreApplication::setOrganizationName("matrix-client-library");
    instance_.reset(new UserSettings());
    instance_->load(profile);
    return instance_;
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
      settings.value("disable_certificate_validation", true).toBool();
    
    ringtone_               = settings.value("user/ringtone", "Default").toString();
    microphone_             = settings.value("user/microphone", QString()).toString();
    camera_                 = settings.value("user/camera", QString()).toString();
    cameraResolution_       = settings.value("user/camera_resolution", QString()).toString();
    cameraFrameRate_        = settings.value("user/camera_frame_rate", QString()).toString();
    useStunServer_          = settings.value("user/use_stun_server", false).toBool();

    settings.beginGroup("secrets");
    QStringList secretKeys = settings.allKeys();
    for(auto const &s: secretKeys)
        secretsMap_[s] = settings.value(s,"").toString();
    settings.endGroup();
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
UserSettings::setRingtone(QString ringtone)
{
    if (ringtone == ringtone_)
        return;
    ringtone_ = ringtone;
    emit ringtoneChanged(ringtone);
    save();
}

void
UserSettings::setMicrophone(QString microphone)
{
    if (microphone == microphone_)
        return;
    microphone_ = microphone;
    emit microphoneChanged(microphone);
    save();
}

void
UserSettings::setCamera(QString camera)
{
    if (camera == camera_)
        return;
    camera_ = camera;
    emit cameraChanged(camera);
    save();
}

void
UserSettings::setCameraResolution(QString resolution)
{
    if (resolution == cameraResolution_)
        return;
    cameraResolution_ = resolution;
    emit cameraResolutionChanged(resolution);
    save();
}

void
UserSettings::setCameraFrameRate(QString frameRate)
{
    if (frameRate == cameraFrameRate_)
        return;
    cameraFrameRate_ = frameRate;
    emit cameraFrameRateChanged(frameRate);
    save();
}

void
UserSettings::setUseStunServer(bool useStunServer)
{
    if (useStunServer == useStunServer_)
        return;
    useStunServer_ = useStunServer;
    emit useStunServerChanged(useStunServer);
    save();
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
    settings.setValue("ringtone", ringtone_);
    settings.setValue("microphone", microphone_);
    settings.setValue("camera", camera_);
    settings.setValue("camera_resolution", cameraResolution_);
    settings.setValue("camera_frame_rate", cameraFrameRate_);
    settings.setValue("use_stun_server", useStunServer_);

    QMap<QString, QString>::iterator i;
    for (i = secretsMap_.begin(); i != secretsMap_.end(); ++i) {
        settings.setValue("secrets/" + i.key(), i.value());
    }
    settings.sync();
}
    
void UserSettings::storeSecret(const QString &key, const QString &value){
    if (secretsMap_[key] == value)
        return;
    secretsMap_[key] = value;
    save();
}

void UserSettings::deleteSecret(const QString &key){
    secretsMap_.remove(key);
    settings.remove("secrets/" + key);
    save();
}