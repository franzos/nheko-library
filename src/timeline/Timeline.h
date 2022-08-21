#pragma once

#include <QObject>
#include <mtx/events/event_type.hpp>
#include <mtx/events/collections.hpp>
#include <mtx/events/power_levels.hpp>
#include "EventStore.h"
#include "../Cache_p.h"
#include "../CacheStructs.h"
#include "Permissions.h"

Q_DECLARE_METATYPE(mtx::events::collections::TimelineEvents)

namespace qml_mtx_events {
Q_NAMESPACE

enum EventType
{
    // Unsupported event
    Unsupported,
    /// m.room_key_request
    KeyRequest,
    /// m.reaction,
    Reaction,
    /// m.room.aliases
    Aliases,
    /// m.room.avatar
    Avatar,
    /// m.call.invite
    CallInvite,
    /// m.call.answer
    CallAnswer,
    /// m.call.hangup
    CallHangUp,
    /// m.call.candidates
    CallCandidates,
    /// m.room.canonical_alias
    CanonicalAlias,
    /// m.room.create
    RoomCreate,
    /// m.room.encrypted.
    Encrypted,
    /// m.room.encryption.
    Encryption,
    /// m.room.guest_access
    RoomGuestAccess,
    /// m.room.history_visibility
    RoomHistoryVisibility,
    /// m.room.join_rules
    RoomJoinRules,
    /// m.room.member
    Member,
    /// m.room.name
    Name,
    /// m.room.power_levels
    PowerLevels,
    /// m.room.tombstone
    Tombstone,
    /// m.room.topic
    Topic,
    /// m.room.redaction
    Redaction,
    /// m.room.pinned_events
    PinnedEvents,
    // m.sticker
    Sticker,
    // m.tag
    Tag,
    // m.widget
    Widget,
    /// m.room.message
    AudioMessage,
    EmoteMessage,
    FileMessage,
    ImageMessage,
    LocationMessage,
    NoticeMessage,
    TextMessage,
    VideoMessage,
    Redacted,
    UnknownMessage,
    KeyVerificationRequest,
    KeyVerificationStart,
    KeyVerificationMac,
    KeyVerificationAccept,
    KeyVerificationCancel,
    KeyVerificationKey,
    KeyVerificationDone,
    KeyVerificationReady,
    //! m.image_pack, currently im.ponies.room_emotes
    ImagePackInRoom,
    //! m.image_pack, currently im.ponies.user_emotes
    ImagePackInAccountData,
    //! m.image_pack.rooms, currently im.ponies.emote_rooms
    ImagePackRooms,
    // m.policy.rule.user
    PolicyRuleUser,
    // m.policy.rule.room
    PolicyRuleRoom,
    // m.policy.rule.server
    PolicyRuleServer,
    // m.space.parent
    SpaceParent,
    // m.space.child
    SpaceChild,
};
Q_ENUM_NS(EventType)
mtx::events::EventType fromRoomEventType(qml_mtx_events::EventType);
EventType toRoomEventType(mtx::events::EventType e);
EventType toRoomEventType(const mtx::events::collections::TimelineEvents &event);
QString toRoomEventTypeString(const mtx::events::collections::TimelineEvents &event);

enum EventState
{
    //! The plaintext message was received by the server.
    Received,
    //! At least one of the participants has read the message.
    Read,
    //! The client sent the message. Not yet received.
    Sent,
    //! When the message is loaded from cache or backfill.
    Empty,
};
Q_ENUM_NS(EventState)
}


