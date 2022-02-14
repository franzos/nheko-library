// SPDX-FileCopyrightText: 2017 Konstantinos Sideris <siderisk@auth.gr>
// SPDX-FileCopyrightText: 2021 Nheko Contributors
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QUrl>
#include <mtx/responses.hpp>
#include <QDebug>
#include <QThread>
#include "Cache.h"
#include "Cache_p.h"
#include "Client.h"
#include "EventAccessors.h"
#include "Logging.h"
#include "MatrixClient.h"
#include "UserProfile.h"
#include "UserSettings.h"
#include "encryption/Olm.h"

Client *Client::instance_  = nullptr;
constexpr int CHECK_CONNECTIVITY_INTERVAL = 15'000;
constexpr int RETRY_TIMEOUT               = 5'000;
constexpr size_t MAX_ONETIME_KEYS         = 50;

Q_DECLARE_METATYPE(std::optional<mtx::crypto::EncryptedFile>)
Q_DECLARE_METATYPE(std::optional<RelatedInfo>)
Q_DECLARE_METATYPE(mtx::presence::PresenceState)
Q_DECLARE_METATYPE(mtx::secret_storage::AesHmacSha2KeyDescription)
Q_DECLARE_METATYPE(SecretsToDecrypt)

Client::Client(QSharedPointer<UserSettings> userSettings)
  :isConnected_(true),
   userSettings_{userSettings}
{
    instance_->enableLogger(true);
    setObjectName("matrix_client");
    qRegisterMetaType<std::optional<mtx::crypto::EncryptedFile>>();
    qRegisterMetaType<std::optional<RelatedInfo>>();
    qRegisterMetaType<mtx::presence::PresenceState>();
    qRegisterMetaType<mtx::secret_storage::AesHmacSha2KeyDescription>();
    qRegisterMetaType<SecretsToDecrypt>();
    _verificationManager = new VerificationManager(this);
    _authentication = new Authentication();
    connect(_authentication,
            &Authentication::logoutOk,
            this,
            &Client::logoutCb,
            Qt::QueuedConnection);
    connect(_authentication,
            &Authentication::loginOk,
            this,
            &Client::loginCb,
            Qt::QueuedConnection);
    connect(_authentication, &Authentication::loginErrorOccurred, [&](std::string &msg) {
        nhlog::net()->info("login failed: {}", msg);
        QString err =QString::fromStdString(msg);
        emit loginErrorOccurred(err);
    });
    connect(_authentication, &Authentication::logoutErrorOccurred, [&](std::string &msg) {
        nhlog::net()->info("logout failed: {}" ,msg);
        QString err =QString::fromStdString(msg);
        emit logoutErrorOccurred(err);
    });

    connect(this,
            &Client::downloadedSecrets,
            this,
            &Client::decryptDownloadedSecrets,
            Qt::QueuedConnection);

    connect(this, &Client::connectionLost, this, [this]() {
        nhlog::net()->info("connectivity lost");
        isConnected_ = false;
        http::client()->shutdown();
    });
    connect(this, &Client::connectionRestored, this, [this]() {
        nhlog::net()->info("trying to re-connect");
        isConnected_ = true;

        // Drop all pending connections.
        http::client()->shutdown();
        trySync();
    });

    connectivityTimer_.setInterval(CHECK_CONNECTIVITY_INTERVAL);
    connect(&connectivityTimer_, &QTimer::timeout, this, [=]() {
        if (http::client()->access_token().empty()) {
            connectivityTimer_.stop();
            return;
        }

        http::client()->versions(
          [this](const mtx::responses::Versions &, mtx::http::RequestErr err) {
              if (err) {
                  emit connectionLost();
                  return;
              }

              if (!isConnected_)
                  emit connectionRestored();
          });
    });

    // connect(
    //   view_manager_,
    //   &TimelineViewManager::inviteUsers,
    //   this,
    //   [this](QString roomId, QStringList users) {
    //       for (int ii = 0; ii < users.size(); ++ii) {
    //           QTimer::singleShot(ii * 500, this, [this, roomId, ii, users]() {
    //               const auto user = users.at(ii);

    //               http::client()->invite_user(
    //                 roomId.toStdString(),
    //                 user.toStdString(),
    //                 [this, user](const mtx::responses::RoomInvite &, mtx::http::RequestErr err) {
    //                     if (err) {
    //                         emit showNotification(tr("Failed to invite user: %1").arg(user));
    //                         return;
    //                     }

    //                     emit showNotification(tr("Invited user: %1").arg(user));
    //                 });
    //           });
    //       }
    //   });

    connect(this, &Client::leftRoom, this, &Client::removeRoom);
    connect(this, &Client::prepareTimelines, this, &Client::prepareTimelinesCB, Qt::QueuedConnection);
    connect(this, &Client::notificationsRetrieved, this, &Client::sendNotifications);
    connect(this,
            &Client::highlightedNotifsRetrieved,
            this,
            [](const mtx::responses::Notifications &notif) {
                try {
                    cache::saveTimelineMentions(notif);
                } catch (const lmdb::error &e) {
                    nhlog::db()->error("failed to save mentions: {}", e.what());
                }
            });

    connect(
      this, &Client::tryInitialSyncCb, this, &Client::tryInitialSync, Qt::QueuedConnection);
    connect(this, &Client::trySyncCb, this, &Client::trySync, Qt::QueuedConnection);
    connect(
      this,
      &Client::tryDelayedSyncCb,
      this,
      [this]() { QTimer::singleShot(RETRY_TIMEOUT, this, &Client::trySync); },
      Qt::QueuedConnection);

    connect(
      this, &Client::newSyncResponse, this, &Client::handleSyncResponse, Qt::QueuedConnection);

    connect(this, &Client::dropToLogin, this, &Client::dropToLoginCb);

    // verification handlers
    connect(this,
            &Client::receivedDeviceVerificationRequest,
            _verificationManager,
            &VerificationManager::receivedDeviceVerificationRequest);
    connect(this,
            &Client::receivedDeviceVerificationStart,
            _verificationManager,
            &VerificationManager::receivedDeviceVerificationStart);
}

