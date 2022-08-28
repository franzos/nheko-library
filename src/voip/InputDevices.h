#pragma once

#include <QtGlobal>
#ifndef Q_OS_ANDROID
#include <QObject>
#include <pulse/pulseaudio.h>
#include <pulse/glib-mainloop.h>
#include <pulse/ext-stream-restore.h>
#include <pulse/ext-device-manager.h>
#include <QMap>
#endif
class InputDeviceInfo {
    Q_GADGET
    public:
        int index;
        QString name;
        QString desc;
        qreal volume;

    Q_PROPERTY(int index MEMBER index)
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(QString desc MEMBER desc)
    Q_PROPERTY(int volume MEMBER volume)
};
#ifndef Q_OS_ANDROID
class InputDevices : public QObject
{
    Q_OBJECT

public:
    InputDevices(QObject *parent = nullptr);
    ~InputDevices(){};
    void removeSource(uint32_t index);
    void updateSource(const pa_source_info &info);
    QMap<uint32_t, InputDeviceInfo> sources() {return _sources;};

signals:
    void newDeviceStatus(uint32_t index);

public slots:
    void setVolume(uint32_t index, qreal volume);
    qreal getVolume(uint32_t index);

private:
    QMap<uint32_t, InputDeviceInfo> _sources;
};
#endif