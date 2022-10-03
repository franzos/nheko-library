// SPDX-FileCopyrightText: 2017 Konstantinos Sideris <siderisk@auth.gr>
// SPDX-FileCopyrightText: 2021 Nheko Contributors
// SPDX-FileCopyrightText: 2022 Nheko Contributors
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "UserSettings.h"

#include <QString>
#include <QCoreApplication>
#include <QFont>
#include <QDebug>
#include <mtx/secret_storage.hpp>
#include "Cache.h"
#include "MatrixClient.h"
#include "UserSettings.h"
#include "Utils.h"
#include "encryption/Olm.h"
#include "Application.h"

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
    tray_        = settings.value(QStringLiteral("user/window/tray"), false).toBool();
    startInTray_ = settings.value(QStringLiteral("user/window/start_in_tray"), false).toBool();

    roomListWidth_ = settings.value(QStringLiteral("user/sidebar/room_list_width"), -1).toInt();
    communityListWidth_ =
      settings.value(QStringLiteral("user/sidebar/community_list_width"), -1).toInt();

    hasDesktopNotifications_ =
      settings.value(QStringLiteral("user/desktop_notifications"), true).toBool();
    hasAlertOnNotification_ =
      settings.value(QStringLiteral("user/alert_on_notification"), false).toBool();
    groupView_         = settings.value(QStringLiteral("user/group_view"), true).toBool();
    buttonsInTimeline_ = settings.value(QStringLiteral("user/timeline/buttons"), true).toBool();
    timelineMaxWidth_  = settings.value(QStringLiteral("user/timeline/max_width"), 0).toInt();
    messageHoverHighlight_ =
      settings.value(QStringLiteral("user/timeline/message_hover_highlight"), false).toBool();
    enlargeEmojiOnlyMessages_ =
      settings.value(QStringLiteral("user/timeline/enlarge_emoji_only_msg"), false).toBool();
    markdown_     = settings.value(QStringLiteral("user/markdown_enabled"), true).toBool();
    bubbles_      = settings.value(QStringLiteral("user/bubbles_enabled"), false).toBool();
    smallAvatars_ = settings.value(QStringLiteral("user/small_avatars_enabled"), false).toBool();
    animateImagesOnHover_ =
      settings.value(QStringLiteral("user/animate_images_on_hover"), false).toBool();
    typingNotifications_ =
      settings.value(QStringLiteral("user/typing_notifications"), true).toBool();
    sortByImportance_ = settings.value(QStringLiteral("user/sort_by_unread"), true).toBool();
    readReceipts_     = settings.value(QStringLiteral("user/read_receipts"), true).toBool();

    font_ = settings.value(QStringLiteral("user/font_family"), "").toString();

    avatarCircles_     = settings.value(QStringLiteral("user/avatar_circles"), true).toBool();
    // useIdenticon_      = settings.value(QStringLiteral("user/use_identicon"), true).toBool();
    openImageExternal_ = settings.value(QStringLiteral("user/open_image_external"), false).toBool();
    openVideoExternal_ = settings.value(QStringLiteral("user/open_video_external"), false).toBool();
    decryptSidebar_    = settings.value(QStringLiteral("user/decrypt_sidebar"), true).toBool();
    spaceNotifications_ = settings.value(QStringLiteral("user/space_notifications"), true).toBool();
    privacyScreen_      = settings.value(QStringLiteral("user/privacy_screen"), false).toBool();
    privacyScreenTimeout_ =
      settings.value(QStringLiteral("user/privacy_screen_timeout"), 0).toInt();

    mobileMode_ = settings.value(QStringLiteral("user/mobile_mode"), false).toBool();
    emojiFont_  = settings.value(QStringLiteral("user/emoji_font_family"), "emoji").toString();
    baseFontSize_ =
      settings.value(QStringLiteral("user/font_size"), QFont().pointSizeF()).toDouble();
    auto tempPresence =
      settings.value(QStringLiteral("user/presence"), "").toString().toStdString();
    // auto presenceValue = QMetaEnum::fromType<Presence>().keyToValue(tempPresence.c_str());
    // if (presenceValue < 0)
    //     presenceValue = 0;
    // presence_   = static_cast<Presence>(presenceValue);
    ringtone_   = settings.value(QStringLiteral("user/ringtone"), "Default").toString();
    microphone_ = settings.value(QStringLiteral("user/microphone"), QString()).toString();
    camera_     = settings.value(QStringLiteral("user/camera"), QString()).toString();
    cameraResolution_ =
      settings.value(QStringLiteral("user/camera_resolution"), QString()).toString();
    cameraFrameRate_ =
      settings.value(QStringLiteral("user/camera_frame_rate"), QString()).toString();
    screenShareFrameRate_ =
      settings.value(QStringLiteral("user/screen_share_frame_rate"), 5).toInt();
    screenSharePiP_ = settings.value(QStringLiteral("user/screen_share_pip"), true).toBool();
    screenShareRemoteVideo_ =
      settings.value(QStringLiteral("user/screen_share_remote_video"), false).toBool();
    screenShareHideCursor_ =
      settings.value(QStringLiteral("user/screen_share_hide_cursor"), false).toBool();
    useStunServer_ = settings.value(QStringLiteral("user/use_stun_server"), false).toBool();

    if (profile) // set to "" if it's the default to maintain compatibility
        profile_ = (*profile == QLatin1String("default")) ? QLatin1String("") : *profile;
    else
        profile_ = settings.value(QStringLiteral("user/currentProfile"), "").toString();

    QString prefix = (profile_ != QLatin1String("") && profile_ != QLatin1String("default"))
                       ? "profile/" + profile_ + "/"
                       : QLatin1String("");
    accessToken_   = settings.value(prefix + "auth/access_token", "").toString();
    homeserver_    = settings.value(prefix + "auth/home_server", "").toString();
    userId_        = settings.value(prefix + "auth/user_id", "").toString();
    cmUserId_      = settings.value(prefix + "auth/cm_user_id", "").toString();
    deviceId_      = settings.value(prefix + "auth/device_id", "").toString();
    hiddenTags_    = settings.value(prefix + "user/hidden_tags", QStringList{}).toStringList();
    mutedTags_  = settings.value(prefix + "user/muted_tags", QStringList{"global"}).toStringList();
    hiddenPins_ = settings.value(prefix + "user/hidden_pins", QStringList{}).toStringList();
    hiddenWidgets_ = settings.value(prefix + "user/hidden_widgets", QStringList{}).toStringList();
    recentReactions_ =
      settings.value(prefix + "user/recent_reactions", QStringList{}).toStringList();

    collapsedSpaces_.clear();
    auto tempSpaces = settings.value(prefix + "user/collapsed_spaces", QList<QVariant>{}).toList();
    for (const auto &e : qAsConst(tempSpaces))
        collapsedSpaces_.push_back(e.toStringList());

    shareKeysWithTrustedUsers_ =
      settings.value(prefix + "user/automatically_share_keys_with_trusted_users", false).toBool();
    onlyShareKeysWithVerifiedUsers_ =
      settings.value(prefix + "user/only_share_keys_with_verified_users", false).toBool();
    useOnlineKeyBackup_ = settings.value(prefix + "user/online_key_backup", false).toBool();

    disableCertificateValidation_ =
      settings.value("disable_certificate_validation", true).toBool();

    settings.beginGroup("secrets");
    QStringList secretKeys = settings.allKeys();
    for(auto const &s: secretKeys)
        secretsMap_[s] = settings.value(s,"").toString();
    settings.endGroup();
}