void
Client::logoutCb()
{
    nhlog::net()->info("Logged out");
    deleteConfigs();
    connectivityTimer_.stop();
    emit logoutOk();
}
void
Client::loginCb(const mtx::responses::Login &res)
{
    auto userid     = QString::fromStdString(res.user_id.to_string());
    auto device_id  = QString::fromStdString(res.device_id);
    auto homeserver = QString::fromStdString(http::client()->server() + ":" +
                                            std::to_string(http::client()->port()));
    auto token      = QString::fromStdString(res.access_token);

    userSettings_.data()->setUserId(userid);
    userSettings_.data()->setAccessToken(token);
    userSettings_.data()->setDeviceId(device_id);
    userSettings_.data()->setHomeserver(homeserver);
    UserInformation user;
    user.userId = userid;
    user.homeServer = homeserver;
    user.deviceId = device_id;
    user.accessToken = token;
    emit loginOk(user);
}

void
Client::dropToLoginCb(const QString &msg)
{
    nhlog::ui()->info("dropping to the login page: {}", msg.toStdString());
    connectivityTimer_.stop();
    deleteConfigs();
}

void
Client::deleteConfigs()
{
    UserSettings::instance()->clear();
    http::client()->shutdown();
    if(cache::client())
        cache::deleteData();
}

void
Client::bootstrap(std::string userid, std::string homeserver, std::string token)
{
    using namespace mtx::identifiers;
    try {
        http::client()->set_user(parse<User>(userid));
    } catch (const std::invalid_argument &) {
        nhlog::ui()->critical("Initialized with invalid user_id: {}", userid);
    }

    http::client()->set_server(homeserver);
    http::client()->set_access_token(token);
    http::client()->set_device_id(UserSettings::instance()->deviceId().toStdString());
    http::client()->verify_certificates(!UserSettings::instance()->disableCertificateValidation());

    // The Olm client needs the user_id & device_id that will be included
    // in the generated payloads & keys.
    olm::client()->set_user_id(http::client()->user_id().to_string());
    olm::client()->set_device_id(http::client()->device_id());

    try {
        cache::init(QString::fromStdString(userid));
        auto p = cache::client();
        while(!p->isDatabaseReady()){
            QThread::sleep(2);
            nhlog::db()->debug("Database is not ready. waiting ...");
        }
        nhlog::db()->info("database ready");

        const bool isInitialized = cache::isInitialized();
        const auto cacheVersion  = cache::formatVersion();
        try {
            if (!isInitialized) {
                cache::setCurrentFormat();
            } else {
                if (cacheVersion == cache::CacheVersion::Current) {
                    loadStateFromCache();
                    return;
                } else if (cacheVersion == cache::CacheVersion::Newer) {
                    // TODO
                    // QMessageBox::critical(
                    //   this,
                    //   tr("Incompatible cache version"),
                    //   tr("The cache on your disk is newer than this version of Nheko "
                    //      "supports. Please update Nheko or clear your cache."));
                    // QCoreApplication::quit();
                    return;
                }
            }

            // It's the first time syncing with this device
            // There isn't a saved olm account to restore.
            nhlog::crypto()->info("creating new olm account");
            olm::client()->create_new_account();
            cache::saveOlmAccount(olm::client()->save(cache::client()->pickleSecret()));
        } catch (const lmdb::error &e) {
            nhlog::crypto()->critical("failed to save olm account {}", e.what());
            emit dropToLogin(e.what());
            return;
        } catch (const mtx::crypto::olm_exception &e) {
            nhlog::crypto()->critical("failed to create new olm account {}", e.what());
            emit dropToLogin(e.what());
            return;
        }
        getProfileInfo();
        getBackupVersion();
        tryInitialSync();
        // connect(cache::client(),
        //         &Cache::newReadReceipts,
        //         view_manager_,
        //         &TimelineViewManager::updateReadReceipts);

        // connect(cache::client(),
        //         &Cache::removeNotification,
        //         &notificationsManager,
        //         &NotificationsManager::removeNotification);

    } catch (const lmdb::error &e) {
        nhlog::db()->critical("failure during boot: {}", e.what());
        emit dropToLogin("Failed to open database, logging out!");
    }
}

