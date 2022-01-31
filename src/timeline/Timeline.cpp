#include "Timeline.h"
#include <QDateTime>
#include <mtx/responses/common.hpp>
#include <mtx/events.hpp>
#include <QDebug>
#include "EventAccessors.h"
#include "Client.h"
#include "Cache_p.h"
#include "MatrixClient.h"

Timeline::Timeline(const QString &roomId, QObject *parent):
    QObject(parent),
    _events(roomId.toStdString(), this),
    _roomId(roomId)
    {
    nhlog::dev()->debug("Timeline created for: \"" + roomId.toStdString() + "\"");
    connect(this,
            &Timeline::newMessageToSend,
            this,
            &Timeline::addPendingMessage,
            Qt::QueuedConnection);
    connect(this, &Timeline::addPendingMessageToStore, &_events, &EventStore::addPending);
    connect(&_events, &EventStore::dataChanged, this, [this](int from, int to) {
        // relatedEventCacheBuster++;
        nhlog::ui()->debug(
          "data changed {} to {}", _events.size() - to - 1, _events.size() - from - 1);
        // emit eventsChanged(_events.size() - to - 1, _events.size() - from - 1);
    });
    connect(&_events, &EventStore::beginInsertRows, this, [this](int from, int to) {
        nhlog::ui()->debug("begin insert from {} to {} (size: {})", from, to, to - from + 1);
        emit newEventsStored(from, to - from + 1);
    });
    // connect(&_events, &EventStore::endInsertRows, this, [this]() {
        // endInsertRows(); 
    // });
    // connect(&_events, &EventStore::beginResetModel, this, [this]() {
        // beginResetModel(); 
    // });
    // connect(&_events, &EventStore::endResetModel, this, [this]() { 
        // endResetModel(); 
    // });
    // connect(&_events, &EventStore::newEncryptedImage, this, &TimelineModel::newEncryptedImage);
    // connect(&_events, &EventStore::fetchedMore, this, [this]() { 
        // setPaginationInProgress(false); 
    // });
    connect(&_events,
            &EventStore::startDMVerification,
            this,
            [this](mtx::events::RoomEvent<mtx::events::msg::KeyVerificationRequest> msg) {
                (void)msg; // TODO
                // ChatPage::instance()->receivedRoomDeviceVerificationRequest(msg, this);
            });
    connect(&_events, &EventStore::updateFlowEventId, this, [this](std::string event_id) {
        (void)event_id; // TODO
        // this->updateFlowEventId(event_id);
    });

    // When a message is sent, check if the current edit/reply relates to that message,
    // and update the event_id so that it points to the sent message and not the pending one.
    connect(
      &_events, &EventStore::messageSent, this, [this](std::string txn_id, std::string event_id) {
          (void)txn_id; (void)event_id; // TODO
        //   if (edit_.toStdString() == txn_id) {
        //       edit_ = QString::fromStdString(event_id);
        //       emit editChanged(edit_);
        //   }
        //   if (reply_.toStdString() == txn_id) {
        //       reply_ = QString::fromStdString(event_id);
        //       emit replyChanged(reply_);
        //   }
      });
}

template<typename T>
void
Timeline::sendEncryptedMessage(mtx::events::RoomEvent<T> msg, mtx::events::EventType eventType)
{
    const auto room_id = _roomId.toStdString();
    using namespace mtx::events;
    using namespace mtx::identifiers;

    json doc = {{"type", mtx::events::to_string(eventType)},
                {"content", json(msg.content)},
                {"room_id", room_id}};

    try {
        mtx::events::EncryptedEvent<mtx::events::msg::Encrypted> event;
        event.content  = olm::encrypt_group_message(room_id, http::client()->device_id(), doc);
        event.event_id = msg.event_id;
        event.room_id  = room_id;
        event.sender   = http::client()->user_id().to_string();
        event.type     = mtx::events::EventType::RoomEncrypted;
        event.origin_server_ts = QDateTime::currentMSecsSinceEpoch();

        emit addPendingMessageToStore(event);

        // TODO: Let the user know about the errors.
    } catch (const lmdb::error &e) {
        nhlog::db()->critical("failed to open outbound megolm session ({}): {}", room_id, e.what());
        // emit ChatPage::instance()->showNotification(
        //   tr("Failed to encrypt event, sending aborted!"));
    } catch (const mtx::crypto::olm_exception &e) {
        nhlog::crypto()->critical(
          "failed to open outbound megolm session ({}): {}", room_id, e.what());
     //     emit ChatPage::instance()->showNotification(
     //       tr("Failed to encrypt event, sending aborted!"));
    }
}

