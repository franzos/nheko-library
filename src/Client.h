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
#include <mtx/identifiers.hpp>
#include <mtx/events/encrypted.hpp>
#include <mtx/events/member.hpp>
#include <mtx/events/presence.hpp>
#include <mtx/secret_storage.hpp>
#include <mtx/responses/sync.hpp>

#include <QMap>
#include <QPoint>
#include <QTimer>

#include "Authentication.h"
#include "UserSettings.h"
#include "Cache.h"
#include "CacheCryptoStructs.h"
#include "CacheStructs.h"
#include "Utils.h"
#include "timeline/Timeline.h"
#include "encryption/VerificationManager.h"

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
struct UserInformation{
    QString userId;
    QString accessToken;
    QString deviceId;
    QString homeServer;
} ;

using SecretsToDecrypt = std::map<std::string, mtx::secret_storage::AesHmacSha2EncryptedData>;

class Client : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE static Client *instance() { 
        if(instance_ == nullptr){
            http::init();
            instance_ = new Client();
        }
        return instance_; 
    }
    QSharedPointer<UserSettings> userSettings() { return userSettings_; }
    void deleteConfigs();
    QString status() const;
    void setStatus(const QString &status);
    mtx::presence::PresenceState currentPresence() const;
    Timeline * timeline(const QString &roomID){
        auto it = _timelines.find(roomID);
        if(it != _timelines.end()){
            return it.value();
        }
        return nullptr;
    }

    VerificationManager *verificationManager() { return _verificationManager; }
    Q_INVOKABLE void getProfileInfo(QString userid = utils::localUser());
    Q_INVOKABLE void start(QString userId = "", QString homeServer = "", QString token = "");
    Q_INVOKABLE void stop();
    Q_INVOKABLE void enableLogger(bool enable, bool enableDebugLogs = false){
        nhlog::init("matrix-client-library", enable, enableDebugLogs);    
    }

public slots:
    QMap<QString, RoomInfo> joinedRoomList();
    QMap<QString, RoomInfo> inviteRoomList();
    RoomInfo roomInfo(const QString &room_id);
    void startChat(QString userid);
    void leaveRoom(const QString &room_id);
    void createRoom(const mtx::requests::CreateRoom &req);
    void joinRoom(const QString &room);
    void joinRoomVia(const QString &room_id,
                     const std::vector<std::string> &via);
    void inviteUser(const QString &roomid, const QString &userid, const QString & reason);
    void kickUser(const QString & roomid, const QString & userid, const QString &reason);
    void banUser(const QString & roomid, const QString  & userid, const QString & reason);
    void unbanUser(const QString & roomid, const QString &userid, const QString & reason);
    void receivedSessionKey(const QString &room_id, const QString &session_id);
    void decryptDownloadedSecrets(const std::string &recoveryKey, mtx::secret_storage::AesHmacSha2KeyDescription keyDesc,
                                  const SecretsToDecrypt &secrets);
    // Authentication
    void loginWithPassword(QString deviceName, QString userId, QString password, QString serverAddress);
    bool hasValidUser();
    UserInformation userInformation();
    void logout();
    std::string serverDiscovery(QString userId);