QMap<QString, RoomInfo> Client::inviteRoomList(){
    return cache::invites();
}    

QMap<QString, RoomInfo> Client::joinedRoomList(){
    return cache::getRoomInfo(cache::joinedRooms());
}    

RoomInfo Client::roomInfo(const QString &room_id){
    return cache::singleRoomInfo(room_id.toStdString());
}

void
Client::loadStateFromCache()
{
    nhlog::db()->info("restoring state from cache");

    try {
        olm::client()->load(cache::restoreOlmAccount(), cache::client()->pickleSecret());
        emit initializeEmptyViews();
        cache::calculateRoomReadStatus();
    } catch (const mtx::crypto::olm_exception &e) {
        nhlog::crypto()->critical("failed to restore olm account: {}", e.what());
        emit dropToLogin("Failed to restore OLM account. Please login again.");
        return;
    } catch (const lmdb::error &e) {
        nhlog::db()->critical("failed to restore cache: {}", e.what());
        emit dropToLogin("Failed to restore save data. Please login again.");
        return;
    } catch (const json::exception &e) {
        nhlog::db()->critical("failed to parse cache data: {}", e.what());
        emit dropToLogin("Failed to restore save data. Please login again.");
        return;
    } catch (const std::exception &e) {
        nhlog::db()->critical("failed to load cache data: {}", e.what());
        emit dropToLogin("Failed to restore save data. Please login again.");
        return;
    }

    nhlog::crypto()->info("ed25519   : {}", olm::client()->identity_keys().ed25519);
    nhlog::crypto()->info("curve25519: {}", olm::client()->identity_keys().curve25519);

    getProfileInfo();
    getBackupVersion();
    verifyOneTimeKeyCountAfterStartup();
    
    emit trySyncCb();
    emit prepareTimelines();
    auto up = new UserProfile("",utils::localUser());
}

void Client::prepareTimelinesCB(){
    createTimelinesFromDB();
    emit initiateFinished();
}

void
Client::removeRoom(const QString &room_id)
{
    try {
        cache::removeRoom(room_id);
        cache::removeInvite(room_id.toStdString());
    } catch (const lmdb::error &e) {
        nhlog::db()->critical("failure while removing room: {}", e.what());
        // TODO: Notify the user.
    }
}

void
Client::sendNotifications(const mtx::responses::Notifications &res)
{
    for (const auto &item : res.notifications) {
        const auto event_id = mtx::accessors::event_id(item.event);

        try {
            if (item.read) {
                cache::removeReadNotification(event_id);
                continue;
            }

            if (!cache::isNotificationSent(event_id)) {
                const auto room_id = QString::fromStdString(item.room_id);

                // We should only sent one notification per event.
                cache::markSentNotification(event_id);

                // Don't send a notification when the current room is opened.
                // if (isRoomActive(room_id))
                //     continue;

                // if (userSettings_->hasDesktopNotifications()) {
                    // auto info = cache::singleRoomInfo(item.room_id);

                    // AvatarProvider::resolve(QString::fromStdString(info.avatar_url),
                    //                         96,
                    //                         this,
                    //                         [this, item](QPixmap image) {
                    //                             notificationsManager.postNotification(
                    //                               item, image.toImage());
                    //                         });
                // }
            }
        } catch (const lmdb::error &e) {
            nhlog::db()->warn("error while sending notification: {}", e.what());
        }
    }
}

void
Client::tryInitialSync()
{
    nhlog::crypto()->info("ed25519   : {}", olm::client()->identity_keys().ed25519);
    nhlog::crypto()->info("curve25519: {}", olm::client()->identity_keys().curve25519);

    // Upload one time keys for the device.
    nhlog::crypto()->info("generating one time keys");
    olm::client()->generate_one_time_keys(MAX_ONETIME_KEYS);
    http::client()->upload_keys(
      olm::client()->create_upload_keys_request(),
      [this](const mtx::responses::UploadKeys &res, mtx::http::RequestErr err) {
          if (err) {
              const int status_code = static_cast<int>(err->status_code);
              if (status_code == 404) {
                  nhlog::net()->warn("skipping key uploading. server doesn't provide /keys/upload");
                  return startInitialSync();
              }
              auto s = utils::httpMtxErrorToString(err).toStdString();
              nhlog::crypto()->critical(
                "failed to upload one time keys: {} {} \n({})", err->matrix_error.error, status_code,s);

              QString errorMsg(tr("Failed to setup encryption keys. Server response: "
                                  "%1 %2. Please try again later.")
                                 .arg(QString::fromStdString(err->matrix_error.error))
                                 .arg(status_code));

              emit dropToLogin(errorMsg);
              return;
          }

          olm::mark_keys_as_published();

          for (const auto &entry : res.one_time_key_counts)
              nhlog::net()->info("uploaded {} {} one-time keys", entry.second, entry.first);

          cache::client()->markUserKeysOutOfDate({http::client()->user_id().to_string()});

          startInitialSync();
      });
}