namespace {
struct RoomEventType
{
    template<class T>
    qml_mtx_events::EventType operator()(const mtx::events::Event<T> &e)
    {
        return qml_mtx_events::toRoomEventType(e.type);
    }
    qml_mtx_events::EventType operator()(const mtx::events::Event<mtx::events::msg::Audio> &)
    {
        return qml_mtx_events::EventType::AudioMessage;
    }
    qml_mtx_events::EventType operator()(const mtx::events::Event<mtx::events::msg::Emote> &)
    {
        return qml_mtx_events::EventType::EmoteMessage;
    }
    qml_mtx_events::EventType operator()(const mtx::events::Event<mtx::events::msg::File> &)
    {
        return qml_mtx_events::EventType::FileMessage;
    }
    qml_mtx_events::EventType operator()(const mtx::events::Event<mtx::events::msg::Image> &)
    {
        return qml_mtx_events::EventType::ImageMessage;
    }
    qml_mtx_events::EventType operator()(const mtx::events::Event<mtx::events::msg::Notice> &)
    {
        return qml_mtx_events::EventType::NoticeMessage;
    }
    qml_mtx_events::EventType operator()(const mtx::events::Event<mtx::events::msg::Text> &)
    {
        return qml_mtx_events::EventType::TextMessage;
    }
    qml_mtx_events::EventType operator()(const mtx::events::Event<mtx::events::msg::Video> &)
    {
        return qml_mtx_events::EventType::VideoMessage;
    }
    qml_mtx_events::EventType
    operator()(const mtx::events::Event<mtx::events::msg::KeyVerificationRequest> &)
    {
        return qml_mtx_events::EventType::KeyVerificationRequest;
    }
    qml_mtx_events::EventType
    operator()(const mtx::events::Event<mtx::events::msg::KeyVerificationStart> &)
    {
        return qml_mtx_events::EventType::KeyVerificationStart;
    }
    qml_mtx_events::EventType
    operator()(const mtx::events::Event<mtx::events::msg::KeyVerificationMac> &)
    {
        return qml_mtx_events::EventType::KeyVerificationMac;
    }
    qml_mtx_events::EventType
    operator()(const mtx::events::Event<mtx::events::msg::KeyVerificationAccept> &)
    {
        return qml_mtx_events::EventType::KeyVerificationAccept;
    }
    qml_mtx_events::EventType
    operator()(const mtx::events::Event<mtx::events::msg::KeyVerificationReady> &)
    {
        return qml_mtx_events::EventType::KeyVerificationReady;
    }
    qml_mtx_events::EventType
    operator()(const mtx::events::Event<mtx::events::msg::KeyVerificationCancel> &)
    {
        return qml_mtx_events::EventType::KeyVerificationCancel;
    }
    qml_mtx_events::EventType
    operator()(const mtx::events::Event<mtx::events::msg::KeyVerificationKey> &)
    {
        return qml_mtx_events::EventType::KeyVerificationKey;
    }
    qml_mtx_events::EventType
    operator()(const mtx::events::Event<mtx::events::msg::KeyVerificationDone> &)
    {
        return qml_mtx_events::EventType::KeyVerificationDone;
    }
    qml_mtx_events::EventType operator()(const mtx::events::Event<mtx::events::msg::Redacted> &)
    {
        return qml_mtx_events::EventType::Redacted;
    }
    qml_mtx_events::EventType operator()(const mtx::events::Event<mtx::events::voip::CallInvite> &)
    {
        return qml_mtx_events::EventType::CallInvite;
    }
    qml_mtx_events::EventType operator()(const mtx::events::Event<mtx::events::voip::CallAnswer> &)
    {
        return qml_mtx_events::EventType::CallAnswer;
    }
    qml_mtx_events::EventType operator()(const mtx::events::Event<mtx::events::voip::CallHangUp> &)
    {
        return qml_mtx_events::EventType::CallHangUp;
    }
    qml_mtx_events::EventType
    operator()(const mtx::events::Event<mtx::events::voip::CallCandidates> &)
    {
        return qml_mtx_events::EventType::CallCandidates;
    }
    // ::EventType::Type operator()(const Event<mtx::events::msg::Location> &e) { return
    // ::EventType::LocationMessage; }
};
}


class StateKeeper
{
public:
    StateKeeper(std::function<void()> &&fn)
      : fn_(std::move(fn))
    {}

    ~StateKeeper() { fn_(); }

private:
    std::function<void()> fn_;
};

