#pragma once

#include <QObject>
#include <mtx/events/event_type.hpp>
#include <mtx/events/collections.hpp>
#include "EventStore.h"
#include "../CacheStructs.h"

Q_DECLARE_METATYPE(mtx::events::collections::TimelineEvents)

class Timeline : public QObject {
Q_OBJECT
public:
    Timeline(const QString &roomId, QObject *parent = nullptr);
    void sync(const mtx::responses::JoinedRoom &room);
    void initialSync();

signals:
    void newEncryptedImage(mtx::crypto::EncryptedFile encryptionInfo);
    void newMessageToSend(mtx::events::collections::TimelineEvents event);
    void addPendingMessageToStore(mtx::events::collections::TimelineEvents event);
    void lastMessageChanged(const DescInfo &message);
    // void eventsChanged(int from, int to);
    void newEventsStored(int from, int len);
    void notificationsChanged();
    void typingUsersChanged(const QStringList &users);

public slots:
    bool canFetchMore() const;
    void sendMessage(const QString &msg);
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
    DescInfo _lastMessage{};
    bool _decryptDescription     = true;
    int _notificationCount = 0, _highlightCount = 0;
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