void
Client::startInitialSync()
{
    nhlog::net()->info("trying initial sync");

    mtx::http::SyncOpts opts;
    opts.timeout      = 0;
    opts.set_presence = currentPresence();

    http::client()->sync(opts, [this](const mtx::responses::Sync &res, mtx::http::RequestErr err) {
        // TODO: Initial Sync should include mentions as well...
        if (err) {
            const auto error      = QString::fromStdString(err->matrix_error.error);
            const auto msg        = tr("Please try to login again: %1").arg(error);
            const auto err_code   = mtx::errors::to_string(err->matrix_error.errcode);
            const int status_code = static_cast<int>(err->status_code);

            nhlog::net()->error("initial sync error: {} {} {} {}",
                                err->parse_error,
                                status_code,
                                err->error_code,
                                err_code);

            // non http related errors
            if (status_code <= 0 || status_code >= 600) {
                startInitialSync();
                return;
            }

            switch (status_code) {
                case 502:
                case 504:
                case 524: {
                    startInitialSync();
                    return;
                }
                default: {
                    emit dropToLogin(msg);
                    return;
                }
            }
        }

        nhlog::net()->info("initial sync completed");

        try {
            cache::client()->saveState(res);
            olm::handle_to_device_messages(res.to_device.events);
            cache::calculateRoomReadStatus();
            emit initialSync(std::move(res));
        } catch (const lmdb::error &e) {
            nhlog::db()->error("failed to save state after initial sync: {}", e.what());
            startInitialSync();
            return;
        }
        emit trySyncCb();
        emit prepareTimelines();
        auto up = new UserProfile("",utils::localUser());
    });
}

void
Client::handleSyncResponse(const mtx::responses::Sync &res, const QString &prev_batch_token)
{
    try {
        if (prev_batch_token.toStdString() != cache::nextBatchToken()) {
            nhlog::net()->warn("Duplicate sync, dropping");
            return;
        }
    } catch (const lmdb::error &e) {
        nhlog::db()->warn("Logged out in the mean time, dropping sync");
    }

    nhlog::net()->debug("sync completed: {}", res.next_batch);

    // Ensure that we have enough one-time keys available.
    ensureOneTimeKeyCount(res.device_one_time_keys_count);

    // TODO: fine grained error handling
    try {
        cache::client()->saveState(res);
        olm::handle_to_device_messages(res.to_device.events);

        auto updates = cache::getRoomInfo(cache::client()->roomsWithStateUpdates(res));

        if( res.rooms.join.size() || res.rooms.invite.size() || res.rooms.leave.size()) {
            syncTimelines(res.rooms);
            emit newUpdated(res);
        }
        // if we process a lot of syncs (1 every 200ms), this means we clean the
        // db every 100s
        static int syncCounter = 0;
        if (syncCounter++ >= 500) {
            cache::deleteOldData();
            syncCounter = 0;
        }
    } catch (const lmdb::map_full_error &e) {
        nhlog::db()->error("lmdb is full: {}", e.what());
        cache::deleteOldData();
    } catch (const lmdb::error &e) {
        nhlog::db()->error("saving sync response: {}", e.what());
    }

    emit trySyncCb();
}

void
Client::trySync()
{
    mtx::http::SyncOpts opts;
    opts.set_presence = currentPresence();

    if (!connectivityTimer_.isActive())
        connectivityTimer_.start();

    try {
        opts.since = cache::nextBatchToken();
    } catch (const lmdb::error &e) {
        nhlog::db()->error("failed to retrieve next batch token: {}", e.what());
        return;
    }

    http::client()->sync(
      opts, [this, since = opts.since](const mtx::responses::Sync &res, mtx::http::RequestErr err) {
          if (err) {
              const auto error      = QString::fromStdString(err->matrix_error.error);
              const auto msg        = tr("Please try to login again: %1").arg(error);
              const auto err_code   = mtx::errors::to_string(err->matrix_error.errcode);
              const int status_code = static_cast<int>(err->status_code);

              if ((http::is_logged_in() &&
                   (err->matrix_error.errcode == mtx::errors::ErrorCode::M_UNKNOWN_TOKEN ||
                    err->matrix_error.errcode == mtx::errors::ErrorCode::M_MISSING_TOKEN)) ||
                  !http::is_logged_in()) {
                  emit dropToLogin(msg);
                  return;
              }

              nhlog::net()->error("sync error: {} {} {} {}",
                                  err->parse_error,
                                  status_code,
                                  err->error_code,
                                  err_code);
              emit tryDelayedSyncCb();
              return;
          }

          emit newSyncResponse(res, QString::fromStdString(since));
      });
}

