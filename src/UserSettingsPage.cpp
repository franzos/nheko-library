// SPDX-FileCopyrightText: 2017 Konstantinos Sideris <siderisk@auth.gr>
// SPDX-FileCopyrightText: 2021 Nheko Contributors
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>
// #include <QComboBox>
#include <QCoreApplication>
// #include <QFileDialog>
// #include <QFontComboBox>
// #include <QFormLayout>
// #include <QInputDialog>
// #include <QLabel>
// #include <QLineEdit>
// #include <QMessageBox>
// #include <QPainter>
// #include <QPushButton>
// #include <QResizeEvent>
// #include <QScrollArea>
// #include <QScroller>
// #include <QSpinBox>
#include <QStandardPaths>
#include <QString>
#include <QTextStream>
// #include <QtQml>

#include "Cache.h"
// #include "Config.h"
#include "MatrixClient.h"
#include "UserSettingsPage.h"
#include "Utils.h"
#include "encryption/Olm.h"
// #include "ui/FlatButton.h"
// #include "ui/ToggleButton.h"
// #include "voip/CallDevices.h"

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
    tray_        = settings.value("user/window/tray", false).toBool();
    startInTray_ = settings.value("user/window/start_in_tray", false).toBool();

    roomListWidth_      = settings.value("user/sidebar/room_list_width", -1).toInt();
    communityListWidth_ = settings.value("user/sidebar/community_list_width", -1).toInt();

    hasDesktopNotifications_ = settings.value("user/desktop_notifications", true).toBool();
    hasAlertOnNotification_  = settings.value("user/alert_on_notification", false).toBool();
    groupView_               = settings.value("user/group_view", true).toBool();
    hiddenTags_              = settings.value("user/hidden_tags", QStringList{}).toStringList();
    buttonsInTimeline_       = settings.value("user/timeline/buttons", true).toBool();
    timelineMaxWidth_        = settings.value("user/timeline/max_width", 0).toInt();
    messageHoverHighlight_ =
      settings.value("user/timeline/message_hover_highlight", false).toBool();
    enlargeEmojiOnlyMessages_ =
      settings.value("user/timeline/enlarge_emoji_only_msg", false).toBool();
    markdown_             = settings.value("user/markdown_enabled", true).toBool();
    animateImagesOnHover_ = settings.value("user/animate_images_on_hover", false).toBool();
    typingNotifications_  = settings.value("user/typing_notifications", true).toBool();
    sortByImportance_     = settings.value("user/sort_by_unread", true).toBool();
    readReceipts_         = settings.value("user/read_receipts", true).toBool();
    // theme_                = settings.value("user/theme", defaultTheme_).toString();
    font_                 = settings.value("user/font_family", "default").toString();
    avatarCircles_        = settings.value("user/avatar_circles", true).toBool();
    useIdenticon_         = settings.value("user/use_identicon", true).toBool();
    decryptSidebar_       = settings.value("user/decrypt_sidebar", true).toBool();
    privacyScreen_        = settings.value("user/privacy_screen", false).toBool();
    privacyScreenTimeout_ = settings.value("user/privacy_screen_timeout", 0).toInt();
    mobileMode_           = settings.value("user/mobile_mode", false).toBool();
    emojiFont_            = settings.value("user/emoji_font_family", "default").toString();
    // baseFontSize_         = settings.value("user/font_size", QFont().pointSizeF()).toDouble();
    auto tempPresence     = settings.value("user/presence", "").toString().toStdString();
    // auto presenceValue    = QMetaEnum::fromType<Presence>().keyToValue(tempPresence.c_str());
    // if (presenceValue < 0)
    //     presenceValue = 0;
    // presence_               = static_cast<Presence>(presenceValue);
    ringtone_               = settings.value("user/ringtone", "Default").toString();
    microphone_             = settings.value("user/microphone", QString()).toString();
    camera_                 = settings.value("user/camera", QString()).toString();
    cameraResolution_       = settings.value("user/camera_resolution", QString()).toString();
    cameraFrameRate_        = settings.value("user/camera_frame_rate", QString()).toString();
    screenShareFrameRate_   = settings.value("user/screen_share_frame_rate", 5).toInt();
    screenSharePiP_         = settings.value("user/screen_share_pip", true).toBool();
    screenShareRemoteVideo_ = settings.value("user/screen_share_remote_video", false).toBool();
    screenShareHideCursor_  = settings.value("user/screen_share_hide_cursor", false).toBool();
    useStunServer_          = settings.value("user/use_stun_server", false).toBool();

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

    // applyTheme();
}
void
UserSettings::setMessageHoverHighlight(bool state)
{
    if (state == messageHoverHighlight_)
        return;
    messageHoverHighlight_ = state;
    emit messageHoverHighlightChanged(state);
    save();
}
void
UserSettings::setEnlargeEmojiOnlyMessages(bool state)
{
    if (state == enlargeEmojiOnlyMessages_)
        return;
    enlargeEmojiOnlyMessages_ = state;
    emit enlargeEmojiOnlyMessagesChanged(state);
    save();
}
void
UserSettings::setTray(bool state)
{
    if (state == tray_)
        return;
    tray_ = state;
    emit trayChanged(state);
    save();
}