signals:
    // Authentication signals
    void loginOk(const UserInformation &user);
    void loginErrorOccurred(const QString &msg);
    void logoutErrorOccurred(const QString &msg);
    void logoutOk();    
    void dropToLogin(const QString &msg);
    //
    void connectionLost();
    void connectionRestored();

    void notificationsRetrieved(const mtx::responses::Notifications &);
    void highlightedNotifsRetrieved(const mtx::responses::Notifications &, const QPoint widgetPos);

    //
    void userDisplayNameReady(const QString &name);
    void userAvatarReady(const QString &avatar);
    // sync signals
    void trySyncCb();
    void tryDelayedSyncCb();
    void tryInitialSyncCb();
    void newSyncResponse(const mtx::responses::Sync &res, const QString &prev_batch_token);
    void initiateFinished();
    void newUpdated(const mtx::responses::Sync &sync);
    void initialSync(const mtx::responses::Sync &sync);
    void prepareTimelines();
    void initializeEmptyViews();
    
    // room signals
    void leftRoom(const QString &room_id);
    void roomLeaveFailed(const QString &error);
    void roomCreated(const QString &room_id);
    void roomCreationFailed(const QString &error);
    void joinedRoom(const QString &room_id);
    void joinRoomFailed(const QString &error);
    void userInvited(const QString &room_id, const QString user_id);
    void userInvitationFailed(const QString &room_id, const QString user_id, const QString &error);
    //
    void retrievedPresence(const QString &statusMsg, mtx::presence::PresenceState state);
    void decryptSidebarChanged();
    //! Signals for device verificaiton
    void receivedDeviceVerificationAccept(const mtx::events::msg::KeyVerificationAccept &message);
    void receivedDeviceVerificationRequest(const mtx::events::msg::KeyVerificationRequest &message,
                                           std::string sender);
    // void receivedRoomDeviceVerificationRequest(
    //   const mtx::events::RoomEvent<mtx::events::msg::KeyVerificationRequest> &message,
    //   TimelineModel *model);
    void receivedDeviceVerificationCancel(const mtx::events::msg::KeyVerificationCancel &message);
    void receivedDeviceVerificationKey(const mtx::events::msg::KeyVerificationKey &message);
    void receivedDeviceVerificationMac(const mtx::events::msg::KeyVerificationMac &message);
    void receivedDeviceVerificationStart(const mtx::events::msg::KeyVerificationStart &message,
                                         std::string sender);
    void receivedDeviceVerificationReady(const mtx::events::msg::KeyVerificationReady &message);
    void receivedDeviceVerificationDone(const mtx::events::msg::KeyVerificationDone &message);

    void downloadedSecrets(const std::string &recoveryKey, mtx::secret_storage::AesHmacSha2KeyDescription keyDesc,
                           const SecretsToDecrypt &secrets);

private slots:
    void logoutCb();
    void loginCb(const mtx::responses::Login &res);
    void removeRoom(const QString &room_id);
    void dropToLoginCb(const QString &msg);
    void handleSyncResponse(const mtx::responses::Sync &res, const QString &prev_batch_token);
    void prepareTimelinesCB();

private:
    static Client *instance_;
    Authentication *_authentication;
    QString _clientName;
    QMap<QString, Timeline *> _timelines;

    Client(QSharedPointer<UserSettings> userSettings = UserSettings::initialize(std::nullopt));
    void startInitialSync();
    void tryInitialSync();
    void trySync();
    void verifyOneTimeKeyCountAfterStartup();
    void ensureOneTimeKeyCount(const std::map<std::string, uint16_t> &counts);
    void getBackupVersion();
    void bootstrap(std::string userid, std::string homeserver, std::string token);
    void syncTimelines(const mtx::responses::Rooms &rooms);
    void syncTimeline(const QString &roomId, const mtx::responses::JoinedRoom &room);
    void createTimelinesFromDB();
    void addTimeline(const QString &roomID); 
    void removeTimeline(const QString &roomID); 

    using UserID      = QString;
    using Membership  = mtx::events::StateEvent<mtx::events::state::Member>;
    using Memberships = std::map<std::string, Membership>;

    void loadStateFromCache();

    template<class Collection>
    Memberships getMemberships(const std::vector<Collection> &events) const;

    //! Send desktop notification for the received messages.
    void sendNotifications(const mtx::responses::Notifications &);

    QTimer connectivityTimer_;
    VerificationManager *_verificationManager = nullptr;
    std::atomic_bool isConnected_;
    // Global user settings.
    QSharedPointer<UserSettings> userSettings_;
};

template<class Collection>
std::map<std::string, mtx::events::StateEvent<mtx::events::state::Member>>
Client::getMemberships(const std::vector<Collection> &collection) const
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
