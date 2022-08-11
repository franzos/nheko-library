#include "Timeline.h"
#include <QDateTime>
#include <mtx/responses/common.hpp>
#include <mtx/events.hpp>
#include <mtx/responses/media.hpp>
#include <QDebug>
#include "EventAccessors.h"
#include "Client.h"
#include "Cache_p.h"
#include "MatrixClient.h"
#include "../Utils.h"


qml_mtx_events::EventType
qml_mtx_events::toRoomEventType(mtx::events::EventType e)
{
    using mtx::events::EventType;
    switch (e) {
    case EventType::RoomKeyRequest:
        return qml_mtx_events::EventType::KeyRequest;
    case EventType::Reaction:
        return qml_mtx_events::EventType::Reaction;
    case EventType::RoomAliases:
        return qml_mtx_events::EventType::Aliases;
    case EventType::RoomAvatar:
        return qml_mtx_events::EventType::Avatar;
    case EventType::RoomCanonicalAlias:
        return qml_mtx_events::EventType::CanonicalAlias;
    case EventType::RoomCreate:
        return qml_mtx_events::EventType::RoomCreate;
    case EventType::RoomEncrypted:
        return qml_mtx_events::EventType::Encrypted;
    case EventType::RoomEncryption:
        return qml_mtx_events::EventType::Encryption;
    case EventType::RoomGuestAccess:
        return qml_mtx_events::EventType::RoomGuestAccess;
    case EventType::RoomHistoryVisibility:
        return qml_mtx_events::EventType::RoomHistoryVisibility;
    case EventType::RoomJoinRules:
        return qml_mtx_events::EventType::RoomJoinRules;
    case EventType::RoomMember:
        return qml_mtx_events::EventType::Member;
    case EventType::RoomMessage:
        return qml_mtx_events::EventType::UnknownMessage;
    case EventType::RoomName:
        return qml_mtx_events::EventType::Name;
    case EventType::RoomPowerLevels:
        return qml_mtx_events::EventType::PowerLevels;
    case EventType::RoomTopic:
        return qml_mtx_events::EventType::Topic;
    case EventType::RoomTombstone:
        return qml_mtx_events::EventType::Tombstone;
    case EventType::RoomRedaction:
        return qml_mtx_events::EventType::Redaction;
    case EventType::RoomPinnedEvents:
        return qml_mtx_events::EventType::PinnedEvents;
    case EventType::Sticker:
        return qml_mtx_events::EventType::Sticker;
    case EventType::Tag:
        return qml_mtx_events::EventType::Tag;
    case EventType::PolicyRuleUser:
        return qml_mtx_events::EventType::PolicyRuleUser;
    case EventType::PolicyRuleRoom:
        return qml_mtx_events::EventType::PolicyRuleRoom;
    case EventType::PolicyRuleServer:
        return qml_mtx_events::EventType::PolicyRuleServer;
    case EventType::SpaceParent:
        return qml_mtx_events::EventType::SpaceParent;
    case EventType::SpaceChild:
        return qml_mtx_events::EventType::SpaceChild;
    case EventType::ImagePackInRoom:
        return qml_mtx_events::ImagePackInRoom;
    case EventType::ImagePackInAccountData:
        return qml_mtx_events::ImagePackInAccountData;
    case EventType::ImagePackRooms:
        return qml_mtx_events::ImagePackRooms;
    case EventType::Unsupported:
        return qml_mtx_events::EventType::Unsupported;
    default:
        return qml_mtx_events::EventType::UnknownMessage;
    }
}

qml_mtx_events::EventType
qml_mtx_events::toRoomEventType(const mtx::events::collections::TimelineEvents &event)
{
    return std::visit(RoomEventType{}, event);
}

QString
qml_mtx_events::toRoomEventTypeString(const mtx::events::collections::TimelineEvents &event)
{
    return std::visit([](const auto &e) { return QString::fromStdString(to_string(e.type)); },
                      event);
}