void
Client::joinRoom(const QString &room_id)
{
    joinRoomVia(room_id, {});
}

void
Client::joinRoomVia(const QString &room_id,
                      const std::vector<std::string> &via) {
    http::client()->join_room(
      room_id.toStdString(), via, [this, room_id](const mtx::responses::RoomId &roomId, mtx::http::RequestErr err) {
          if (err) {
              emit joinRoomFailed(QString::fromStdString(err->matrix_error.error));
              return;
          }

          emit joinedRoom(QString::fromStdString(roomId.room_id));

          // We remove any invites with the same room_id.
          try {
              cache::removeInvite(room_id.toStdString());
          } catch (const lmdb::error &e) {
              nhlog::db()->error("Failed to remove invite: {}", e.what()); // TODO error to user
          }
      });
}

void
Client::createRoom(const mtx::requests::CreateRoom &req)
{
    http::client()->create_room(
      req, [this](const mtx::responses::CreateRoom &res, mtx::http::RequestErr err) {
          if (err) {
              const auto err_code   = mtx::errors::to_string(err->matrix_error.errcode);
              const auto error      = err->matrix_error.error;
              const int status_code = static_cast<int>(err->status_code);
              nhlog::net()->warn("failed to create room: {} {} ({})", error, err_code, status_code);
              emit roomCreationFailed(QString::fromStdString(error));
              return;
          }

          nhlog::net()->info("Room {} created.",res.room_id.to_string());
          emit roomCreated(QString::fromStdString(res.room_id.to_string()));
      });
}

void
Client::leaveRoom(const QString &room_id)
{
    http::client()->leave_room(
      room_id.toStdString(),
      [this, room_id](const mtx::responses::Empty &, mtx::http::RequestErr err) {
          if (err) {
              emit roomLeaveFailed(QString::fromStdString(err->matrix_error.error));
              return;
          }
          emit leftRoom(room_id);
      });
}

void
Client::inviteUser(const QString &roomid,const QString &userid, const QString &reason)
{
    http::client()->invite_user(
      roomid.toStdString(),
      userid.toStdString(),
      [this, userid, roomid](const mtx::responses::Empty &, mtx::http::RequestErr err) {
          if (err) {
              emit userInvitationFailed(userid,
                                        roomid,
                                        QString::fromStdString(err->matrix_error.error));
          } else
              emit userInvited(roomid, userid);
      },
      reason.trimmed().toStdString());
}
void
Client::kickUser(const QString & roomid,const QString & userid, const QString & reason)
{
    http::client()->kick_user(
      roomid.toStdString(),
      userid.toStdString(),
      [this, userid, roomid](const mtx::responses::Empty &, mtx::http::RequestErr err) {
          if (err) {
            // TODO emit error
            //   emit showNotification(tr("Failed to kick %1 from %2: %3")
            //                           .arg(userid)
            //                           .arg(roomid)
            //                           .arg(QString::fromStdString(err->matrix_error.error)));
          } else {
            // TODO emit done
            //   emit showNotification(tr("Kicked user: %1").arg(userid));
          }
      },
      reason.trimmed().toStdString());
}
void
Client::banUser(const QString &roomid, const QString & userid, const QString & reason)
{
    http::client()->ban_user(
      roomid.toStdString(),
      userid.toStdString(),
      [this, userid, roomid](const mtx::responses::Empty &, mtx::http::RequestErr err) {
          if (err) {
            // TODO emit error
            //   emit showNotification(tr("Failed to ban %1 in %2: %3")
            //                           .arg(userid)
            //                           .arg(room)
            //                           .arg(QString::fromStdString(err->matrix_error.error)));
          } else {
            // TODO emit done
            //   emit showNotification(tr("Banned user: %1").arg(userid));
          }
      },
      reason.trimmed().toStdString());
}
void
Client::unbanUser(const QString  &roomid, const QString & userid, const QString & reason)
{
    http::client()->unban_user(
      roomid.toStdString(),
      userid.toStdString(),
      [this, userid, roomid](const mtx::responses::Empty &, mtx::http::RequestErr err) {
          if (err) {
            // TODO emit erro
            //   emit showNotification(tr("Failed to unban %1 in %2: %3")
            //                           .arg(userid)
            //                           .arg(room)
            //                           .arg(QString::fromStdString(err->matrix_error.error)));
          } else{
            // TODO emit done
            //   emit showNotification(tr("Unbanned user: %1").arg(userid));
          }
      },
      reason.trimmed().toStdString());
}