template<class T>
void
Timeline::sendMessageEvent(const T &content, mtx::events::EventType eventType)
{
    if constexpr (std::is_same_v<T, mtx::events::msg::StickerImage>) {
        mtx::events::Sticker msgCopy = {};
        msgCopy.content              = content;
        msgCopy.type                 = eventType;
        emit newMessageToSend(msgCopy);
    } else {
        mtx::events::RoomEvent<T> msgCopy = {};
        msgCopy.content                   = content;
        msgCopy.type                      = eventType;
        emit newMessageToSend(msgCopy);
    }
}

struct SendMessageVisitor
{
    explicit SendMessageVisitor(Timeline *timeline)
      : _timeline(timeline)
    {}

    template<typename T, mtx::events::EventType Event>
    void sendRoomEvent(mtx::events::RoomEvent<T> msg)
    {
        if (cache::isRoomEncrypted(_timeline->_roomId.toStdString())) {
            auto encInfo = mtx::accessors::file(msg);
            if (encInfo)
                emit _timeline->newEncryptedImage(encInfo.value());

            _timeline->sendEncryptedMessage(msg, Event);
        } else {
            msg.type = Event;
            emit _timeline->addPendingMessageToStore(msg);
        }
    }

    // Do-nothing operator for all unhandled events
    template<typename T>
    void operator()(const mtx::events::Event<T> &)
    {}

    // Operator for m.room.message events that contain a msgtype in their content
    template<typename T,
             std::enable_if_t<std::is_same<decltype(T::msgtype), std::string>::value, int> = 0>
    void operator()(mtx::events::RoomEvent<T> msg)
    {
        sendRoomEvent<T, mtx::events::EventType::RoomMessage>(msg);
    }

    // Special operator for reactions, which are a type of m.room.message, but need to be
    // handled distinctly for their differences from normal room messages.  Specifically,
    // reactions need to have the relation outside of ciphertext, or synapse / the homeserver
    // cannot handle it correctly.  See the MSC for more details:
    // https://github.com/matrix-org/matrix-doc/blob/matthew/msc1849/proposals/1849-aggregations.md#end-to-end-encryption
    void operator()(mtx::events::RoomEvent<mtx::events::msg::Reaction> msg)
    {
        msg.type = mtx::events::EventType::Reaction;
        emit _timeline->addPendingMessageToStore(msg);
    }

    void operator()(const mtx::events::RoomEvent<mtx::events::msg::CallInvite> &event)
    {
        sendRoomEvent<mtx::events::msg::CallInvite, mtx::events::EventType::CallInvite>(event);
    }

    void operator()(const mtx::events::RoomEvent<mtx::events::msg::CallCandidates> &event)
    {
        sendRoomEvent<mtx::events::msg::CallCandidates, mtx::events::EventType::CallCandidates>(
          event);
    }

    void operator()(const mtx::events::RoomEvent<mtx::events::msg::CallAnswer> &event)
    {
        sendRoomEvent<mtx::events::msg::CallAnswer, mtx::events::EventType::CallAnswer>(event);
    }

    void operator()(const mtx::events::RoomEvent<mtx::events::msg::CallHangUp> &event)
    {
        sendRoomEvent<mtx::events::msg::CallHangUp, mtx::events::EventType::CallHangUp>(event);
    }

    void operator()(const mtx::events::RoomEvent<mtx::events::msg::KeyVerificationRequest> &msg)
    {
        sendRoomEvent<mtx::events::msg::KeyVerificationRequest,
                      mtx::events::EventType::RoomMessage>(msg);
    }

    void operator()(const mtx::events::RoomEvent<mtx::events::msg::KeyVerificationReady> &msg)
    {
        sendRoomEvent<mtx::events::msg::KeyVerificationReady,
                      mtx::events::EventType::KeyVerificationReady>(msg);
    }

    void operator()(const mtx::events::RoomEvent<mtx::events::msg::KeyVerificationStart> &msg)
    {
        sendRoomEvent<mtx::events::msg::KeyVerificationStart,
                      mtx::events::EventType::KeyVerificationStart>(msg);
    }

    void operator()(const mtx::events::RoomEvent<mtx::events::msg::KeyVerificationAccept> &msg)
    {
        sendRoomEvent<mtx::events::msg::KeyVerificationAccept,
                      mtx::events::EventType::KeyVerificationAccept>(msg);
    }

    void operator()(const mtx::events::RoomEvent<mtx::events::msg::KeyVerificationMac> &msg)
    {
        sendRoomEvent<mtx::events::msg::KeyVerificationMac,
                      mtx::events::EventType::KeyVerificationMac>(msg);
    }

