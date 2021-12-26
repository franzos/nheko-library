// SPDX-FileCopyrightText: 2017 Konstantinos Sideris <siderisk@auth.gr>
// SPDX-FileCopyrightText: 2021 Nheko Contributors
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <atomic>
#include <optional>
#include <stack>
#include <variant>

#include <mtx/common.hpp>
#include <mtx/events.hpp>
#include <mtx/events/encrypted.hpp>
#include <mtx/events/member.hpp>
#include <mtx/events/presence.hpp>
#include <mtx/secret_storage.hpp>
#include <mtx/responses/sync.hpp>

#include <QMap>
#include <QPoint>
#include <QTimer>

#include "Authentication.h"
#include "UserSettingsPage.h"
#include "Cache.h"
#include "CacheCryptoStructs.h"
#include "CacheStructs.h"
#include "Utils.h"

class UserSettings;

namespace mtx::requests {
struct CreateRoom;
}
namespace mtx::responses {
struct Notifications;
struct Sync;
struct Timeline;
struct Rooms;
}

using SecretsToDecrypt = std::map<std::string, mtx::secret_storage::AesHmacSha2EncryptedData>;

class Chat : public QObject
{
    Q_OBJECT

public:
    void initialize(std::string userid, std::string homeserver, std::string token);
    static Chat *instance() { 
        if(instance_ == nullptr){
            instance_ = new Chat();
        }
        return instance_; 
    }
    static Authentication *authentication() { return _authentication;};
    QSharedPointer<UserSettings> userSettings() { return userSettings_; }
    void deleteConfigs();
    QString status() const;
    void setStatus(const QString &status);
    mtx::presence::PresenceState currentPresence() const;
    void getProfileInfo(std::string userid = utils::localUser().toStdString());

public slots:
    std::map<QString, RoomInfo> joinedRoomList();
    QHash<QString, RoomInfo> inviteRoomList();
    RoomInfo roomInfo(const std::string &room_id);
    void startChat(QString userid);
    void leaveRoom(const std::string &room_id);
    void createRoom(const mtx::requests::CreateRoom &req);
    void joinRoom(const std::string &room);
    void joinRoomVia(const std::string &room_id,
                     const std::vector<std::string> &via);
    void inviteUser(const std::string &roomid, const std::string &userid, const std::string & reason);
    void kickUser(const std::string & roomid, const std::string & userid, const std::string &reason);
    void banUser(const std::string & roomid, const std::string & userid, const std::string & reason);
    void unbanUser(const std::string & roomid, const std::string &userid, const std::string & reason);
    void receivedSessionKey(const std::string &room_id, const std::string &session_id);
    void decryptDownloadedSecrets(mtx::secret_storage::AesHmacSha2KeyDescription keyDesc,
                                  const SecretsToDecrypt &secrets);
signals:
    void connectionLost();
    void connectionRestored();

    void notificationsRetrieved(const mtx::responses::Notifications &);
    void highlightedNotifsRetrieved(const mtx::responses::Notifications &, const QPoint widgetPos);

    void initiateFinished();
    void unreadMessages(int count);
    void showNotification(const QString &msg);
    void showLoginPage(const QString &msg);

    void userDisplayNameReady(const std::string &name);
    void userAvatarReady(const std::string &avatar);

    void trySyncCb();
    void tryDelayedSyncCb();
    void tryInitialSyncCb();
    void newSyncResponse(const mtx::responses::Sync &res, const std::string &prev_batch_token);
    void leftRoom(const std::string &room_id);
    void roomLeaveFailed(const std::string &error);
    void roomCreated(const std::string &room_id);
    void roomCreationFailed(const std::string &error);
    void joinedRoom(const std::string &room_id);
    void joinRoomFailed(const std::string &error);
    void userInvited(const std::string &room_id, const std::string user_id);
    void userInvitationFailed(const std::string &room_id, const std::string user_id, const std::string &error);

    void roomListUpdated(const mtx::responses::Rooms &rooms);
    void dropToLoginPageCb(const QString &msg);

    void notifyMessage(const QString &roomid,
                       const QString &eventid,
                       const QString &roomname,
                       const QString &sender,
                       const QString &message,
                       const QImage &icon);

    void retrievedPresence(const QString &statusMsg, mtx::presence::PresenceState state);
    void decryptSidebarChanged();
    void chatFocusChanged(const bool focused);

    //! Signals for device verificaiton
    void receivedDeviceVerificationAccept(const mtx::events::msg::KeyVerificationAccept &message);
    void receivedDeviceVerificationRequest(const mtx::events::msg::KeyVerificationRequest &message,
                                           std::string sender);
    void receivedDeviceVerificationCancel(const mtx::events::msg::KeyVerificationCancel &message);
    void receivedDeviceVerificationKey(const mtx::events::msg::KeyVerificationKey &message);
    void receivedDeviceVerificationMac(const mtx::events::msg::KeyVerificationMac &message);
    void receivedDeviceVerificationStart(const mtx::events::msg::KeyVerificationStart &message,
                                         std::string sender);
    void receivedDeviceVerificationReady(const mtx::events::msg::KeyVerificationReady &message);
    void receivedDeviceVerificationDone(const mtx::events::msg::KeyVerificationDone &message);

    void downloadedSecrets(mtx::secret_storage::AesHmacSha2KeyDescription keyDesc,
                           const SecretsToDecrypt &secrets);

private slots:
    void logout();
    void removeRoom(const std::string &room_id);
    void dropToLoginPage(const QString &msg);
    void handleSyncResponse(const mtx::responses::Sync &res, const std::string &prev_batch_token);

private:
    static Chat *instance_;
    static Authentication *_authentication;
    Chat(QSharedPointer<UserSettings> userSettings = UserSettings::initialize(std::nullopt));
    void startInitialSync();
    void tryInitialSync();
    void trySync();
    void verifyOneTimeKeyCountAfterStartup();
    void ensureOneTimeKeyCount(const std::map<std::string, uint16_t> &counts);
    void getBackupVersion();

    using UserID      = QString;
    using Membership  = mtx::events::StateEvent<mtx::events::state::Member>;
    using Memberships = std::map<std::string, Membership>;

    void loadStateFromCache();

    template<class Collection>
    Memberships getMemberships(const std::vector<Collection> &events) const;

    //! Send desktop notification for the received messages.
    void sendNotifications(const mtx::responses::Notifications &);

    QTimer connectivityTimer_;
    std::atomic_bool isConnected_;

    // Global user settings.
    QSharedPointer<UserSettings> userSettings_;
};

template<class Collection>
std::map<std::string, mtx::events::StateEvent<mtx::events::state::Member>>
Chat::getMemberships(const std::vector<Collection> &collection) const
{
    std::map<std::string, mtx::events::StateEvent<mtx::events::state::Member>> memberships;

    using Member = mtx::events::StateEvent<mtx::events::state::Member>;

    for (const auto &event : collection) {
        if (auto member = std::get_if<Member>(event)) {
            memberships.emplace(member->state_key, *member);
        }
    }

    return memberships;
}