void
Client::receivedSessionKey(const QString &room_id, const QString &session_id)
{
    // nhlog::crypto()->warn("TODO: using {} and {}", room_id, session_id);
    // view_manager_->receivedSessionKey(room_id, session_id);
}

QString
Client::status() const
{
    return QString::fromStdString(cache::statusMessage(utils::localUser().toStdString()));
}

void
Client::setStatus(const QString &status)
{
    http::client()->put_presence_status(
      currentPresence(), status.toStdString(), [](mtx::http::RequestErr err) {
          if (err) {
              nhlog::net()->warn("failed to set presence status_msg: {}", err->matrix_error.error);
          }
      });
}

mtx::presence::PresenceState
Client::currentPresence() const
{
    switch (userSettings_->presence()) {
    case UserSettings::Presence::Online:
        return mtx::presence::online;
    case UserSettings::Presence::Unavailable:
        return mtx::presence::unavailable;
    case UserSettings::Presence::Offline:
        return mtx::presence::offline;
    default:
        return mtx::presence::online;
    }
}

void
Client::verifyOneTimeKeyCountAfterStartup()
{
    http::client()->upload_keys(
      olm::client()->create_upload_keys_request(),
      [this](const mtx::responses::UploadKeys &res, mtx::http::RequestErr err) {
          if (err) {
              nhlog::crypto()->warn("failed to update one-time keys: {} {} {}",
                                    err->matrix_error.error,
                                    static_cast<int>(err->status_code),
                                    static_cast<int>(err->error_code));

              if (err->status_code < 400 || err->status_code >= 500)
                  return;
          }

          std::map<std::string, uint16_t> key_counts;
          auto count = 0;
          if (auto c = res.one_time_key_counts.find(mtx::crypto::SIGNED_CURVE25519);
              c == res.one_time_key_counts.end()) {
              key_counts[mtx::crypto::SIGNED_CURVE25519] = 0;
          } else {
              key_counts[mtx::crypto::SIGNED_CURVE25519] = c->second;
              count                                      = c->second;
          }

          nhlog::crypto()->info(
            "Fetched server key count {} {}", count, mtx::crypto::SIGNED_CURVE25519);

          ensureOneTimeKeyCount(key_counts);
      });
}

void
Client::ensureOneTimeKeyCount(const std::map<std::string, uint16_t> &counts)
{
    if (auto count = counts.find(mtx::crypto::SIGNED_CURVE25519); count != counts.end()) {
        nhlog::crypto()->debug(
          "Updated server key count {} {}", count->second, mtx::crypto::SIGNED_CURVE25519);

        if (count->second < MAX_ONETIME_KEYS) {
            const int nkeys = MAX_ONETIME_KEYS - count->second;

            nhlog::crypto()->info("uploading {} {} keys", nkeys, mtx::crypto::SIGNED_CURVE25519);
            olm::client()->generate_one_time_keys(nkeys);

            http::client()->upload_keys(
              olm::client()->create_upload_keys_request(),
              [](const mtx::responses::UploadKeys &, mtx::http::RequestErr err) {
                  if (err) {
                      nhlog::crypto()->warn("failed to update one-time keys: {} {} {}",
                                            err->matrix_error.error,
                                            static_cast<int>(err->status_code),
                                            static_cast<int>(err->error_code));

                      if (err->status_code < 400 || err->status_code >= 500)
                          return;
                  }

                  // mark as published anyway, otherwise we may end up in a loop.
                  olm::mark_keys_as_published();
              });
        } else if (count->second > 2 * MAX_ONETIME_KEYS) {
            nhlog::crypto()->warn("too many one-time keys, deleting 1");
            mtx::requests::ClaimKeys req;
            req.one_time_keys[http::client()->user_id().to_string()][http::client()->device_id()] =
              std::string(mtx::crypto::SIGNED_CURVE25519);
            http::client()->claim_keys(
              req, [](const mtx::responses::ClaimKeys &, mtx::http::RequestErr err) {
                  if (err)
                      nhlog::crypto()->warn("failed to clear 1 one-time key: {} {} {}",
                                            err->matrix_error.error,
                                            static_cast<int>(err->status_code),
                                            static_cast<int>(err->error_code));
                  else
                      nhlog::crypto()->info("cleared 1 one-time key");
              });
        }
    }
}

void
Client::getProfileInfo(QString userid)
{
    http::client()->get_profile(
      userid.toStdString(), [this](const mtx::responses::Profile &res, mtx::http::RequestErr err) {
          if (err) {
              auto s = utils::httpMtxErrorToString(err).toStdString();
              nhlog::net()->warn("failed to retrieve own profile info ({})", s);
              return;
          }
          emit userDisplayNameReady(QString::fromStdString(res.display_name));
          emit userAvatarReady(QString::fromStdString(res.avatar_url));
      });
}