    void operator()(const mtx::events::RoomEvent<mtx::events::msg::KeyVerificationKey> &msg)
    {
        sendRoomEvent<mtx::events::msg::KeyVerificationKey,
                      mtx::events::EventType::KeyVerificationKey>(msg);
    }

    void operator()(const mtx::events::RoomEvent<mtx::events::msg::KeyVerificationDone> &msg)
    {
        sendRoomEvent<mtx::events::msg::KeyVerificationDone,
                      mtx::events::EventType::KeyVerificationDone>(msg);
    }

    void operator()(const mtx::events::RoomEvent<mtx::events::msg::KeyVerificationCancel> &msg)
    {
        sendRoomEvent<mtx::events::msg::KeyVerificationCancel,
                      mtx::events::EventType::KeyVerificationCancel>(msg);
    }
    void operator()(mtx::events::Sticker msg)
    {
        msg.type = mtx::events::EventType::Sticker;
        if (cache::isRoomEncrypted(_timeline->_roomId.toStdString())) {
            _timeline->sendEncryptedMessage(msg, mtx::events::EventType::Sticker);
        } else
            emit _timeline->addPendingMessageToStore(msg);
    }

    Timeline *_timeline;
};

void
Timeline::addPendingMessage(mtx::events::collections::TimelineEvents event)
{
    std::visit(
      [](auto &msg) {
          // gets overwritten for reactions and stickers in SendMessageVisitor
          msg.type             = mtx::events::EventType::RoomMessage;
          msg.event_id         = "m" + http::client()->generate_txn_id();
          msg.sender           = http::client()->user_id().to_string();
          msg.origin_server_ts = QDateTime::currentMSecsSinceEpoch();
      },
      event);

    std::visit(SendMessageVisitor{this}, event);
}

void Timeline::sendMessage(const QString &msg) {
    mtx::events::msg::Text text = {};
    text.body                   = msg.trimmed().toStdString();
    sendMessageEvent(text, mtx::events::EventType::RoomMessage);
}

void Timeline::sync(const mtx::responses::JoinedRoom &room){
    // this->syncState(room.state);
    addEvents(room.timeline);
    if (room.unread_notifications.highlight_count != _highlightCount ||
        room.unread_notifications.notification_count != _notificationCount) {
        _notificationCount = room.unread_notifications.notification_count;
        _highlightCount    = room.unread_notifications.highlight_count;
        emit notificationsChanged();
    }
    for (const auto &ev : room.ephemeral.events) {
        if (auto t = std::get_if<mtx::events::EphemeralEvent<mtx::events::ephemeral::Typing>>(&ev)) {
            QStringList typing;
            typing.reserve(t->content.user_ids.size());
            for (const auto &user : t->content.user_ids) {
                if (user != http::client()->user_id().to_string())
                    typing << QString::fromStdString(user);
            }
            updateTypingUsers(typing);
        }
    }
}

void Timeline::initialSync(){
    // if(canFetchMore()){
    //     _events.fetchMore();
    // }
}

bool Timeline::canFetchMore()const {
    if (!_events.size())
        return true;
    if (auto first = _events.get(0);
        first &&
        !std::holds_alternative<mtx::events::StateEvent<mtx::events::state::Create>>(*first))
        return true;
    else
        return false;
}

void Timeline::addEvents(const mtx::responses::Timeline &timeline){
    if (timeline.events.empty())
        return;

    _events.handleSync(timeline);

    using namespace mtx::events;

    for (auto e : timeline.events) {
        if (auto encryptedEvent = std::get_if<EncryptedEvent<msg::Encrypted>>(&e)) {
            MegolmSessionIndex index(_roomId.toStdString(), encryptedEvent->content);

            auto result = olm::decryptEvent(index, *encryptedEvent);
            if (result.event)
                e = result.event.value();
        }

    //     if (std::holds_alternative<RoomEvent<msg::CallCandidates>>(e) ||
    //         std::holds_alternative<RoomEvent<msg::CallInvite>>(e) ||
    //         std::holds_alternative<RoomEvent<msg::CallAnswer>>(e) ||
    //         std::holds_alternative<RoomEvent<msg::CallHangUp>>(e))
    //         std::visit(
    //           [this](auto &event) {
    //               event.room_id = room_id_.toStdString();
    //               if constexpr (std::is_same_v<std::decay_t<decltype(event)>,
    //                                            RoomEvent<msg::CallAnswer>> ||
    //                             std::is_same_v<std::decay_t<decltype(event)>,
    //                                            RoomEvent<msg::CallHangUp>>)
    //                   emit newCallEvent(event);
    //               else {
    //                   if (event.sender != http::client()->user_id().to_string())
    //                       emit newCallEvent(event);
    //               }
    //           },
    //           e);
    //     else if (std::holds_alternative<StateEvent<state::Avatar>>(e))
    //         emit roomAvatarUrlChanged();
    //     else if (std::holds_alternative<StateEvent<state::Name>>(e))
    //         emit roomNameChanged();
    //     else if (std::holds_alternative<StateEvent<state::Topic>>(e))
    //         emit roomTopicChanged();
    //     else if (std::holds_alternative<StateEvent<state::PowerLevels>>(e)) {
    //         permissions_.invalidate();
    //         emit permissionsChanged();
    //     } else if (std::holds_alternative<StateEvent<state::Member>>(e)) {
    //         emit roomAvatarUrlChanged();
    //         emit roomNameChanged();
    //         emit roomMemberCountChanged();
    //     } else if (std::holds_alternative<StateEvent<state::Encryption>>(e)) {
    //         this->isEncrypted_ = cache::isRoomEncrypted(room_id_.toStdString());
    //         emit encryptionChanged();
    //     }
    }
    updateLastMessage();
}

