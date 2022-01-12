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

public slots:
    bool canFetchMore() const;
    void sendMessage(const QString &msg);
    void setDecryptDescription(bool decrypt) { _decryptDescription = decrypt; }
    int  eventSize() {return _events.size();};
    QVector<DescInfo> getEvents(int from, int len);

private slots:
    void addPendingMessage(mtx::events::collections::TimelineEvents event);

private:
    void updateLastMessage();
    void addEvents(const mtx::responses::Timeline &timeline);
    template<class T>
    void sendMessageEvent(const T &content, mtx::events::EventType eventType);

    template<typename T>
    void sendEncryptedMessage(mtx::events::RoomEvent<T> msg, mtx::events::EventType eventType);
   
    mutable EventStore _events;
    QString _roomId;
    DescInfo _lastMessage{};
    bool _decryptDescription     = true;
    friend struct SendMessageVisitor;
};