void
Client::getBackupVersion()
{
    if (!UserSettings::instance()->useOnlineKeyBackup()) {
        nhlog::crypto()->info("Online key backup disabled.");
        return;
    }

    http::client()->backup_version(
      [this](const mtx::responses::backup::BackupVersion &res, mtx::http::RequestErr err) {
          if (err) {
              nhlog::net()->warn("Failed to retrieve backup version");
              if (err->status_code == 404)
                  cache::client()->deleteBackupVersion();
              return;
          }

          // switch to UI thread for secrets stuff
          QTimer::singleShot(0, this, [res] {
              auto auth_data = nlohmann::json::parse(res.auth_data);

              if (res.algorithm == "m.megolm_backup.v1.curve25519-aes-sha2") {
                  auto key = cache::secret(mtx::secret_storage::secrets::megolm_backup_v1);
                  if (!key) {
                      nhlog::crypto()->info("No key for online key backup.");
                      cache::client()->deleteBackupVersion();
                      return;
                  }

                  using namespace mtx::crypto;
                  auto pubkey = CURVE25519_public_key_from_private(to_binary_buf(base642bin(*key)));

                  if (auth_data["public_key"].get<std::string>() != pubkey) {
                      nhlog::crypto()->info("Our backup key {} does not match the one "
                                            "used in the online backup {}",
                                            pubkey,
                                            auth_data["public_key"]);
                      cache::client()->deleteBackupVersion();
                      return;
                  }

                  nhlog::crypto()->info("Using online key backup.");
                  OnlineBackupVersion data{};
                  data.algorithm = res.algorithm;
                  data.version   = res.version;
                  cache::client()->saveBackupVersion(data);
              } else {
                  nhlog::crypto()->info("Unsupported key backup algorithm: {}", res.algorithm);
                  cache::client()->deleteBackupVersion();
              }
          });
      });
}

void
Client::decryptDownloadedSecrets(const std::string &recoveryKey, mtx::secret_storage::AesHmacSha2KeyDescription keyDesc,
                                   const SecretsToDecrypt &secrets)
{
    if (recoveryKey.empty())
        return;

    auto decryptionKey = mtx::crypto::key_from_recoverykey(recoveryKey, keyDesc);

    if (!decryptionKey && keyDesc.passphrase) {
        try {
            decryptionKey = mtx::crypto::key_from_passphrase(recoveryKey, keyDesc);
        } catch (std::exception &e) {
            nhlog::crypto()->error("Failed to derive secret key from passphrase: {}", e.what());
        }
    }

    // if (!decryptionKey) {
    //     QMessageBox::information(
    //       Client::instance(),
    //       QCoreApplication::translate("CrossSigningSecrets", "Decryption failed"),
    //       QCoreApplication::translate("CrossSigningSecrets",
    //                                   "Failed to decrypt secrets with the "
    //                                   "provided recovery key or passphrase"));
    //     return;
    // }

    auto deviceKeys = cache::client()->userKeys(http::client()->user_id().to_string());
    mtx::requests::KeySignaturesUpload req;

    for (const auto &[secretName, encryptedSecret] : secrets) {
        auto decrypted = mtx::crypto::decrypt(encryptedSecret, *decryptionKey, secretName);
        if (!decrypted.empty()) {
            cache::storeSecret(secretName, decrypted);

            if (deviceKeys && deviceKeys->device_keys.count(http::client()->device_id()) &&
                secretName == mtx::secret_storage::secrets::cross_signing_self_signing) {
                auto myKey = deviceKeys->device_keys.at(http::client()->device_id());
                if (myKey.user_id == http::client()->user_id().to_string() &&
                    myKey.device_id == http::client()->device_id() &&
                    myKey.keys["ed25519:" + http::client()->device_id()] ==
                      olm::client()->identity_keys().ed25519 &&
                    myKey.keys["curve25519:" + http::client()->device_id()] ==
                      olm::client()->identity_keys().curve25519) {
                    json j = myKey;
                    j.erase("signatures");
                    j.erase("unsigned");

                    auto ssk = mtx::crypto::PkSigning::from_seed(decrypted);
                    myKey.signatures[http::client()->user_id().to_string()]
                                    ["ed25519:" + ssk.public_key()] = ssk.sign(j.dump());
                    req.signatures[http::client()->user_id().to_string()]
                                  [http::client()->device_id()] = myKey;
                }
            } else if (deviceKeys &&
                       secretName == mtx::secret_storage::secrets::cross_signing_master) {
                auto mk = mtx::crypto::PkSigning::from_seed(decrypted);

                if (deviceKeys->master_keys.user_id == http::client()->user_id().to_string() &&
                    deviceKeys->master_keys.keys["ed25519:" + mk.public_key()] == mk.public_key()) {
                    json j = deviceKeys->master_keys;
                    j.erase("signatures");
                    j.erase("unsigned");
                    mtx::crypto::CrossSigningKeys master_key = j;
                    master_key.signatures[http::client()->user_id().to_string()]
                                         ["ed25519:" + http::client()->device_id()] =
                      olm::client()->sign_message(j.dump());
                    req.signatures[http::client()->user_id().to_string()][mk.public_key()] =
                      master_key;
                }
            }
        }
    }

    if (!req.signatures.empty())
        http::client()->keys_signatures_upload(
          req, [](const mtx::responses::KeySignaturesUpload &res, mtx::http::RequestErr err) {
              if (err) {
                  nhlog::net()->error("failed to upload signatures: {},{}",
                                      mtx::errors::to_string(err->matrix_error.errcode),
                                      static_cast<int>(err->status_code));
              }

              for (const auto &[user_id, tmp] : res.errors)
                  for (const auto &[key_id, e] : tmp)
                      nhlog::net()->error("signature error for user '{}' and key "
                                          "id {}: {}, {}",
                                          user_id,
                                          key_id,
                                          mtx::errors::to_string(e.errcode),
                                          e.error);
          });
}