// Workaround. We also want to see a room at the top, if we just joined it
auto
isYourJoin(const mtx::events::StateEvent<mtx::events::state::Member> &e)
{
    return e.content.membership == mtx::events::state::Membership::Join &&
           e.state_key == http::client()->user_id().to_string();
}
template<typename T>
auto
isYourJoin(const mtx::events::Event<T> &)
{
    return false;
}


template<typename T>
auto
isMessage(const mtx::events::RoomEvent<T> &e)
  -> std::enable_if_t<std::is_same<decltype(e.content.msgtype), std::string>::value, bool>
{
    return true;
}

template<typename T>
auto
isMessage(const mtx::events::Event<T> &)
{
    return false;
}

template<typename T>
auto
isMessage(const mtx::events::EncryptedEvent<T> &)
{
    return true;
}

auto
isMessage(const mtx::events::RoomEvent<mtx::events::msg::CallInvite> &)
{
    return true;
}

auto
isMessage(const mtx::events::RoomEvent<mtx::events::msg::CallAnswer> &)
{
    return true;
}
auto
isMessage(const mtx::events::RoomEvent<mtx::events::msg::CallHangUp> &)
{
    return true;
}

void Timeline::updateLastMessage(){
    for (auto it = _events.size() - 1; it >= 0; --it) {
        auto event = _events.get(it, _decryptDescription);
        if (!event)
            continue;
        if (std::visit([](const auto &e) -> bool { return isYourJoin(e); }, *event)) {
            auto time   = mtx::accessors::origin_server_ts(*event);
            uint64_t ts = time.toMSecsSinceEpoch();
            auto description = DescInfo{QString::fromStdString(mtx::accessors::event_id(*event)),
                                QString::fromStdString(http::client()->user_id().to_string()),
                                tr("You joined this room."),
                                utils::descriptiveTime(time),
                                ts,
                                time,true};
            if (description != _lastMessage) {
                _lastMessage = description;
                emit lastMessageChanged(_lastMessage);
            }
            return;
        }
        if (!std::visit([](const auto &e) -> bool { return isMessage(e); }, *event))
            continue;

        auto description = utils::getMessageDescription(
          *event,
          QString::fromStdString(http::client()->user_id().to_string()),
          cache::displayName(_roomId, QString::fromStdString(mtx::accessors::sender(*event))));
        if (description != _lastMessage) {
            _lastMessage = description;
            emit lastMessageChanged(_lastMessage);
        }
        return;
    }
}

QVector<DescInfo> Timeline::getEvents(int from, int len, bool markAsRead){
    QVector<DescInfo> events;
    QStringList eventIds;
    for(int i = from; i < from + len && i < _events.size(); i++){
        auto e = _events.get(i,true);
        if(e) {
            auto descMsg = utils::getMessageDescription(*e, 
                    utils::localUser(), 
                    cache::displayName(_roomId, QString::fromStdString(mtx::accessors::sender(*e))));
            events.push_back(descMsg);
            eventIds << descMsg.event_id;
        }
    }
    if(markAsRead){
        markEventsAsRead(eventIds);
    }
    return events;
}

void Timeline::markEventsAsRead(const QStringList &event_ids){
    for(auto const &id: event_ids){
        http::client()->read_event(_roomId.toStdString(), id.toStdString(), [this, id](mtx::http::RequestErr err) {
            if (err) {
                nhlog::net()->warn(
                    "failed to read_event ({}, {})", _roomId.toStdString(), id.toStdString());
            }
        });
    }
}

QString Timeline::displayName(QString id) const {
    return cache::displayName(_roomId, id).toHtmlEscaped();
}

QString Timeline::avatarUrl(QString id) const {
    return cache::avatarUrl(_roomId, id);
}