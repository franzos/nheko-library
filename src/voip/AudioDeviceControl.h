#pragma once 
#include <QByteArray>
#include <QObject>
#include <QPixmap>
#include <QScopedPointer>
#include <QtGlobal>

#ifndef Q_OS_ANDROID
#include <QAudioInput>
#else

struct QAudioDeviceInfo {};
struct QAudioInput {};

#endif

#include "AudioInfo.h"
#include "AudioDevices.h"

class AudioDeviceControl : public QObject
{
    Q_OBJECT
public:
    AudioDeviceControl();

public slots:
    void initializeAudio(const QString &deviceDesc);
    void deviceChanged(const QString &deviceDesc);
    void setMicrophoneVolume(const QString &deviceDesc, qreal volume);
    qreal getMicrophoneVolume(const QString &deviceDesc);
    void setSpeakerVolume(qreal volume);
    qreal getSpeakerVolume();
    AudioDeviceInfo deviceInfo(qint32 index);
    
private:
    QAudioDeviceInfo audioDeviceInfo(const QString &deviceDesc);
    int32_t audioDeviceIndex(const QString &deviceDesc);

signals:
    void levelChanged(const qreal &level);
    void newInputDeviceStatus(qint32 index);
    void newOutputDeviceStatus(qint32 index);

private:
    QScopedPointer<AudioInfo> m_audioInfo;
    QScopedPointer<QAudioInput> m_audioInput;
    AudioDevices _audioDevices;
};