void
Client::startChat(QString userid)
{
    auto joined_rooms = cache::joinedRooms();
    auto room_infos   = cache::getRoomInfo(joined_rooms);

    for (std::string room_id : joined_rooms) {
        if (room_infos[QString::fromStdString(room_id)].member_count == 2) {
            auto room_members = cache::roomMembers(room_id);
            if (std::find(room_members.begin(), room_members.end(), (userid).toStdString()) !=
                room_members.end()) {
                // view_manager_->rooms()->setCurrentRoom(QString::fromStdString(room_id));
                return;
            }
        }
    }

    mtx::requests::CreateRoom req;
    req.preset     = mtx::requests::Preset::PrivateChat;
    req.visibility = mtx::common::RoomVisibility::Private;
    if (utils::localUser() != userid) {
        req.invite    = {userid.toStdString()};
        req.is_direct = true;
    }
    emit Client::instance()->createRoom(req);
}

void Client::loginWithPassword(QString deviceName, QString userId, QString password, QString serverAddress){
    
    _authentication->loginWithPassword(deviceName.toStdString(), userId.toStdString(), password.toStdString(), serverAddress.toStdString());
   
}

bool Client::hasValidUser(){
     if (UserSettings::instance()->accessToken() != "") {
        return true;
    }
    return false;
}

UserInformation Client::userInformation(){
    using namespace mtx::identifiers;
    UserInformation result;    
    // res.user_id    = parse<User>(http::client()->user_id().to_string());
    result.userId         = UserSettings::instance()->userId();
    result.deviceId        = UserSettings::instance()->deviceId();
    result.accessToken    = UserSettings::instance()->accessToken();
    result.homeServer       = UserSettings::instance()->homeserver();

    return result;
}

void Client::logout(){
    _authentication->logout();
}

std::string Client::serverDiscovery(QString userId){
    return _authentication->serverDiscovery(userId.toStdString());
}

void Client::start(QString userId, QString homeServer, QString token){
    if(userId.isEmpty()){
        if(hasValidUser()){
          auto info = userInformation();
          userId = info.userId;
          homeServer = info.homeServer;
          token = info.accessToken;            
        }else{
            //to make sure signal will recieved via reciever
            QTimer::singleShot(500, this, [&] {
                emit dropToLogin("Need to loging in!");
            });            
            return;
        }
    }
    bootstrap(userId.toStdString(), homeServer.toStdString(), token.toStdString());
}

void Client::stop(){
    if (http::client() != nullptr) {
        nhlog::net()->debug("shutting down all I/O threads & open connections");
        http::client()->close(true);
        nhlog::net()->debug("bye");
    }
}

void Client::syncTimelines(const mtx::responses::Rooms &rooms){
    for(auto const &r: rooms.join){
        addTimeline(QString::fromStdString(r.first));
        syncTimeline(QString::fromStdString(r.first), r.second);
    }
    for(auto const &r: rooms.leave){
        removeTimeline(QString::fromStdString(r.first));
    }
}

void Client::syncTimeline(const QString &roomId, const mtx::responses::JoinedRoom &room){
    auto it = _timelines.find(roomId);
    if(it != _timelines.end()){
        it.value()->sync(room);
    }
}

void Client::createTimelinesFromDB(){
    auto rooms = joinedRoomList();
    for(auto const &r: rooms.toStdMap()){
        addTimeline(r.first);
    }
}

void Client::addTimeline(const QString &roomID){
    auto it = _timelines.find(roomID);
    if(it == _timelines.end()){
        _timelines[roomID] = new Timeline(roomID);
        _timelines[roomID]->initialSync();
    }
}

void Client::removeTimeline(const QString &roomID){
    auto it = _timelines.find(roomID);
    if(it != _timelines.end()){
        auto timeline = _timelines[roomID];
        _timelines.remove(roomID);
        delete timeline;
    }
}