mtx::events::EventType
qml_mtx_events::fromRoomEventType(qml_mtx_events::EventType t)
{
    switch (t) {
    // Unsupported event
    case qml_mtx_events::Unsupported:
        return mtx::events::EventType::Unsupported;

    /// m.room_key_request
    case qml_mtx_events::KeyRequest:
        return mtx::events::EventType::RoomKeyRequest;
    /// m.reaction:
    case qml_mtx_events::Reaction:
        return mtx::events::EventType::Reaction;
    /// m.room.aliases
    case qml_mtx_events::Aliases:
        return mtx::events::EventType::RoomAliases;
    /// m.room.avatar
    case qml_mtx_events::Avatar:
        return mtx::events::EventType::RoomAvatar;
    /// m.call.invite
    case qml_mtx_events::CallInvite:
        return mtx::events::EventType::CallInvite;
    /// m.call.answer
    case qml_mtx_events::CallAnswer:
        return mtx::events::EventType::CallAnswer;
    /// m.call.hangup
    case qml_mtx_events::CallHangUp:
        return mtx::events::EventType::CallHangUp;
    /// m.call.candidates
    case qml_mtx_events::CallCandidates:
        return mtx::events::EventType::CallCandidates;
    /// m.room.canonical_alias
    case qml_mtx_events::CanonicalAlias:
        return mtx::events::EventType::RoomCanonicalAlias;
    /// m.room.create
    case qml_mtx_events::RoomCreate:
        return mtx::events::EventType::RoomCreate;
    /// m.room.encrypted.
    case qml_mtx_events::Encrypted:
        return mtx::events::EventType::RoomEncrypted;
    /// m.room.encryption.
    case qml_mtx_events::Encryption:
        return mtx::events::EventType::RoomEncryption;
    /// m.room.guest_access
    case qml_mtx_events::RoomGuestAccess:
        return mtx::events::EventType::RoomGuestAccess;
    /// m.room.history_visibility
    case qml_mtx_events::RoomHistoryVisibility:
        return mtx::events::EventType::RoomHistoryVisibility;
    /// m.room.join_rules
    case qml_mtx_events::RoomJoinRules:
        return mtx::events::EventType::RoomJoinRules;
    /// m.room.member
    case qml_mtx_events::Member:
        return mtx::events::EventType::RoomMember;
    /// m.room.name
    case qml_mtx_events::Name:
        return mtx::events::EventType::RoomName;
    /// m.room.power_levels
    case qml_mtx_events::PowerLevels:
        return mtx::events::EventType::RoomPowerLevels;
    /// m.room.tombstone
    case qml_mtx_events::Tombstone:
        return mtx::events::EventType::RoomTombstone;
    /// m.room.topic
    case qml_mtx_events::Topic:
        return mtx::events::EventType::RoomTopic;
    /// m.room.redaction
    case qml_mtx_events::Redaction:
        return mtx::events::EventType::RoomRedaction;
    /// m.room.pinned_events
    case qml_mtx_events::PinnedEvents:
        return mtx::events::EventType::RoomPinnedEvents;
    /// m.widget
    case qml_mtx_events::Widget:
        return mtx::events::EventType::Widget;
    // m.sticker
    case qml_mtx_events::Sticker:
        return mtx::events::EventType::Sticker;
    // m.tag
    case qml_mtx_events::Tag:
        return mtx::events::EventType::Tag;
    case qml_mtx_events::PolicyRuleUser:
        return mtx::events::EventType::PolicyRuleUser;
    case qml_mtx_events::PolicyRuleRoom:
        return mtx::events::EventType::PolicyRuleRoom;
    case qml_mtx_events::PolicyRuleServer:
        return mtx::events::EventType::PolicyRuleServer;
    // m.space.parent
    case qml_mtx_events::SpaceParent:
        return mtx::events::EventType::SpaceParent;
    // m.space.child
    case qml_mtx_events::SpaceChild:
        return mtx::events::EventType::SpaceChild;
    /// m.room.message
    case qml_mtx_events::AudioMessage:
    case qml_mtx_events::EmoteMessage:
    case qml_mtx_events::FileMessage:
    case qml_mtx_events::ImageMessage:
    case qml_mtx_events::LocationMessage:
    case qml_mtx_events::NoticeMessage:
    case qml_mtx_events::TextMessage:
    case qml_mtx_events::VideoMessage:
    case qml_mtx_events::Redacted:
    case qml_mtx_events::UnknownMessage:
    case qml_mtx_events::KeyVerificationRequest:
    case qml_mtx_events::KeyVerificationStart:
    case qml_mtx_events::KeyVerificationMac:
    case qml_mtx_events::KeyVerificationAccept:
    case qml_mtx_events::KeyVerificationCancel:
    case qml_mtx_events::KeyVerificationKey:
    case qml_mtx_events::KeyVerificationDone:
    case qml_mtx_events::KeyVerificationReady:
        return mtx::events::EventType::RoomMessage;
        //! m.image_pack, currently im.ponies.room_emotes
    case qml_mtx_events::ImagePackInRoom:
        return mtx::events::EventType::ImagePackInRoom;
    //! m.image_pack, currently im.ponies.user_emotes
    case qml_mtx_events::ImagePackInAccountData:
        return mtx::events::EventType::ImagePackInAccountData;
    //! m.image_pack.rooms, currently im.ponies.emote_rooms
    case qml_mtx_events::ImagePackRooms:
        return mtx::events::EventType::ImagePackRooms;
    default:
        return mtx::events::EventType::Unsupported;
    };
}