void
UserSettings::setStartInTray(bool state)
{
    if (state == startInTray_)
        return;
    startInTray_ = state;
    emit startInTrayChanged(state);
    save();
}

void
UserSettings::setMobileMode(bool state)
{
    if (state == mobileMode_)
        return;
    mobileMode_ = state;
    emit mobileModeChanged(state);
    save();
}

void
UserSettings::setGroupView(bool state)
{
    if (groupView_ == state)
        return;

    groupView_ = state;
    emit groupViewStateChanged(state);
    save();
}

void
UserSettings::setHiddenTags(QStringList hiddenTags)
{
    hiddenTags_ = hiddenTags;
    save();
}

void
UserSettings::setMarkdown(bool state)
{
    if (state == markdown_)
        return;
    markdown_ = state;
    emit markdownChanged(state);
    save();
}

void
UserSettings::setAnimateImagesOnHover(bool state)
{
    if (state == animateImagesOnHover_)
        return;
    animateImagesOnHover_ = state;
    emit animateImagesOnHoverChanged(state);
    save();
}

void
UserSettings::setReadReceipts(bool state)
{
    if (state == readReceipts_)
        return;
    readReceipts_ = state;
    emit readReceiptsChanged(state);
    save();
}

void
UserSettings::setTypingNotifications(bool state)
{
    if (state == typingNotifications_)
        return;
    typingNotifications_ = state;
    emit typingNotificationsChanged(state);
    save();
}

void
UserSettings::setSortByImportance(bool state)
{
    if (state == sortByImportance_)
        return;
    sortByImportance_ = state;
    emit roomSortingChanged(state);
    save();
}

void
UserSettings::setButtonsInTimeline(bool state)
{
    if (state == buttonsInTimeline_)
        return;
    buttonsInTimeline_ = state;
    emit buttonInTimelineChanged(state);
    save();
}

void
UserSettings::setTimelineMaxWidth(int state)
{
    if (state == timelineMaxWidth_)
        return;
    timelineMaxWidth_ = state;
    emit timelineMaxWidthChanged(state);
    save();
}
void
UserSettings::setCommunityListWidth(int state)
{
    if (state == communityListWidth_)
        return;
    communityListWidth_ = state;
    emit communityListWidthChanged(state);
    save();
}
void
UserSettings::setRoomListWidth(int state)
{
    if (state == roomListWidth_)
        return;
    roomListWidth_ = state;
    emit roomListWidthChanged(state);
    save();
}

void
UserSettings::setDesktopNotifications(bool state)
{
    if (state == hasDesktopNotifications_)
        return;
    hasDesktopNotifications_ = state;
    emit desktopNotificationsChanged(state);
    save();
}

void
UserSettings::setAlertOnNotification(bool state)
{
    if (state == hasAlertOnNotification_)
        return;
    hasAlertOnNotification_ = state;
    emit alertOnNotificationChanged(state);
    save();
}

void
UserSettings::setAvatarCircles(bool state)
{
    if (state == avatarCircles_)
        return;
    avatarCircles_ = state;
    emit avatarCirclesChanged(state);
    save();
}

void
UserSettings::setDecryptSidebar(bool state)
{
    if (state == decryptSidebar_)
        return;
    decryptSidebar_ = state;
    emit decryptSidebarChanged(state);
    save();
}

void
UserSettings::setPrivacyScreen(bool state)
{
    if (state == privacyScreen_) {
        return;
    }
    privacyScreen_ = state;
    emit privacyScreenChanged(state);
    save();
}

void
UserSettings::setPrivacyScreenTimeout(int state)
{
    if (state == privacyScreenTimeout_) {
        return;
    }
    privacyScreenTimeout_ = state;
    emit privacyScreenTimeoutChanged(state);
    save();
}

void
UserSettings::setFontSize(double size)
{
    if (size == baseFontSize_)
        return;
    baseFontSize_ = size;
    emit fontSizeChanged(size);
    save();
}

void
UserSettings::setFontFamily(QString family)
{
    if (family == font_)
        return;
    font_ = family;
    emit fontChanged(family);
    save();
}

