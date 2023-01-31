#pragma once

#include <QMap>
#include <QObject>
#include <QtGlobal>

#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
#include <pulse/pulseaudio.h>
#include <pulse/glib-mainloop.h>
#include <pulse/ext-stream-restore.h>
#include <pulse/ext-device-manager.h>
#else 

struct pa_source_info {};
struct pa_sink_info {};

#endif

class AudioDeviceInfo {
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

class AudioDevices : public QObject
{
    Q_OBJECT

public:
    AudioDevices(QObject *parent = nullptr);
    ~AudioDevices(){};
    void removeSource(uint32_t index);
    void updateSource(const pa_source_info &info);

    void checkDefaultOutput();
    void removeSink(uint32_t index);
    void updateSink(const pa_sink_info &info);
    QMap<uint32_t, AudioDeviceInfo> sources() {return _sources;};
    QMap<uint32_t, AudioDeviceInfo> sinks() {return _sinks;};

signals:
    void newInputDeviceStatus(uint32_t index);
    void newOutputDeviceStatus(uint32_t index);
    void defaultOutputDeviceChanged(uint32_t index);

public slots:
    void setMicrophoneVolume(uint32_t index, qreal volume);
    qreal getMicrophoneVolume(uint32_t index);
    void setSpeakerVolume(uint32_t index, qreal volume);
    qreal getSpeakerVolume(uint32_t index);
    QString defaultAudioOutput();

private:
    QMap<uint32_t, AudioDeviceInfo> _sources;
    QMap<uint32_t, AudioDeviceInfo> _sinks;
    QString    _defaultAudioOutput;
};