struct DecryptionResult
{
    //! The decrypted content as a normal plaintext event.
    mtx::events::collections::TimelineEvents event;
    //! Whether or not the decryption was successful.
    bool isDecrypted = false;
};

class Timeline : public QObject {
Q_OBJECT
public:
    Timeline(const QString &roomId, QObject *parent = nullptr);
    void sync(const mtx::responses::JoinedRoom &room);
    void initialSync();
    QString escapeEmoji(QString str) const;
    QString id() {return _roomId;};    
    using UserReceipts = std::multimap<uint64_t, std::string, std::greater<uint64_t>>;

signals:
    void newEncryptedImage(mtx::crypto::EncryptedFile encryptionInfo);
    void newMessageToSend(mtx::events::collections::TimelineEvents event);
    void addPendingMessageToStore(mtx::events::collections::TimelineEvents event);
    void newCallEvent(const mtx::events::collections::TimelineEvents &event);
    void lastMessageChanged(const DescInfo &message);
    // void eventsChanged(int from, int to);
    void newEventsStored(int from, int len);
    void notificationsChanged();
    void typingUsersChanged(const QStringList &users);
    void forwardToRoom(mtx::events::collections::TimelineEvents *e, QString roomId);

public slots:
    mtx::events::state::PowerLevels powerLevels() { 
        return cache::client()
                   ->getStateEvent<mtx::events::state::PowerLevels>(_roomId.toStdString())
                   .value_or(mtx::events::StateEvent<mtx::events::state::PowerLevels>{})
                   .content; };
    Permissions *permissions() { return &_permissions; };
    bool canFetchMore() const;
    void setDecryptDescription(bool decrypt) { _decryptDescription = decrypt; }
    int  eventSize() {return _events.size();};
    QVector<DescInfo> getEvents(int from, int len, bool markAsRead = true);
    void updateLastMessage();
    int highlightCount() { return _highlightCount; }
    int notificationCount() { return _notificationCount; }
    void updateTypingUsers(const QStringList &users) {
        if (this->_typingUsers != users) {
            this->_typingUsers = users;
            emit typingUsersChanged(_typingUsers);
        }
    }
    QStringList typingUsers() const { return _typingUsers; }
    void markEventsAsRead(const QStringList &event_ids);
    QString displayName(QString id) const;
    QString avatarUrl(QString id) const;
    EventStore *events() {return &_events;};
    QStringList pinnedMessages() const;
    int roomMemberCount() const;
    bool isDirect() const { return roomMemberCount() <= 2; }
    QString directChatOtherUserId() const;
    void unpin(const QString &id);
    void pin(const QString &id);
    std::optional<mtx::events::state::CanonicalAlias> getRoomAliases();
    std::vector<RoomMember> getMembers(std::size_t startIndex = 0, std::size_t len = 30);
    void kickUser(const QString & userid, const QString & reason);
    void banUser(const QString & userid, const QString & reason);
    void unbanUser(const QString & userid, const QString & reason);
    void forwardMessage(const QString &eventId, QString roomId);
    QString viewDecryptedRawMessage(const QString &id);
    QString viewRawMessage(const QString &id);
    UserReceipts readReceipts(const QString &event_id);
    DescInfo lastMessage() const;
    uint64_t lastMessageTimestamp() const { return _lastMessage.timestamp; }

private slots:
    void addPendingMessage(mtx::events::collections::TimelineEvents event);

public:
    template<class T>
    void sendMessageEvent(const T &content, mtx::events::EventType eventType);

private:
    void addEvents(const mtx::responses::Timeline &timeline);

    template<typename T>
    void sendEncryptedMessage(mtx::events::RoomEvent<T> msg, mtx::events::EventType eventType);
   
    mutable EventStore _events;
    QString _roomId;
    Permissions _permissions;
    DescInfo _lastMessage{};
    bool _decryptDescription     = true;
    uint64_t _notificationCount = 0, _highlightCount = 0;
    QStringList _typingUsers;
    friend struct SendMessageVisitor;
};



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