// bool
// UserSettings::useIdenticon() const
// {
//     return useIdenticon_ && JdenticonProvider::isAvailable();
// }

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
UserSettings::setHiddenTags(const QStringList &hiddenTags)
{
    hiddenTags_ = hiddenTags;
    save();
}

void
UserSettings::setMutedTags(const QStringList &mutedTags)
{
    mutedTags_ = mutedTags;
    save();
}

void
UserSettings::setHiddenPins(const QStringList &hiddenTags)
{
    hiddenPins_ = hiddenTags;
    save();
    emit hiddenPinsChanged();
}

void
UserSettings::setHiddenWidgets(const QStringList &hiddenTags)
{
    hiddenWidgets_ = hiddenTags;
    save();
    emit hiddenWidgetsChanged();
}

void
UserSettings::setRecentReactions(QStringList recent)
{
    recentReactions_ = recent;
    save();
    emit recentReactionsChanged();
}

void
UserSettings::setCollapsedSpaces(QList<QStringList> spaces)
{
    collapsedSpaces_ = spaces;
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
UserSettings::setBubbles(bool state)
{
    if (state == bubbles_)
        return;
    bubbles_ = state;
    emit bubblesChanged(state);
    save();
}

void
UserSettings::setSmallAvatars(bool state)
{
    if (state == smallAvatars_)
        return;
    smallAvatars_ = state;
    emit smallAvatarsChanged(state);
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
UserSettings::setSpaceNotifications(bool state)
{
    if (state == spaceNotifications_)
        return;
    spaceNotifications_ = state;
    emit spaceNotificationsChanged(state);
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
        emojiFont_ = QStringLiteral("emoji");
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

    if (useBackup)
        olm::download_full_keybackup();
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
UserSettings::setCMUserId(QString userId){
    if (userId == cmUserId_)
        return;
    cmUserId_ = userId;
    emit cmUserIdChanged(cmUserId_);
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

// void
// UserSettings::setUseIdenticon(bool state)
// {
//     if (state == useIdenticon_)
//         return;
//     useIdenticon_ = state;
//     emit useIdenticonChanged(useIdenticon_);
//     save();
// }


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
UserSettings::setOpenImageExternal(bool state)
{
    if (state == openImageExternal_)
        return;
    openImageExternal_ = state;
    emit openImageExternalChanged(openImageExternal_);
    save();
}

void
UserSettings::setOpenVideoExternal(bool state)
{
    if (state == openVideoExternal_)
        return;
    openVideoExternal_ = state;
    emit openVideoExternalChanged(openVideoExternal_);
    save();
}

void
UserSettings::save()
{
    settings.beginGroup(QStringLiteral("user"));
    settings.setValue("microphone", microphone_);
    settings.setValue("camera", camera_);
    settings.setValue("camera_resolution", cameraResolution_);
    settings.setValue("camera_frame_rate", cameraFrameRate_);

    settings.beginGroup(QStringLiteral("window"));
    settings.setValue(QStringLiteral("tray"), tray_);
    settings.setValue(QStringLiteral("start_in_tray"), startInTray_);
    settings.endGroup(); // window

    settings.beginGroup(QStringLiteral("sidebar"));
    settings.setValue(QStringLiteral("community_list_width"), communityListWidth_);
    settings.setValue(QStringLiteral("room_list_width"), roomListWidth_);
    settings.endGroup(); // window

    settings.beginGroup(QStringLiteral("timeline"));
    settings.setValue(QStringLiteral("buttons"), buttonsInTimeline_);
    settings.setValue(QStringLiteral("message_hover_highlight"), messageHoverHighlight_);
    settings.setValue(QStringLiteral("enlarge_emoji_only_msg"), enlargeEmojiOnlyMessages_);
    settings.setValue(QStringLiteral("max_width"), timelineMaxWidth_);
    settings.endGroup(); // timeline

    settings.setValue(QStringLiteral("avatar_circles"), avatarCircles_);
    settings.setValue(QStringLiteral("decrypt_sidebar"), decryptSidebar_);
    settings.setValue(QStringLiteral("space_notifications"), spaceNotifications_);
    settings.setValue(QStringLiteral("privacy_screen"), privacyScreen_);
    settings.setValue(QStringLiteral("privacy_screen_timeout"), privacyScreenTimeout_);
    settings.setValue(QStringLiteral("mobile_mode"), mobileMode_);
    settings.setValue(QStringLiteral("font_size"), baseFontSize_);
    settings.setValue(QStringLiteral("typing_notifications"), typingNotifications_);
    settings.setValue(QStringLiteral("sort_by_unread"), sortByImportance_);
    settings.setValue(QStringLiteral("minor_events"), sortByImportance_);
    settings.setValue(QStringLiteral("read_receipts"), readReceipts_);
    settings.setValue(QStringLiteral("group_view"), groupView_);
    settings.setValue(QStringLiteral("markdown_enabled"), markdown_);
    settings.setValue(QStringLiteral("bubbles_enabled"), bubbles_);
    settings.setValue(QStringLiteral("small_avatars_enabled"), smallAvatars_);
    settings.setValue(QStringLiteral("animate_images_on_hover"), animateImagesOnHover_);
    settings.setValue(QStringLiteral("desktop_notifications"), hasDesktopNotifications_);
    settings.setValue(QStringLiteral("alert_on_notification"), hasAlertOnNotification_);
    settings.setValue(QStringLiteral("font_family"), font_);
    settings.setValue(QStringLiteral("emoji_font_family"), emojiFont_);
    // settings.setValue(
    //   QStringLiteral("presence"),
    //   QString::fromUtf8(QMetaEnum::fromType<Presence>().valueToKey(static_cast<int>(presence_))));
    settings.setValue(QStringLiteral("ringtone"), ringtone_);
    settings.setValue(QStringLiteral("microphone"), microphone_);
    settings.setValue(QStringLiteral("camera"), camera_);
    settings.setValue(QStringLiteral("camera_resolution"), cameraResolution_);
    settings.setValue(QStringLiteral("camera_frame_rate"), cameraFrameRate_);
    settings.setValue(QStringLiteral("screen_share_frame_rate"), screenShareFrameRate_);
    settings.setValue(QStringLiteral("screen_share_pip"), screenSharePiP_);
    settings.setValue(QStringLiteral("screen_share_remote_video"), screenShareRemoteVideo_);
    settings.setValue(QStringLiteral("screen_share_hide_cursor"), screenShareHideCursor_);
    settings.setValue(QStringLiteral("use_stun_server"), useStunServer_);
    settings.setValue(QStringLiteral("currentProfile"), profile_);
    // settings.setValue(QStringLiteral("use_identicon"), useIdenticon_);
    settings.setValue(QStringLiteral("open_image_external"), openImageExternal_);
    settings.setValue(QStringLiteral("open_video_external"), openVideoExternal_);

    settings.endGroup(); // user

    QString prefix = (profile_ != QLatin1String("") && profile_ != QLatin1String("default"))
                       ? "profile/" + profile_ + "/"
                       : QLatin1String("");
    settings.setValue(prefix + "auth/access_token", accessToken_);
    settings.setValue(prefix + "auth/home_server", homeserver_);
    settings.setValue(prefix + "auth/user_id", userId_);
    settings.setValue(prefix + "auth/cm_user_id", cmUserId_);
    settings.setValue(prefix + "auth/device_id", deviceId_);

    settings.setValue(prefix + "user/automatically_share_keys_with_trusted_users",
                      shareKeysWithTrustedUsers_);
    settings.setValue(prefix + "user/only_share_keys_with_verified_users",
                      onlyShareKeysWithVerifiedUsers_);
    settings.setValue(prefix + "user/online_key_backup", useOnlineKeyBackup_);
    settings.setValue(prefix + "user/hidden_tags", hiddenTags_);
    settings.setValue(prefix + "user/muted_tags", mutedTags_);
    settings.setValue(prefix + "user/hidden_pins", hiddenPins_);
    settings.setValue(prefix + "user/hidden_widgets", hiddenWidgets_);
    settings.setValue(prefix + "user/recent_reactions", recentReactions_);

    QVariantList v;
    v.reserve(collapsedSpaces_.size());
    for (const auto &e : qAsConst(collapsedSpaces_))
        v.push_back(e);
    settings.setValue(prefix + "user/collapsed_spaces", v);

    settings.setValue(QStringLiteral("disable_certificate_validation"),
                      disableCertificateValidation_);
                      
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