Timeline::Timeline(const QString &roomId, QObject *parent):
    QObject(parent),
    _events(roomId.toStdString(), this),
    _roomId(roomId),
    _permissions(_roomId)
    {
    nhlog::dev()->debug("Timeline created for: \"" + roomId.toStdString() + "\"");
    connect(cache::client(), &Cache::newReadReceipts,[&](const QString &room_id, const std::vector<QString> &event_ids){
        if(room_id == _roomId)
            emit newReadReceipts(event_ids);
    });
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
                // Client::instance()->receivedRoomDeviceVerificationRequest(msg, this);
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

    nlohmann::json doc = {{"type", mtx::events::to_string(eventType)},
                          {"content", nlohmann::json(msg.content)},
                {"room_id", room_id}};

    try {
        mtx::events::EncryptedEvent<mtx::events::msg::Encrypted> event;
        event.content  = olm::encrypt_group_message(room_id, http::client()->device_id(), doc);
        event.event_id = msg.event_id;
        event.room_id  = room_id;
        event.sender   = http::client()->user_id().to_string();
        event.type     = mtx::events::EventType::RoomEncrypted;
        event.origin_server_ts = QDateTime::currentMSecsSinceEpoch();

        emit this->addPendingMessageToStore(event);

        // TODO: Let the user know about the errors.
    } catch (const lmdb::error &e) {
        nhlog::db()->critical("failed to open outbound megolm session ({}): {}", room_id, e.what());
        emit Client::instance()->showNotification(
          tr("Failed to encrypt event, sending aborted!"));
    } catch (const mtx::crypto::olm_exception &e) {
        nhlog::crypto()->critical(
          "failed to open outbound megolm session ({}): {}", room_id, e.what());
        emit Client::instance()->showNotification(
          tr("Failed to encrypt event, sending aborted!"));
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

            encInfo = mtx::accessors::thumbnail_file(msg);
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

    void operator()(const mtx::events::RoomEvent<mtx::events::voip::CallInvite> &event)
    {
        sendRoomEvent<mtx::events::voip::CallInvite, mtx::events::EventType::CallInvite>(event);
    }

    void operator()(const mtx::events::RoomEvent<mtx::events::voip::CallCandidates> &event)
    {
        sendRoomEvent<mtx::events::voip::CallCandidates, mtx::events::EventType::CallCandidates>(
          event);
    }

    void operator()(const mtx::events::RoomEvent<mtx::events::voip::CallAnswer> &event)
    {
        sendRoomEvent<mtx::events::voip::CallAnswer, mtx::events::EventType::CallAnswer>(event);
    }

    void operator()(const mtx::events::RoomEvent<mtx::events::voip::CallHangUp> &event)
    {
        sendRoomEvent<mtx::events::voip::CallHangUp, mtx::events::EventType::CallHangUp>(event);
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

        if (std::holds_alternative<RoomEvent<voip::CallCandidates>>(e) ||
            std::holds_alternative<RoomEvent<voip::CallInvite>>(e) ||
            std::holds_alternative<RoomEvent<voip::CallAnswer>>(e) ||
            std::holds_alternative<RoomEvent<voip::CallHangUp>>(e))
            std::visit(
              [this](auto &event) {
                  event.room_id = _roomId.toStdString();
                  if constexpr (std::is_same_v<std::decay_t<decltype(event)>,
                                               RoomEvent<voip::CallAnswer>> ||
                                std::is_same_v<std::decay_t<decltype(event)>,
                                               RoomEvent<voip::CallHangUp>>)
                      emit newCallEvent(event);
                  else {
                      if (event.sender != http::client()->user_id().to_string())
                          emit newCallEvent(event);
                  }
              },
              e);
        // else if (std::holds_alternative<StateEvent<state::Avatar>>(e))
        //     emit roomAvatarUrlChanged();
        // else if (std::holds_alternative<StateEvent<state::Name>>(e))
        //     emit roomNameChanged();
        // else if (std::holds_alternative<StateEvent<state::Topic>>(e))
        //     emit roomTopicChanged();
        // else if (std::holds_alternative<StateEvent<state::PinnedEvents>>(e))
        //     emit pinnedMessagesChanged();
        // else if (std::holds_alternative<StateEvent<state::Widget>>(e))
        //     emit widgetLinksChanged();
        // else if (std::holds_alternative<StateEvent<state::PowerLevels>>(e)) {
        //     permissions_.invalidate();
        //     emit permissionsChanged();
        // } else if (std::holds_alternative<StateEvent<state::Member>>(e)) {
        //     emit roomAvatarUrlChanged();
        //     emit roomNameChanged();
        //     emit roomMemberCountChanged();
        // } else if (std::holds_alternative<StateEvent<state::Encryption>>(e)) {
        //     this->isEncrypted_ = cache::isRoomEncrypted(room_id_.toStdString());
        //     emit encryptionChanged();
        // }
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
isMessage(const mtx::events::RoomEvent<mtx::events::voip::CallInvite> &)
{
    return true;
}

auto
isMessage(const mtx::events::RoomEvent<mtx::events::voip::CallAnswer> &)
{
    return true;
}
auto
isMessage(const mtx::events::RoomEvent<mtx::events::voip::CallHangUp> &)
{
    return true;
}

DescInfo
Timeline::lastMessage() const
{
    if (_lastMessage.event_id.isEmpty())
        QTimer::singleShot(0, this, &Timeline::updateLastMessage);

    return _lastMessage;
}

void
Timeline::updateLastMessage()
{
    // only try to generate a preview for the last 1000 messages
    auto end = std::max(_events.size() - 1001, 0);
    for (auto it = _events.size() - 1; it >= end; --it) {
        auto event = _events.get(it, _decryptDescription);
        if (!event)
            continue;

        if (std::visit([](const auto &e) -> bool { return isYourJoin(e); }, *event)) {
            auto time   = mtx::accessors::origin_server_ts(*event);
            uint64_t ts = time.toMSecsSinceEpoch();
            auto description =
              DescInfo{QString::fromStdString(mtx::accessors::event_id(*event)),
                                QString::fromStdString(http::client()->user_id().to_string()),
                                tr("You joined this room."),
                                utils::descriptiveTime(time),
                                ts,
                                time,true};
            if (description != _lastMessage) {
                if (_lastMessage.timestamp == 0) {
                    cache::client()->updateLastMessageTimestamp(_roomId.toStdString(),
                                                                description.timestamp);
                }
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
            if (_lastMessage.timestamp == 0) {
                cache::client()->updateLastMessageTimestamp(_roomId.toStdString(),
                                                            description.timestamp);
            }
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


QString Timeline::escapeEmoji(QString str) const
{
    return utils::replaceEmoji(str);
}

QStringList Timeline::pinnedMessages() const {
    auto pinned =
      cache::client()->getStateEvent<mtx::events::state::PinnedEvents>(_roomId.toStdString());

    if (!pinned || pinned->content.pinned.empty())
        return {};

    QStringList list;
    list.reserve(pinned->content.pinned.size());
    for (const auto &p : pinned->content.pinned)
        list.push_back(QString::fromStdString(p));

    return list;
}


int Timeline::roomMemberCount() const {
    return (int)cache::client()->memberCount(_roomId.toStdString());
}

QString Timeline::directChatOtherUserId() const {
    if (roomMemberCount() < 3) {
        QString id;
        for (const auto &member : cache::getMembers(_roomId.toStdString()))
            if (member.user_id != UserSettings::instance()->userId())
                id = member.user_id;
        return id;
    } else
        return {};
}


void Timeline::unpin(const QString &id) {
    auto pinned =
      cache::client()->getStateEvent<mtx::events::state::PinnedEvents>(_roomId.toStdString());

    mtx::events::state::PinnedEvents content{};
    if (pinned)
        content = pinned->content;

    auto idStr = id.toStdString();

    for (auto it = content.pinned.begin(); it != content.pinned.end(); ++it) {
        if (*it == idStr) {
            content.pinned.erase(it);
            break;
        }
    }

    http::client()->send_state_event(
      _roomId.toStdString(),
      content,
      [idStr](const mtx::responses::EventId &, mtx::http::RequestErr err) {
          if (err)
              nhlog::net()->error("Failed to unpin {}: {}", idStr, *err);
          else
              nhlog::net()->debug("Unpinned {}", idStr);
      });
}

void Timeline::pin(const QString &id) {
    auto pinned =
      cache::client()->getStateEvent<mtx::events::state::PinnedEvents>(_roomId.toStdString());

    mtx::events::state::PinnedEvents content{};
    if (pinned)
        content = pinned->content;

    auto idStr = id.toStdString();
    content.pinned.push_back(idStr);

    http::client()->send_state_event(
      _roomId.toStdString(),
      content,
      [idStr](const mtx::responses::EventId &, mtx::http::RequestErr err) {
          if (err)
              nhlog::net()->error("Failed to pin {}: {}", idStr, *err);
          else
              nhlog::net()->debug("Pinned {}", idStr);
      });
}


std::optional<mtx::events::state::CanonicalAlias> Timeline::getRoomAliases(){
    return cache::client()->getRoomAliases(_roomId.toStdString());
}

std::vector<RoomMember> Timeline::getMembers(std::size_t startIndex, std::size_t len){
    return cache::getMembers(_roomId.toStdString(), startIndex, len);
}
void
Timeline::kickUser(const QString & userid, const QString & reason)
{
    http::client()->kick_user(
      _roomId.toStdString(),
      userid.toStdString(),
      [this, userid](const mtx::responses::Empty &, mtx::http::RequestErr err) {
          if (err) {
            emit Client::instance()->showNotification(tr("Failed to kick %1 from %2: %3")
                                      .arg(userid)
                                      .arg(_roomId)
                                      .arg(QString::fromStdString(err->matrix_error.error)));            
          } else {
            emit Client::instance()->showNotification(tr("Kicked user: %1").arg(userid));
          }
      },
      reason.trimmed().toStdString());
}

void
Timeline::banUser(const QString & userid, const QString & reason)
{
    http::client()->ban_user(
      _roomId.toStdString(),
      userid.toStdString(),
      [this, userid](const mtx::responses::Empty &, mtx::http::RequestErr err) {
          if (err) {            
              emit Client::instance()->showNotification(tr("Failed to ban %1 in %2: %3")
                                      .arg(userid)
                                      .arg(_roomId)
                                      .arg(QString::fromStdString(err->matrix_error.error)));
          } else {
              emit Client::instance()->showNotification(tr("Banned user: %1").arg(userid));
          }
      },
      reason.trimmed().toStdString());
}
void
Timeline::unbanUser(const QString & userid, const QString & reason)
{
    http::client()->unban_user(
      _roomId.toStdString(),
      userid.toStdString(),
      [this, userid](const mtx::responses::Empty &, mtx::http::RequestErr err) {
          if (err) {
              emit Client::instance()->showNotification(tr("Failed to unban %1 in %2: %3")
                                      .arg(userid)
                                      .arg(_roomId)
                                      .arg(QString::fromStdString(err->matrix_error.error)));
          } else{
              emit Client::instance()->showNotification(tr("Unbanned user: %1").arg(userid));
          }
      },
      reason.trimmed().toStdString());
}

void Timeline::forwardMessage(const QString &eventId, QString roomId) {
    auto e = _events.get(eventId.toStdString(), "");
    if (!e)
        return;
    emit forwardToRoom(e, std::move(roomId));
}

QString Timeline::viewDecryptedRawMessage(const QString &id){
    auto e = _events.get(id.toStdString(), "");
    if (!e)
        return "";

    return QString::fromStdString(mtx::accessors::serialize_event(*e).dump(4));
}

QString Timeline::viewRawMessage(const QString &id){
    auto e = _events.get(id.toStdString(), "", false);
    if (!e)
        return "";
    return QString::fromStdString(mtx::accessors::serialize_event(*e).dump(4));
}

Timeline::UserReceipts Timeline::readReceipts(const QString &event_id){
    return cache::readReceipts(event_id, _roomId);
}