void
UserSettings::setEmojiFontFamily(QString family)
{
    if (family == emojiFont_)
        return;

    if (family == tr("Default")) {
        emojiFont_ = "default";
    } else {
        emojiFont_ = family;
    }

    emit emojiFontChanged(family);
    save();
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

// void
// UserSettings::setTheme(QString theme)
// {
//     if (theme == theme_)
//         return;
//     theme_ = theme;
//     save();
//     applyTheme();
//     emit themeChanged(theme);
// }

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
UserSettings::setScreenShareFrameRate(int frameRate)
{
    if (frameRate == screenShareFrameRate_)
        return;
    screenShareFrameRate_ = frameRate;
    emit screenShareFrameRateChanged(frameRate);
    save();
}

void
UserSettings::setScreenSharePiP(bool state)
{
    if (state == screenSharePiP_)
        return;
    screenSharePiP_ = state;
    emit screenSharePiPChanged(state);
    save();
}

void
UserSettings::setScreenShareRemoteVideo(bool state)
{
    if (state == screenShareRemoteVideo_)
        return;
    screenShareRemoteVideo_ = state;
    emit screenShareRemoteVideoChanged(state);
    save();
}

void
UserSettings::setScreenShareHideCursor(bool state)
{
    if (state == screenShareHideCursor_)
        return;
    screenShareHideCursor_ = state;
    emit screenShareHideCursorChanged(state);
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
UserSettings::setUseIdenticon(bool state)
{
    if (state == useIdenticon_)
        return;
    useIdenticon_ = state;
    emit useIdenticonChanged(useIdenticon_);
    save();
}

// void
// UserSettings::applyTheme()
// {
//     QFile stylefile;

//     if (this->theme() == "light") {
//         stylefile.setFileName(":/styles/styles/nheko.qss");
//     } else if (this->theme() == "dark") {
//         stylefile.setFileName(":/styles/styles/nheko-dark.qss");
//     } else {
//         stylefile.setFileName(":/styles/styles/system.qss");
//     }
//     QApplication::setPalette(Theme::paletteFromTheme(this->theme().toStdString()));

//     stylefile.open(QFile::ReadOnly);
//     QString stylesheet = QString(stylefile.readAll());

//     qobject_cast<QApplication *>(QApplication::instance())->setStyleSheet(stylesheet);
// }

void
UserSettings::save()
{
    settings.beginGroup("user");

    settings.beginGroup("window");
    settings.setValue("tray", tray_);
    settings.setValue("start_in_tray", startInTray_);
    settings.endGroup(); // window

    settings.beginGroup("sidebar");
    settings.setValue("community_list_width", communityListWidth_);
    settings.setValue("room_list_width", roomListWidth_);
    settings.endGroup(); // window

    settings.beginGroup("timeline");
    settings.setValue("buttons", buttonsInTimeline_);
    settings.setValue("message_hover_highlight", messageHoverHighlight_);
    settings.setValue("enlarge_emoji_only_msg", enlargeEmojiOnlyMessages_);
    settings.setValue("max_width", timelineMaxWidth_);
    settings.endGroup(); // timeline

    settings.setValue("avatar_circles", avatarCircles_);
    settings.setValue("decrypt_sidebar", decryptSidebar_);
    settings.setValue("privacy_screen", privacyScreen_);
    settings.setValue("privacy_screen_timeout", privacyScreenTimeout_);
    settings.setValue("mobile_mode", mobileMode_);
    settings.setValue("font_size", baseFontSize_);
    settings.setValue("typing_notifications", typingNotifications_);
    settings.setValue("sort_by_unread", sortByImportance_);
    settings.setValue("minor_events", sortByImportance_);
    settings.setValue("read_receipts", readReceipts_);
    settings.setValue("group_view", groupView_);
    settings.setValue("hidden_tags", hiddenTags_);
    settings.setValue("markdown_enabled", markdown_);
    settings.setValue("animate_images_on_hover", animateImagesOnHover_);
    settings.setValue("desktop_notifications", hasDesktopNotifications_);
    settings.setValue("alert_on_notification", hasAlertOnNotification_);
    // settings.setValue("theme", theme());
    settings.setValue("font_family", font_);
    settings.setValue("emoji_font_family", emojiFont_);
    // settings.setValue(
    //   "presence",
    //   QString::fromUtf8(QMetaEnum::fromType<Presence>().valueToKey(static_cast<int>(presence_))));
    settings.setValue("ringtone", ringtone_);
    settings.setValue("microphone", microphone_);
    settings.setValue("camera", camera_);
    settings.setValue("camera_resolution", cameraResolution_);
    settings.setValue("camera_frame_rate", cameraFrameRate_);
    settings.setValue("screen_share_frame_rate", screenShareFrameRate_);
    settings.setValue("screen_share_pip", screenSharePiP_);
    settings.setValue("screen_share_remote_video", screenShareRemoteVideo_);
    settings.setValue("screen_share_hide_cursor", screenShareHideCursor_);
    settings.setValue("use_stun_server", useStunServer_);
    settings.setValue("currentProfile", profile_);
    settings.setValue("use_identicon", useIdenticon_);

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
